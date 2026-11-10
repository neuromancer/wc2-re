#include "launcher.h"
#include "launcher_api.h"
#include "launcher_config.h"
#include "launcher_paths.h"
#include "wc2_logo.h"

#include <nfd.h>
#include <slint.h>
#include <SDL.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>

namespace {

namespace fs = std::filesystem;

using namespace wc2_launcher;

struct LauncherJoystick {
    SDL_JoystickID instanceId;
    SDL_JoystickGUID guid;
    std::string path;
};

std::vector<LauncherJoystick> list_joysticks(LauncherWindow &window)
{
    std::vector<LauncherJoystick> joysticks;
    std::vector<slint::SharedString> names = {window.get_automatic_joystick()};

    SDL_JoystickUpdate();
    SDL_LockJoysticks();
    const int count = SDL_NumJoysticks();
    for (int index = 0; index < count; ++index) {
        const auto instanceId = SDL_JoystickGetDeviceInstanceID(index);
        if (instanceId < 0)
            continue;
        const char *name = SDL_JoystickNameForIndex(index);
        const char *path = nullptr;
#if SDL_VERSION_ATLEAST(2, 24, 0)
        path = SDL_JoystickPathForIndex(index);
#endif
        joysticks.push_back({instanceId, SDL_JoystickGetDeviceGUID(index),
                             path != nullptr ? path : ""});
        names.emplace_back(std::to_string(index + 1) + " — " +
                            (name != nullptr ? std::string(name) : std::string(window.get_unnamed_joystick())));
    }
    SDL_UnlockJoysticks();
    window.set_joystick_devices(
        std::make_shared<slint::VectorModel<slint::SharedString>>(names));
    window.set_joystick_device_index(0);
    return joysticks;
}

void refresh_window(LauncherWindow &window,
                    std::vector<GameInstallation> &installations)
{
    const std::string directory(window.get_game_directory());
    const DirectoryStatus status = validate_game_directory(directory);
    const int selected = add_game_installation(installations, directory);
    std::vector<slint::SharedString> choices;
    window.set_has_installations(!installations.empty());
    choices.emplace_back(window.get_installation_prompt());
    for (const auto &installation : installations)
        choices.emplace_back(std::string(installation.edition == LauncherMessage::Dos ? "DOS" : "Kilrathi Saga") + " — " + installation.directory);

    window.set_installations(std::make_shared<slint::VectorModel<slint::SharedString>>(choices));
    window.set_installation_index(selected + 1);
    window.set_has_installations(!installations.empty());

    window.set_picker_error(false);
    window.set_path_valid(status.valid);
    window.set_has_so1(status.valid && validate_game_directory(directory, 1).valid);
    window.set_status(status.message);
    window.set_status_detail(slint::SharedString(status.detail));
}

void report_picker_error(LauncherWindow &window, LauncherMessage message,
                         const char *details)
{
    window.set_picker_error(true);
    window.set_status(message);
    window.set_status_detail(details ? details : "");
}

void browse_for_game_directory(LauncherWindow &window,
                               std::vector<GameInstallation> &installations)
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
        report_picker_error(window, LauncherMessage::PickerFailed,
                            NFD_GetError());
        return;
    }

    const std::string selection(selectedDirectory);
    NFD_FreePathU8(selectedDirectory);
    window.set_game_directory(slint::SharedString(selection));
    refresh_window(window, installations);
}

} // namespace

