#include "launcher.h"
#include "launcher_api.h"
#include "wc2_logo.h"

#include <nfd.h>
#include <slint.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace {

namespace fs = std::filesystem;

struct DirectoryStatus {
    bool valid;
    std::string message;
};

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

bool equal_ascii_case(std::string_view left, std::string_view right)
{
    if (left.size() != right.size())
        return false;
    return std::equal(
        left.begin(), left.end(), right.begin(),
        [](unsigned char leftCharacter, unsigned char rightCharacter) {
            if (leftCharacter >= 'A' && leftCharacter <= 'Z')
                leftCharacter = static_cast<unsigned char>(
                    leftCharacter + ('a' - 'A'));
            if (rightCharacter >= 'A' && rightCharacter <= 'Z')
                rightCharacter = static_cast<unsigned char>(
                    rightCharacter + ('a' - 'A'));
            return leftCharacter == rightCharacter;
        });
}

std::optional<fs::path> find_child_case_insensitive(
    const fs::path &directory, std::string_view expected)
{
    std::error_code error;
    fs::directory_iterator iterator(directory, error);
    const fs::directory_iterator end;

    while (!error && iterator != end) {
        if (equal_ascii_case(path_to_utf8(iterator->path().filename()),
                             expected))
            return iterator->path();
        iterator.increment(error);
    }
    return std::nullopt;
}

DirectoryStatus validate_game_directory(std::string_view directoryText)
{
    const fs::path directory = path_from_utf8(directoryText);
    std::error_code error;

    if (!fs::is_directory(directory, error)) {
        return {false, "The selected path is not a directory."};
    }
    const auto gameData = find_child_case_insensitive(directory, "GAMEDAT");
    if (!gameData) {
        return {false, "GAMEDAT was not found in this directory."};
    }
    const auto cockpit = find_child_case_insensitive(*gameData, "COCKPIT.VGA");
    if (!cockpit) {
        return {false, "GAMEDAT/COCKPIT.VGA is missing."};
    }

    std::ifstream cockpitFile(*cockpit, std::ios::binary);
    std::array<unsigned char, 8> header = {};

    if (!cockpitFile) {
        return {false, "GAMEDAT/COCKPIT.VGA could not be read."};
    }
    cockpitFile.read(reinterpret_cast<char *>(header.data()), header.size());
    if (cockpitFile.gcount() != static_cast<std::streamsize>(header.size())) {
        return {false, "GAMEDAT/COCKPIT.VGA is too small to identify."};
    }
    if (header[7] == 0xc1)
        return {true, "Wing Commander II DOS data found."};

    const auto streams = find_child_case_insensitive(directory, "STREAMS");
    if (!streams) {
        return {false, "The sibling STREAMS directory is missing."};
    }
    if (!find_child_case_insensitive(*streams, "GAMEFLOW.STR")) {
        return {false, "STREAMS/GAMEFLOW.STR is missing."};
    }
    return {true, "Wing Commander II Kilrathi Saga data found."};
}

void refresh_window(LauncherWindow &window)
{
    const std::string directory(window.get_game_directory());
    const DirectoryStatus status = validate_game_directory(directory);

    window.set_picker_error(false);
    window.set_path_valid(status.valid);
    window.set_status_text(slint::SharedString(status.message));
}

void report_picker_error(LauncherWindow &window, std::string message,
                         const char *details)
{
    if (details != nullptr && details[0] != '\0') {
        message += ": ";
        message += details;
    }
    window.set_picker_error(true);
    window.set_status_text(slint::SharedString(message));
}

void browse_for_game_directory(LauncherWindow &window)
{
    const std::string currentDirectory(window.get_game_directory());
    std::string defaultDirectory;
    std::error_code error;

    if (!currentDirectory.empty()) {
        const fs::path candidate = path_from_utf8(currentDirectory);
        if (fs::is_directory(candidate, error)) {
            const fs::path absolute = fs::absolute(candidate, error);
            if (!error)
                defaultDirectory = path_to_utf8(absolute);
        }
    }

    nfdu8char_t *selectedDirectory = nullptr;
    const nfdresult_t result = NFD_PickFolderU8(
        &selectedDirectory,
        defaultDirectory.empty() ? nullptr : defaultDirectory.c_str());
    if (result == NFD_CANCEL)
        return;
    if (result != NFD_OKAY || selectedDirectory == nullptr) {
        report_picker_error(window, "The folder picker failed",
                            NFD_GetError());
        return;
    }

    const std::string selection(selectedDirectory);
    NFD_FreePathU8(selectedDirectory);
    window.set_game_directory(slint::SharedString(selection));
    refresh_window(window);
}

} // namespace

int SdlRunLauncherGui(SdlLauncherOptions *options)
{
    bool accepted;
    bool pickerInitialized;

    if (options == nullptr)
        return SDL_LAUNCHER_ERROR;

    auto window = LauncherWindow::create();
    accepted = false;
    pickerInitialized = false;
    window->set_title_artwork(
        slint::private_api::load_image_from_embedded_data(
            wc2_launcher_artwork::kWingCommanderLogoPng, "png"));
    window->set_game_directory(slint::SharedString(options->gameDirectory));
    window->set_enhanced_renderer(options->enhancedRenderer != 0);
    window->set_balanced_difficulty(options->balancedDifficulty != 0);
    window->set_joystick_rumble(options->joystickRumble != 0);
    window->set_show_frame_rate(options->showFrameRate != 0);
    window->set_cockpitless(options->cockpitless != 0);
    window->set_joystick_mode_index(options->joystickMode);
    window->set_joystick_axes_index(options->joystickAxes);
    refresh_window(*window);

    window->on_configuration_changed([&window]() {
        refresh_window(*window);
    });
    window->on_launch_requested([&window, &accepted]() {
        refresh_window(*window);
        if (!window->get_path_valid())
            return;
        accepted = true;
        slint::quit_event_loop();
    });
    window->on_cancel_requested([]() {
        slint::quit_event_loop();
    });

    window->show();
    /* NFD_Init records Cocoa's activation policy. Initialize it from the
     * callback, after winit launches NSApplication, so NFD_Quit cannot restore
     * the temporary pre-launch Prohibited policy before SDL takes over. */
    window->on_browse_requested([&window, &pickerInitialized]() {
        if (!pickerInitialized) {
            if (NFD_Init() != NFD_OKAY) {
                report_picker_error(
                    *window, "The folder picker could not be initialized",
                    NFD_GetError());
                return;
            }
            pickerInitialized = true;
        }
        browse_for_game_directory(*window);
    });
    slint::run_event_loop();
    if (pickerInitialized)
        NFD_Quit();
    window->hide();
    if (!accepted)
        return SDL_LAUNCHER_CANCELLED;

    const std::string gameDirectory(window->get_game_directory());
    if (gameDirectory.size() >= sizeof(options->gameDirectory))
        return SDL_LAUNCHER_ERROR;
    std::memcpy(options->gameDirectory, gameDirectory.c_str(),
                gameDirectory.size() + 1);
    options->enhancedRenderer = window->get_enhanced_renderer();
    options->balancedDifficulty = window->get_balanced_difficulty();
    options->joystickRumble = window->get_joystick_rumble();
    options->showFrameRate = window->get_show_frame_rate();
    options->cockpitless = window->get_cockpitless();
    options->joystickMode = std::max(0, window->get_joystick_mode_index());
    options->joystickAxes = std::max(0, window->get_joystick_axes_index());
    return SDL_LAUNCHER_ACCEPTED;
}
