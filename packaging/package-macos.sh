#!/usr/bin/env bash
# See README-macos-build.txt for the build environment and dependencies.
set -euo pipefail

version=${1:?usage: package-macos.sh VERSION ARCH GUI_BINARY}
architecture=${2:?usage: package-macos.sh VERSION ARCH GUI_BINARY}
binary=${3:?usage: package-macos.sh VERSION ARCH GUI_BINARY}
if test "$(uname -s)" != Darwin; then
    echo "The macOS app packager must run on macOS." >&2
    exit 1
fi
if [[ "$version" =~ ^v?([0-9]+\.[0-9]+\.[0-9]+)([-.][A-Za-z0-9.-]+)?$ ]]; then
    bundle_version=${BASH_REMATCH[1]}
elif [[ "$version" =~ ^ci-[0-9]+-[0-9]+$ ]]; then
    # Manual workflow runs produce unpublished artifacts, not release versions.
    bundle_version=0.0.0
else
    echo "Expected a release version or a ci-RUN-ATTEMPT artifact version." >&2
    exit 1
fi
case "$architecture" in
    arm64|x86_64) ;;
    *) echo "Unsupported architecture: $architecture" >&2; exit 1 ;;
esac
binary="$(cd "$(dirname "$binary")" && pwd)/$(basename "$binary")"
lipo "$binary" -verify_arch "$architecture"
if ! nm -gU "$binary" | grep ' _SdlRunLauncherGui$' >/dev/null; then
    echo "The disk image requires the integrated modern-gui executable." >&2
    exit 1
fi
if nm -u "$binary" | grep -E '__asan_|__ubsan_' >/dev/null; then
    echo "Build with MODERN_RELEASE=1 in a separate output directory first." >&2
    exit 1
fi

root=$(cd "$(dirname "$0")/.." && pwd)
dmgbuild=${DMGBUILD:-dmgbuild}
if ! command -v "$dmgbuild" >/dev/null; then
    echo "Install packaging/requirements-macos.txt in a virtualenv and activate it." >&2
    exit 1
fi
dist_dir=${DIST_DIR:-"$root/dist"}
mkdir -p "$dist_dir"
dist_dir=$(cd "$dist_dir" && pwd)
archive="wc2-re-${version}-macos-${architecture}"
dmg="$dist_dir/$archive.dmg"
if test -e "$dmg"; then
    echo "Disk image already exists: $dmg" >&2
    exit 1
fi
work=$(mktemp -d "$dist_dir/.macos-package.XXXXXX")
cleanup() {
    rm -rf "$work"
}
trap cleanup EXIT

# Reuse the release packager's dependency and license collection.
(
    cd "$root"
    DIST_DIR="$work/portable" packaging/package-modern.sh \
        "$version" macos "$architecture" "$binary"
)
portable="$work/portable/$archive-sdl2"
app="$work/image/Wing Commander II.app"
mkdir -p "$app/Contents/"{MacOS,Frameworks,Resources}
mv "$portable/wc2-modern" "$app/Contents/MacOS/wc2-modern"
# SDL_GetBasePath() resolves to Contents/Resources for an application bundle.
# Keep catalogs out of the code directory so bundle signing treats them as data.
mv "$portable/translations" "$app/Contents/Resources/"
mv "$portable/"*.dylib "$app/Contents/Frameworks/"
cp "$portable/"*LICENSE.txt "$portable/THIRD-PARTY-NOTICES.txt" "$app/Contents/Resources/"

for library in "$app/Contents/Frameworks/"*.dylib; do
    install_name_tool -id "@rpath/$(basename "$library")" "$library"
done
for executable in "$app/Contents/MacOS/wc2-modern" "$app/Contents/Frameworks/"*.dylib; do
    lipo "$executable" -verify_arch "$architecture"
    minimum=$(xcrun vtool -show-build "$executable" | awk '/minos/ {print $2}')
    if ! awk -v version="$minimum" 'BEGIN {
        split(version, parts, "."); exit !(version != "" &&
            (parts[1] < 15 || (parts[1] == 15 && parts[2] == 0 && parts[3] == 0)))
    }'; then
        echo "Cannot confirm macOS 15.0 support: $executable ($minimum)" >&2
        exit 1
    fi
    while IFS= read -r dependency; do
        name=$(basename "$dependency")
        # otool also lists a dylib's own install ID.
        if test "$name" = "$(basename "$executable")"; then
            continue
        fi
        if test -f "$app/Contents/Frameworks/$name"; then
            install_name_tool -change "$dependency" \
                "@executable_path/../Frameworks/$name" "$executable"
        else
            case "$dependency" in
                /System/Library/*|/usr/lib/*) ;;
                *) echo "Unbundled dependency: $dependency" >&2; exit 1 ;;
            esac
        fi
    done < <(otool -L "$executable" | tail -n +2 | awk '{print $1}')
done

swift "$root/packaging/macos-assets.swift" "$work/assets"
mkdir "$work/WC2.iconset"
for size in 16 32 128 256 512; do
    sips -z "$size" "$size" "$work/assets/icon.png" \
        --out "$work/WC2.iconset/icon_${size}x${size}.png" >/dev/null
    sips -z "$((size * 2))" "$((size * 2))" "$work/assets/icon.png" \
        --out "$work/WC2.iconset/icon_${size}x${size}@2x.png" >/dev/null
done
iconutil -c icns "$work/WC2.iconset" -o "$app/Contents/Resources/WC2.icns"
cat > "$app/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
    <key>CFBundleDevelopmentRegion</key><string>en</string>
    <key>CFBundleExecutable</key><string>wc2-modern</string>
    <key>CFBundleIdentifier</key><string>io.github.neuromancer.wc2-re</string>
    <key>CFBundleName</key><string>Wing Commander II</string>
    <key>CFBundleDisplayName</key><string>Wing Commander II</string>
    <key>CFBundleIconFile</key><string>WC2.icns</string>
    <key>CFBundlePackageType</key><string>APPL</string>
    <key>CFBundleInfoDictionaryVersion</key><string>6.0</string>
    <key>CFBundleShortVersionString</key><string>$bundle_version</string>
    <key>CFBundleVersion</key><string>$bundle_version</string>
    <key>LSMinimumSystemVersion</key><string>15.0</string>
    <key>LSApplicationCategoryType</key><string>public.app-category.simulation-games</string>
    <key>NSHighResolutionCapable</key><true/>
</dict></plist>
EOF
plutil -lint "$app/Contents/Info.plist"
for library in "$app/Contents/Frameworks/"*.dylib; do
    codesign --force --sign - "$library"
done
codesign --force --sign - "$app"
codesign --verify --deep --strict "$app"

"$dmgbuild" -s "$root/packaging/macos-dmg-settings.py" \
    -D app="$app" \
    -D background="$work/assets/background.png" \
    "Wing Commander II" "$work/installer.dmg"
hdiutil verify "$work/installer.dmg"
mv "$work/installer.dmg" "$dmg"
echo "Created: $dmg"
