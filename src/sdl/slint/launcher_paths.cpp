#include "launcher_paths.h"
#include "launcher_api.h"

#include <SDL_filesystem.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <optional>
#include <system_error>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

namespace wc2_launcher {

namespace fs = std::filesystem;

std::string path_to_utf8(const fs::path &path)
{
    const auto utf8 = path.u8string();
    return std::string(utf8.begin(), utf8.end());
}

fs::path path_from_utf8(std::string_view value)
{
    std::u8string utf8;

    utf8.reserve(value.size());
    for (const unsigned char character : value)
        utf8.push_back(static_cast<char8_t>(character));
    return fs::path(utf8);
}

static bool equal_ascii_case(std::string_view left, std::string_view right)
{
    if (left.size() != right.size())
        return false;
    return std::equal(
        left.begin(), left.end(), right.begin(),
        [](unsigned char leftCharacter, unsigned char rightCharacter) {
            if (leftCharacter >= 'A' && leftCharacter <= 'Z')
                leftCharacter += 'a' - 'A';
            if (rightCharacter >= 'A' && rightCharacter <= 'Z')
                rightCharacter += 'a' - 'A';
            return leftCharacter == rightCharacter;
        });
}

static std::optional<fs::path> find_child_case_insensitive(
    const fs::path &directory, std::string_view expected)
{
    std::error_code error;
    fs::directory_iterator iterator(directory, error);
    const fs::directory_iterator end;

    while (!error && iterator != end) {
        if (equal_ascii_case(path_to_utf8(iterator->path().filename()), expected))
            return iterator->path();
        iterator.increment(error);
    }
    return std::nullopt;
}

static bool has_readable_file(const fs::path &directory, std::string_view name)
{
    const auto path = find_child_case_insensitive(directory, name);
    std::error_code error;

    return path && fs::is_regular_file(*path, error) &&
        std::ifstream(*path, std::ios::binary).peek() != std::ifstream::traits_type::eof();
}

DirectoryStatus validate_game_directory(std::string_view directoryText)
{
    if (directoryText.empty())
        return {false, "Choose a Wing Commander II directory."};
    if (directoryText.size() >= SDL_LAUNCHER_DIRECTORY_CAPACITY)
        return {false, "The directory path is too long."};

    const fs::path directory = path_from_utf8(directoryText);
    std::error_code error;
    if (!fs::is_directory(directory, error))
        return {false, "The selected path is not a directory."};
    const auto gameData = find_child_case_insensitive(directory, "GAMEDAT");
    if (!gameData || !fs::is_directory(*gameData, error))
        return {false, "Choose a WC2 directory containing GAMEDAT."};

    const auto cockpit = find_child_case_insensitive(*gameData, "COCKPIT.VGA");
    if (!cockpit || !fs::is_regular_file(*cockpit, error))
        return {false, "GAMEDAT/COCKPIT.VGA is missing."};
    std::ifstream cockpitFile(*cockpit, std::ios::binary);
    std::array<unsigned char, 8> header = {};
    cockpitFile.read(reinterpret_cast<char *>(header.data()), header.size());
    if (cockpitFile.gcount() != static_cast<std::streamsize>(header.size()))
        return {false, "GAMEDAT/COCKPIT.VGA is unreadable or too small to identify."};
    // WC1 also has COCKPIT.VGA. Require WC2's campaign and gameflow files
    // before accepting a data root, including one chosen manually.
    for (const auto name : {"CAMPAIGN.S00", "GAMEFLOW.S00"}) {
        if (!has_readable_file(*gameData, name))
            return {false, std::string("GAMEDAT/") + name +
                " is missing or unreadable. Select a WC2 installation."};
    }

    // Match SdlUsingDosData: the Saga conversion changed 0xc1 to 0xe0.
    if (header[7] == 0xc1)
        return {true, "DOS"};
    if (header[7] != 0xe0)
        return {false, "GAMEDAT/COCKPIT.VGA is not recognized WC2 data."};
    const auto streams = find_child_case_insensitive(directory, "STREAMS");
    if (!streams || !fs::is_directory(*streams, error))
        return {false, "Kilrathi Saga data needs a sibling STREAMS directory."};
    for (const auto name : {"GAMEFLOW.STR", "GAMETWO.STR", "SPACEFLT.STR"}) {
        if (!has_readable_file(*streams, name))
            return {false, std::string("STREAMS/") + name + " is missing or unreadable."};
    }
    return {true, "Kilrathi Saga"};
}

int add_game_installation(std::vector<GameInstallation> &installations,
                          std::string_view directory)
{
    const auto status = validate_game_directory(directory);
    if (!status.valid)
        return -1;
    std::error_code error;
    const auto canonical = fs::canonical(path_from_utf8(directory), error);
    if (error)
        return -1;
    const auto text = path_to_utf8(canonical);
    if (text.size() >= SDL_LAUNCHER_DIRECTORY_CAPACITY)
        return -1;
    for (size_t index = 0; index < installations.size(); ++index) {
        if (fs::equivalent(canonical, path_from_utf8(installations[index].directory), error))
            return static_cast<int>(index);
    }
    installations.push_back({text, status.message});
    return static_cast<int>(installations.size() - 1);
}

static std::string normalized_installation_name(const fs::path &directory)
{
    std::string result;
    // Ignore spaces, punctuation and trademark symbols used by store titles.
    for (unsigned char character : path_to_utf8(directory.filename())) {
        if (character >= 'A' && character <= 'Z')
            character += 'a' - 'A';
        if ((character >= 'a' && character <= 'z') ||
            (character >= '0' && character <= '9'))
            result += character;
    }
    return result;
}

static bool is_wc2_installation_name(const fs::path &directory)
{
    const auto name = normalized_installation_name(directory);
    for (const std::string_view prefix : {"wingcommanderii", "wingcommander2", "wc2", "wing2"}) {
        if (!name.starts_with(prefix))
            continue;
        const std::string_view suffix = std::string_view(name).substr(prefix.size());
        // Store names include both the subtitle and the anniversary edition.
        // Exact suffixes prevent WC III and other games from matching WC II.
        for (const auto expected : {"", "vengeanceofthekilrathi",
                                   "35thanniversaryedition",
                                   "vengeanceofthekilrathi35thanniversaryedition",
                                   "dos", "dosversion", "legacy", "1991",
                                   "thekilrathisaga", "kilrathisaga"}) {
            if (suffix == expected)
                return true;
        }
    }
    return name == "wingcommander1and2" || name == "wingcommander12" ||
        name == "wingcommanderiandii" || name == "wcks" ||
        name == "kilrathisaga" || name == "thekilrathisaga" ||
        name == "wingcommanderthekilrathisaga" || name == "wingcommanderkilrathisaga";
}

static void scan_game_directory(std::vector<GameInstallation> &installations,
                                const fs::path &directory, int depth = 0)
{
    if (directory.empty())
        return;
    add_game_installation(installations, path_to_utf8(directory));
    if (depth == 3)
        return;
    // Only descend through known package layouts, never through arbitrary
    // user folders, Wine prefixes, or another game's data.
    for (const auto name : {"Kilrathi_Saga", "Kilrathi Saga", "DOS", "WC2", "WING2"}) {
        const auto child = find_child_case_insensitive(directory, name);
        if (child)
            scan_game_directory(installations, *child, depth + 1);
    }
}

static void scan_game_library(std::vector<GameInstallation> &installations,
                              const fs::path &library)
{
    std::error_code error;
    fs::directory_iterator iterator(library, error);
    const fs::directory_iterator end;
    std::vector<fs::path> directories;
    while (!error && iterator != end) {
        if (is_wc2_installation_name(iterator->path()))
            directories.push_back(iterator->path());
        iterator.increment(error);
    }
    std::sort(directories.begin(), directories.end());
    for (const auto &directory : directories)
        scan_game_directory(installations, directory);
}

static fs::path environment_directory(const char *name)
{
#ifdef _WIN32
    // Windows environment strings are UTF-16, not the active ANSI codepage.
    const std::wstring wideName(name, name + std::char_traits<char>::length(name));
    const wchar_t *value = _wgetenv(wideName.c_str());
    return value != nullptr ? fs::path(value) : fs::path();
#else
    const char *value = std::getenv(name);
    return value != nullptr ? fs::path(value) : fs::path();
#endif
}

std::vector<GameInstallation> discover_game_installations()
{
    std::vector<fs::path> dataDirectories;
    std::vector<fs::path> libraries;
    std::error_code error;
    const auto workingDirectory = fs::current_path(error);
    if (!error)
        dataDirectories.push_back(workingDirectory);
    // Desktop shortcuts/Finder need not use the executable's directory as cwd.
    if (char *base = SDL_GetBasePath()) {
        dataDirectories.push_back(path_from_utf8(base));
        SDL_free(base);
    }

#ifdef _WIN32
    const auto home = environment_directory("USERPROFILE");
#else
    auto home = environment_directory("HOME");
    if (home.empty()) {
        if (const auto *user = getpwuid(getuid()))
            home = user->pw_dir;
    }
#endif
    if (!home.empty()) {
        libraries.push_back(home / "Games" / "Heroic");
        libraries.push_back(home / "GOG Games");
    }

#ifdef _WIN32
    auto systemDrive = environment_directory("SystemDrive");
    if (systemDrive.empty())
        systemDrive = L"C:";
    // Keep the system drive as a fallback if drive enumeration fails.
    std::vector<fs::path> driveRoots = {systemDrive / L"\\"};
    const DWORD driveMask = GetLogicalDrives();
    for (wchar_t letter = L'A'; letter <= L'Z'; ++letter) {
        if ((driveMask & (1UL << (letter - L'A'))) == 0)
            continue;
        const wchar_t root[] = {letter, L':', L'\\', L'\0'};
        if (GetDriveTypeW(root) == DRIVE_FIXED &&
            std::find(driveRoots.begin(), driveRoots.end(), root) == driveRoots.end())
            driveRoots.emplace_back(root);
    }

    std::vector<fs::path> libraryRoots;
    for (const auto &driveRoot : driveRoots) {
        libraryRoots.push_back(driveRoot); // WING2, WC2, WCKS
        libraryRoots.push_back(driveRoot / "Program Files");
        libraryRoots.push_back(driveRoot / "Program Files (x86)");
    }
    // Environment paths also cover relocated Program Files directories.
    for (const auto &programFiles : {
             environment_directory("ProgramFiles"),
             environment_directory("ProgramFiles(x86)"),
             environment_directory("ProgramW6432")}) {
        if (!programFiles.empty() &&
            std::find(libraryRoots.begin(), libraryRoots.end(), programFiles) == libraryRoots.end())
            libraryRoots.push_back(programFiles);
    }

    // Probe the same known layouts on every fixed drive; only recognized WC2
    // folders and their package layouts are explored by scan_game_library.
    for (const auto &libraryRoot : libraryRoots) {
        libraries.push_back(libraryRoot);
        for (const auto relative : {"GOG.com", "GOG Games", "GOG Galaxy/Games",
                                    "Games", "Games/Heroic", "Heroic",
                                    "Steam/steamapps/common", "SteamLibrary/steamapps/common",
                                    "steamapps/common", "Origin", "Origin Games",
                                    "Origin Systems", "EA Games", "EA/Games"})
            libraries.push_back(libraryRoot / relative);
    }
#elif defined(__APPLE__)
    if (!home.empty())
        libraries.push_back(home / "Library/Application Support/Steam/steamapps/common");
#else
    if (!home.empty()) {
        libraries.push_back(home / ".steam/steam/steamapps/common");
        libraries.push_back(home / ".local/share/Steam/steamapps/common");
        libraries.push_back(home / ".var/app/com.valvesoftware.Steam/data/Steam/steamapps/common");
    }
#endif
    std::vector<GameInstallation> installations;
    for (const auto &directory : dataDirectories)
        scan_game_directory(installations, directory);
    for (const auto &library : libraries)
        scan_game_library(installations, library);
    return installations;
}

} // namespace wc2_launcher