int SdlRunLauncherGui(SdlLauncherOptions *options)
{
    bool accepted;
    bool pickerInitialized;
    int selectedCampaign = 0;

    if (options == nullptr)
        return SDL_LAUNCHER_ERROR;
    const auto configPath = launcher_config_path();
    load_launcher_config(configPath, *options);
    auto installations = discover_game_installations();

    std::string initialDirectory(options->gameDirectory);
    if (!validate_game_directory(initialDirectory).valid && !installations.empty()) {
        initialDirectory = installations.front().directory;
    } else if (initialDirectory.empty()) {
        std::error_code error;
        const auto currentDirectory = fs::current_path(error);
        initialDirectory = error ? "." : path_to_utf8(currentDirectory);
    }

    auto window = LauncherWindow::create();
    slint::select_bundled_translation(options->language == 1 ? "es" : "en");
    accepted = false;
    pickerInitialized = false;
    window->set_title_artwork(
        slint::private_api::load_image_from_embedded_data(
            wc2_launcher_artwork::kWingCommanderLogoPng, "png"));
    window->set_game_directory(slint::SharedString(initialDirectory));
    window->set_start_fullscreen(options->fullscreen != 0);
    window->set_language_index(options->language);
    window->set_enhanced_renderer(options->enhancedRenderer != 0);
    window->set_ega_dither(options->egaDither != 0);
    window->set_balanced_difficulty(options->balancedDifficulty != 0);
    window->set_cheat_mode(options->cheatMode != 0);
    window->set_joystick_rumble(options->joystickRumble != 0);
    window->set_joystick_mode_index(options->joystickMode);
    window->set_joystick_axes_index(options->joystickAxes);
    // Keep the subsystem alive through the SDL handoff so IDs stay valid.
    SDL_SetMainReady();
    const bool joystickInitialized = SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0;
    std::vector<LauncherJoystick> joysticks;
    if (joystickInitialized)
        joysticks = list_joysticks(*window);
    refresh_window(*window, installations);

    window->on_language_selected([&window, &installations](int language) {
        const auto unnamed = window->get_unnamed_joystick();
        if (!slint::select_bundled_translation(language == 1 ? "es" : "en"))
            return;
        window->set_language_index(language == 1 ? 1 : 0);
        auto devices = window->get_joystick_devices();
        for (size_t i = 0; i < devices->row_count(); ++i) {
            const auto value = devices->row_data(i);
            if (i == 0)
                devices->set_row_data(i, window->get_automatic_joystick());
            else if (value && std::string(*value) == std::to_string(i) + " — " + std::string(unnamed))
                devices->set_row_data(i, slint::SharedString(std::to_string(i) + " — " + std::string(window->get_unnamed_joystick())));
        }
        refresh_window(*window, installations);
    });

    window->on_installation_selected([&window, &installations](int index) {
        if (index > 0 && static_cast<size_t>(index) <= installations.size())
            window->set_game_directory(slint::SharedString(installations[index - 1].directory));
        refresh_window(*window, installations);
    });
    window->on_launch_requested([&window, &accepted, &installations, &selectedCampaign](int campaign) {
        refresh_window(*window, installations);
        if (!window->get_path_valid() ||
            (campaign == 1 && !window->get_has_so1()))
            return;
        std::error_code error;
        const std::string directory(window->get_game_directory());
        const auto absoluteDirectory = fs::absolute(path_from_utf8(directory), error);
        if (error) {
            report_picker_error(*window, LauncherMessage::CannotResolveDirectory,
                                error.message().c_str());
            return;
        }
        const auto absoluteText = path_to_utf8(absoluteDirectory);
        if (absoluteText.size() >= SDL_LAUNCHER_DIRECTORY_CAPACITY) {
            report_picker_error(*window, LauncherMessage::PathTooLong, nullptr);
            return;
        }
        fs::current_path(absoluteDirectory, error);
        if (error) {
            report_picker_error(*window, LauncherMessage::CannotOpenDirectory,
                                error.message().c_str());
            return;
        }
        window->set_game_directory(slint::SharedString(absoluteText));
        selectedCampaign = campaign;
        accepted = true;
        /* Hide while the event loop still runs: on Linux the request takes
           effect only when the loop processes it, so hiding after the loop
           left the launcher on screen during the game. */
        window->hide();
        slint::quit_event_loop();
    });
    window->on_cancel_requested([]() {
        slint::quit_event_loop();
    });

    window->show();
    /* NFD_Init records Cocoa's activation policy. Initialize it from the
     * callback, after winit launches NSApplication, so NFD_Quit cannot restore
     * the temporary pre-launch Prohibited policy before SDL takes over. */
    window->on_browse_requested([&window, &pickerInitialized, &installations]() {
        if (!pickerInitialized) {
            if (NFD_Init() != NFD_OKAY) {
                report_picker_error(
                    *window, LauncherMessage::PickerInitFailed,
                    NFD_GetError());
                return;
            }
            pickerInitialized = true;
        }
        browse_for_game_directory(*window, installations);
    });
    slint::run_event_loop();
    if (pickerInitialized)
        NFD_Quit();
    window->hide();
    if (!accepted) {
        if (joystickInitialized)
            SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        return SDL_LAUNCHER_CANCELLED;
    }

    const std::string gameDirectory(window->get_game_directory());
    if (gameDirectory.size() >= sizeof(options->gameDirectory)) {
        if (joystickInitialized)
            SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        return SDL_LAUNCHER_ERROR;
    }
    std::memcpy(options->gameDirectory, gameDirectory.c_str(),
                gameDirectory.size() + 1);
    options->fullscreen = window->get_start_fullscreen();
    options->language = std::max(0, window->get_language_index());
    options->campaign = selectedCampaign;
    options->enhancedRenderer = window->get_enhanced_renderer();
    options->egaDither = window->get_ega_dither();
    options->balancedDifficulty = window->get_balanced_difficulty();
    options->cheatMode = window->get_cheat_mode();
    options->joystickRumble = window->get_joystick_rumble();
    options->joystickMode = std::max(0, window->get_joystick_mode_index());
    options->joystickAxes = std::max(0, window->get_joystick_axes_index());
    options->joystickInstanceId = -1;
    options->joystickGuidUnique = 0;
    options->joystickGuid[0] = '\0';
    options->joystickPath[0] = '\0';
    const int deviceIndex = window->get_joystick_device_index() - 1;
    if (deviceIndex >= 0 && static_cast<size_t>(deviceIndex) < joysticks.size()) {
        const auto &device = joysticks[deviceIndex];
        options->joystickInstanceId = device.instanceId;
        options->joystickGuidUnique = std::count_if(
            joysticks.begin(), joysticks.end(), [&device](const LauncherJoystick &other) {
                return std::memcmp(&device.guid, &other.guid, sizeof(device.guid)) == 0;
            }) == 1;
        SDL_JoystickGetGUIDString(device.guid, options->joystickGuid,
                                  sizeof(options->joystickGuid));
        if (device.path.size() < sizeof(options->joystickPath))
            std::memcpy(options->joystickPath, device.path.c_str(), device.path.size() + 1);
    }
    if (!configPath.empty() && !save_launcher_config(configPath, *options))
        std::fprintf(stderr, "Cannot save launcher settings: %s\n",
                     path_to_utf8(configPath).c_str());
    return SDL_LAUNCHER_ACCEPTED;
}
