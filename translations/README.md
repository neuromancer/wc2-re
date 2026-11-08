# External translations

Choose **Language → Español** in the launcher, or start with
`--language=es`. Use `--language=en` for English. The launcher remembers the
selection; command-line language options override its saved setting. Changing
the language also updates the launcher immediately.

Spanish covers 3,337 base-game cutscene dialogue entries, 158 marked game UI
strings, radio text, mission and navigation labels, menu artwork, opening
credits and scrolling credits, using DOS or Kilrathi Saga game data. The
launcher has its own translation catalog. Voices remain unchanged. The
upstream translation's reordered COMMUNIC.T29 packets and Special Operations
resources are excluded.

The game reads `translations/es.dat` beside its executable. Editable UTF-8
PO files are compiled into WC2's single-byte text and accent glyphs, following
wc1-re's WCT2 catalog approach. English fallback comes from the user's original
game data and the recovered source. Game files on disk are never rewritten.
Text, font glyphs and selected image frames change in memory; scripts, timing,
resource indices, hotspot commands and existing community fixes are preserved.
Artwork changes require the expected packet checksum and leave unrelated
frames and palettes intact. Opening credits apply after the community
director-credit correction, preserving the corrected name. Their Spanish
caption placement is included in the images, leaving the scripts unchanged.

Edit `es/cutscenes.po`, `es/resources.po` or `es/ui.po`, preserving `msgctxt`, `msgid`, placeholders
such as `{callsign}` and `{last_name}`, formatting directives and shortcut
letters. IDs hash the original bytes instead of copying English dialogue into
the catalog. Empty or fuzzy entries fall back to English. Compilation rejects
changed placeholders, oversized dialogue or resource fields, and invalid glyph definitions.
Glyphs use CP1252 slots; WC2's six control bytes remain reserved in cutscenes.

```sh
make modern-translations          # rebuild the external game catalog without relinking
make modern modern-gui            # also build the executables
make update-translations         # refresh marked game UI strings; requires GNU gettext
make update-launcher-translations # refresh Slint strings; also requires slint-tr-extractor
```

Mark new game UI strings with `_()` or `C_(context, text)`. Static initializers
can use `N_()`/`NC_()`, with translation performed at display time. These macros
preserve the original expressions in the reference build. Mark launcher strings
with Slint's `@tr()` and edit `es/LC_MESSAGES/launcher.po`; these are bundled
into the launcher when it is rebuilt. Keep keyboard shortcut letters unchanged
even when the surrounding text is translated.

The imported Spanish text, artwork and glyphs are by **L@Zar0** and **darkpadawan**, from
version 1.1 (2021). The [upstream notice](upstream/Leeme.txt) declares GPL v3.0
with an additional no-charge term. [provenance.json](provenance.json) records
the download, hashes, one restored callsign placeholder and three alternative
IDs for Saga's line-wrap changes. Game UI and launcher translations reuse the
matching wc1-re catalog entries, with additions for WC2; see their PO headers.

The editable catalogs contain the imported text and artwork changes; builds
need neither the Spanish installer nor an import tool. The import excludes
executables, voices, gameplay changes and expansion resources. One Python
script, `bin/compileTranslations.py`, compiles game catalogs and refreshes
marked UI strings. Release archives and both installers include the compiled
catalog and the upstream notice.
