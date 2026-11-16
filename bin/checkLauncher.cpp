/* Exercises the launcher's tabbed window on an offscreen Slint window, after
 * wc1-re's former launcher check: the game controller's labels for each mode,
 * the keyboard and mouse table and the action list that the game shares
 * (src/sdl/keymap_actions.h), saving and reading back changed bindings, the
 * guns and afterburner buttons' swap, the German patch's language menu, the
 * joystick dead zone setting, the calibration page, which calibrates SDL
 * virtual flight sticks, and the Music row and its MT-32 line. With an argument it also saves BMP previews named
 * after it: every tab, the calibration page, the Steam Deck's 1280x800 at a
 * scale of 1.2 and a 640x480 window that scrolls. */
#include "../src/sdl/slint/slint_static.h"
#include "../src/sdl/slint/launcher.cpp"
#include <slint-platform.h>
#include <SDL.h>
#include "../src/sdl/keymap_actions.h"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <thread>

// The game reads real executables; fixtures mark a German WC2.EXE by its
// first line.
extern "C" int SdlCountGermanText(const char *path)
{
    std::ifstream file(path);
    std::string line;
    return std::getline(file, line) && line == "translated" ? 121 : 0;
}

// The check has no MT-32 ROMs: a folder holding cm32l.rom stands for a
// usable set (src/sdl/mt32.c describes real ones).
extern "C" int SdlDescribeMt32Roms(const char *directory, const char *machine,
                                   char *text, size_t textSize)
{
    std::error_code error;
    const bool usable = directory != nullptr && *directory != '\0' &&
        fs::exists(path_from_utf8(directory) / "cm32l.rom", error);
    (void)machine;
    std::snprintf(text, textSize, "%s", usable ? "CM-32L test ROMs (cm32l_1_02)"
                                               : "No MT-32 or CM-32L ROMs.");
    return usable;
}

struct TestWindow : slint::platform::WindowAdapter {
    slint::platform::SoftwareRenderer pixels {slint::platform::SoftwareRenderer::RepaintBufferType::NewBuffer};
    slint::PhysicalSize dimensions {{800, 660}};
    slint::PhysicalSize size() override
    {
        return dimensions;
    }
    slint::platform::AbstractRenderer &renderer() override
    {
        return pixels;
    }
};

struct TestPlatform : slint::platform::Platform {
    TestWindow *view = nullptr;
    std::unique_ptr<slint::platform::WindowAdapter> create_window_adapter() override
    {
        auto adapter = std::make_unique<TestWindow>();
        view = adapter.get();
        return adapter;
    }
};

static void require(bool condition, const char *message)
{
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}

int main(int argc, char **argv)
{
    auto backend = std::make_unique<TestPlatform>();
    auto *platform = backend.get();
    slint::platform::set_platform(std::move(backend));
    auto window = LauncherWindow::create();
    // Each window gets its own adapter; keep the one this window draws on.
    TestWindow *view = platform->view;
    window->set_joystick_mode_index(0);
    window->set_joystick_axes_index(0);

    // Enhanced rendering does not opt into HUD fonts or discard the choice.
    require(!window->get_hires_hud_fonts(), "HUD fonts default to off in the GUI");
    const bool enhancedRenderer = window->get_enhanced_renderer();
    window->set_enhanced_renderer(true);
    require(!window->get_hires_hud_fonts(), "enhanced rendering leaves HUD fonts off");
    window->set_hires_hud_fonts(true);
    window->set_enhanced_renderer(false);
    require(window->get_hires_hud_fonts(), "disabling enhanced rendering preserves the HUD-font choice");
    window->set_enhanced_renderer(enhancedRenderer);
    window->set_hires_hud_fonts(false);

    // The Joystick tab labels the game controller for the chosen layout.
    const auto binding = [&window](PadControl control) {
        return std::string(window->invoke_pad_binding(control));
    };
    require(binding(PadControl::A) == "fire guns" && binding(PadControl::B) == "hold to roll and change speed" &&
            binding(PadControl::RightTrigger) == "fire guns" &&
            binding(PadControl::LeftTrigger) == "hold to roll and change speed" &&
            binding(PadControl::DpadUp).empty() && binding(PadControl::Start) == "pause" &&
            binding(PadControl::Back) == "cycle views",
            "original two-button controller bindings, the triggers standing in for A and B");
    window->set_joystick_mode_index(1);
    require(binding(PadControl::B) == "fire missile" && binding(PadControl::Y) == "hold to roll and change speed" &&
            binding(PadControl::DpadUp) == "full speed" && binding(PadControl::DpadDown) == "immediate stop" &&
            binding(PadControl::LeftShoulder) == "gun type" &&
            binding(PadControl::RightShoulder) == "weapon system" && binding(PadControl::RightStick).empty(),
            "WCAT two-axis controller bindings");
    window->set_joystick_mode_index(2);
    window->set_joystick_axes_index(2);
    require(binding(PadControl::Y) == "change target" && binding(PadControl::LeftStick) == "roll and pitch" &&
            binding(PadControl::RightStick) == "yaw, throttle position" &&
            binding(PadControl::LeftTrigger) == "fire missile" &&
            binding(PadControl::RightTrigger) == "fire guns", "WCAT four-axis HOTAS-yaw controller bindings");
    window->set_joystick_mode_index(0);
    window->set_joystick_axes_index(0);

    // Opening the keyboard and mouse tab asks for the saved bindings, each
    // time; the launcher takes them the first time. Slint runs change
    // handlers with its timers, as the event loop does between clicks.
    int requested = 0;
    window->on_controls_requested([&requested]() { ++requested; });
    for (const int tab : {3, 0, 3, 0}) {
        window->set_settings_tab(tab);
        slint::platform::update_timers_and_animations();
    }
    require(requested == 2, "the keyboard and mouse tab asks for the bindings as it opens");

    // One table row per game input, each with its keys and mouse inputs.
    prepare_controls(window);
    const auto rows = window->get_keyboard_rows();
    const auto find_row = [&rows](const char *id) {
        for (size_t index = 0; index < rows->row_count(); ++index) {
            const auto row = rows->row_data(index);
            if (row && row->id == id)
                return static_cast<int>(index);
        }
        return -1;
    };
    const auto inputs = [](const InputBinding &binding) {
        std::string text;
        for (size_t index = 0; binding.inputs && index < binding.inputs->row_count(); ++index)
            text += (index ? "|" : "") + std::string(binding.inputs->row_data(index).value_or(slint::SharedString()));
        return text;
    };
    const auto current = [&window, &inputs](int row) {
        std::string text;
        const auto table = window->get_keyboard_bindings();
        const auto bindings = table && row >= 0 && static_cast<size_t>(row) < table->row_count()
            ? table->row_data(row).value_or(nullptr) : nullptr;
        for (size_t slot = 0; bindings && slot < bindings->row_count(); ++slot)
            text += (slot ? "/" : "") + inputs(bindings->row_data(slot).value_or(InputBinding {}));
        return text;
    };
    const int missileRow = find_row("mouse-missile");
    const int rollRow = find_row("mouse-roll");
    require(current(find_row("pitch-down")) == "up" && current(find_row("roll-left")) == "insert/," &&
            current(find_row("speed-up")) == "+/wheel-up" && current(find_row("stop-turn")) == "keypad-5" &&
            current(find_row("attack-my-target")) == "alt|a" && current(find_row("eject")) == "ctrl|e" &&
            current(find_row("pause")) == "p/escape" &&
            current(missileRow) == "mouse-left|mouse-right" && current(rollRow) == "mouse-right|mouse-move",
            "the launcher copies the original bindings for the tab");

    // The rows the game can rebind, in src/sdl/keymap_actions.h's order.
    {
#define LAUNCHER_KEYMAP_ID(id, ...) id,
        static const char *const gameActions[] = {SDL_KEYMAP_ACTION_LIST(LAUNCHER_KEYMAP_ID)};
#undef LAUNCHER_KEYMAP_ID
        size_t next = 0;
        bool matches = true;
        for (size_t index = 0; index < rows->row_count(); ++index) {
            const auto row = rows->row_data(index);
            if (!row || row->id.empty() || row->fixed || row->id == "full-screen" ||
                row->id == "mouse-guns" || row->id == "mouse-afterburner")
                continue;
            matches = matches && next < std::size(gameActions) && row->id == gameActions[next];
            ++next;
        }
        require(matches && next == std::size(gameActions), "the game rebinds every changeable row by its id");
    }
    for (size_t index = 0; index < rows->row_count(); ++index) {
        const auto row = rows->row_data(index);
        const auto limit = static_cast<size_t>(row && row->limit > 0 ? row->limit : window->get_max_bindings());
        require(row && (!row->bindings || row->bindings->row_count() <= limit),
                "no game input starts with more bindings than it takes");
    }

    // Saved bindings come back as the same [keys] lines.
    // In the table's order: the mouse rows come before the wingman's.
    const std::string saved = "pitch-down=k\nroll-left=\nmouse-guns=mouse-right\n"
                              "mouse-afterburner=mouse-left\nattack-my-target=y\nfull-screen=f11\n";
    {
        auto reopened = LauncherWindow::create();
        prepare_controls(reopened, saved);
        require(changed_key_bindings(*reopened) == saved, "saved bindings read back unchanged");
    }
    require(changed_key_bindings(*window).empty(), "an untouched tab changes nothing");

    // Giving the guns row the afterburner's button swaps the two.
    const int gunsRow = find_row("mouse-guns");
    InputBinding right;
    right.inputs = std::make_shared<slint::VectorModel<slint::SharedString>>(
        std::vector<slint::SharedString> {"mouse-right"});
    window->invoke_binding_changed(gunsRow, 0, right);
    require(current(gunsRow) == "mouse-right" && current(find_row("mouse-afterburner")) == "mouse-left" &&
            current(missileRow) == "mouse-right|mouse-left" && current(rollRow) == "mouse-left|mouse-move",
            "the guns and afterburner buttons swap");
    window->invoke_bindings_reset();
    require(changed_key_bindings(*window).empty(), "restore defaults resets every row");

    // A DOS installation for the settings and the calibration below.
    std::error_code error;
    const auto fixture = fs::temp_directory_path(error) / "wc2-launcher-check";
    fs::remove_all(fixture, error);
    fs::create_directories(fixture / "GAMEDAT");
    const char dosHeader[8] = {28, 0, 0, 0, 28, 0, 0, static_cast<char>(0xc1)};
    std::ofstream(fixture / "GAMEDAT/COCKPIT.VGA", std::ios::binary).write(dosHeader, 8);
    std::ofstream(fixture / "GAMEDAT/CAMPAIGN.S00") << "x";
    std::ofstream(fixture / "GAMEDAT/GAMEFLOW.S00") << "x";
    std::vector<GameInstallation> installations;
    require(!window->get_calibration_available(), "nothing to calibrate without an installation");

    {
        // The German patch for the DOS release replaces WC2.EXE; its own text
        // replaces what Spanish and Polish translate.
        window->set_game_directory(slint::SharedString(path_to_utf8(fixture)));
        refresh_window(*window, installations);
        require(window->get_path_valid() && !window->get_german_text() &&
                window->get_language_choices()->row_count() == 3,
                "an English DOS install offers every language");
        set_language(*window, SDL_LAUNCHER_LANGUAGE_POLISH);
        require(window->get_language_index() == SDL_LAUNCHER_LANGUAGE_POLISH,
                "Polish selected for an English install");
        std::ofstream(fixture / "WC2.EXE") << "translated\n";
        refresh_window(*window, installations);
        require(window->get_german_text() && window->get_language_choices()->row_count() == 1 &&
                window->get_language_choices()->row_data(0) == slint::SharedString("German") &&
                window->get_language_index() == SDL_LAUNCHER_LANGUAGE_ENGLISH,
                "a German WC2.EXE offers only German, switching Polish back");
        set_language(*window, SDL_LAUNCHER_LANGUAGE_SPANISH);
        require(window->get_language_index() == SDL_LAUNCHER_LANGUAGE_ENGLISH,
                "Spanish refused for a German install");
        fs::remove(fixture / "WC2.EXE", error);
        refresh_window(*window, installations);
        require(!window->get_german_text() && window->get_language_choices()->row_count() == 3,
                "an English install offers every language again");
    }

    // The Music row: a DOS install offers the AdLib and the MT-32, a Kilrathi
    // Saga install also its digital music, and the MT-32 line says whether
    // the ROMs can play.
    {
        require(window->get_dos_data() && window->get_music_available() &&
                window->get_music_choices()->row_count() == 2,
                "a DOS install offers the AdLib and the MT-32");
        window->set_music_device(SDL_LAUNCHER_MUSIC_MT32);
        show_music_device(*window);
        require(window->get_music_index() == 1, "the MT-32 is the DOS install's second choice");
        const auto saga = fs::temp_directory_path(error) / "wc2-launcher-saga";
        fs::remove_all(saga, error);
        fs::create_directories(saga / "GAMEDAT");
        fs::create_directories(saga / "STREAMS");
        const char sagaHeader[8] = {28, 0, 0, 0, 28, 0, 0, static_cast<char>(0xe0)};
        std::ofstream(saga / "GAMEDAT/COCKPIT.VGA", std::ios::binary).write(sagaHeader, 8);
        std::ofstream(saga / "GAMEDAT/CAMPAIGN.S00") << "x";
        std::ofstream(saga / "GAMEDAT/GAMEFLOW.S00") << "x";
        for (const char *name : {"GAMEFLOW.STR", "GAMETWO.STR", "SPACEFLT.STR"})
            std::ofstream(saga / "STREAMS" / name) << "x";
        window->set_game_directory(slint::SharedString(path_to_utf8(saga)));
        refresh_window(*window, installations);
        require(window->get_path_valid() && !window->get_dos_data() &&
                window->get_music_choices()->row_count() == 3 && window->get_music_index() == 2,
                "a Kilrathi Saga install offers digital, AdLib and MT-32 music");
        window->set_game_directory(slint::SharedString(path_to_utf8(fixture)));
        refresh_window(*window, installations);
        fs::remove_all(saga, error);

        refresh_mt32_roms(*window, "", "");
        require(!window->get_mt32_roms_set() && !window->get_mt32_ready(), "no ROM folder yet");
        const auto roms = fs::temp_directory_path(error) / "wc2-launcher-roms";
        fs::create_directories(roms);
        std::ofstream(roms / "cm32l.rom") << "x";
        refresh_mt32_roms(*window, path_to_utf8(roms), "");
        require(window->get_mt32_roms_set() && window->get_mt32_ready() &&
                std::string(window->get_mt32_detail()) == "CM-32L test ROMs (cm32l_1_02)",
                "a usable ROM folder");

        SdlLauncherOptions music = {};
        music.musicDevice = SDL_LAUNCHER_MUSIC_MT32;
        std::snprintf(music.mt32Roms, sizeof(music.mt32Roms), "%s", path_to_utf8(roms).c_str());
        std::snprintf(music.mt32Model, sizeof(music.mt32Model), "%s", "mt32_1_07");
        require(save_launcher_config(fixture / "music.ini", music), "save the music settings");
        SdlLauncherOptions restored = {};
        load_launcher_config(fixture / "music.ini", restored);
        require(restored.musicDevice == SDL_LAUNCHER_MUSIC_MT32 &&
                std::string(restored.mt32Roms) == path_to_utf8(roms) &&
                std::string(restored.mt32Model) == "mt32_1_07", "the music settings survive a restart");
        fs::remove_all(roms, error);
        refresh_mt32_roms(*window, "", "");
    }

    // HUD fonts default to off and retain either saved choice across sessions.
    {
        SdlLauncherOptions restored = {};
        load_launcher_config(fixture / "hud.ini", restored);
        require(restored.hiResHudFonts == 0, "HUD fonts stay off without a settings file");
        std::ofstream(fixture / "hud.ini") << "[launcher]\nenhanced_renderer=1\n";
        load_launcher_config(fixture / "hud.ini", restored);
        require(restored.enhancedRenderer == 1 && restored.hiResHudFonts == 0,
                "older settings with enhanced rendering leave HUD fonts off");

        SdlLauncherOptions saved = {};
        saved.hiResHudFonts = 1;
        saved.joystickDeadZone = 5;
        require(save_launcher_config(fixture / "hud.ini", saved), "save HUD fonts and dead zone");
        restored = {};
        load_launcher_config(fixture / "hud.ini", restored);
        require(restored.hiResHudFonts == 1 && restored.joystickDeadZone == 5,
                "HUD fonts and dead zone survive a restart independently");
        restored = {};
        restored.explicitOptions = SDL_LAUNCHER_EXPLICIT_HIRES_HUD;
        load_launcher_config(fixture / "hud.ini", restored);
        require(restored.hiResHudFonts == 0 && restored.joystickDeadZone == 5,
                "a HUD-font override still loads the saved dead zone");
        restored = {};
        restored.explicitOptions = SDL_LAUNCHER_EXPLICIT_JOYSTICK_DEAD_ZONE;
        load_launcher_config(fixture / "hud.ini", restored);
        require(restored.hiResHudFonts == 1 && restored.joystickDeadZone == 0,
                "a dead-zone override still loads the saved HUD fonts");

        saved.hiResHudFonts = 0;
        require(save_launcher_config(fixture / "hud.ini", saved), "save HUD fonts disabled again");
        restored = {};
        restored.hiResHudFonts = 1;
        load_launcher_config(fixture / "hud.ini", restored);
        require(restored.hiResHudFonts == 0 && restored.joystickDeadZone == 5,
                "the saved off choice replaces a previously enabled HUD-font setting");
    }

    // The dead zone is a percentage saved with the other settings.
    {
        SdlLauncherOptions zone = {};
        zone.joystickDeadZone = 5;
        require(save_launcher_config(fixture / "launcher.ini", zone), "save a dead zone");
        SdlLauncherOptions restored = {};
        restored.joystickDeadZone = SDL_JOYSTICK_DEAD_ZONE_ORIGINAL;
        load_launcher_config(fixture / "launcher.ini", restored);
        require(restored.joystickDeadZone == 5, "the dead zone survives a restart");
        std::ofstream(fixture / "launcher.ini", std::ios::app) << "joystick_dead_zone=34\n";
        load_launcher_config(fixture / "launcher.ini", restored);
        require(restored.joystickDeadZone == 5, "a dead zone past 33% is ignored");
        require(window->get_joystick_dead_zone() == SDL_JOYSTICK_DEAD_ZONE_ORIGINAL &&
                window->get_calibration_available(), "the original dead zone; WC2 data can be calibrated");
        const auto describe = [&window](int percent) {
            return std::string(window->global<DeadZoneText>().invoke_describe(percent));
        };
        require(describe(22) == "22% (original)" && describe(5) == "5%", "the launcher names the original dead zone");
        require(slint::select_bundled_translation("es") && describe(22) == "22% (original)" &&
                slint::select_bundled_translation("pl") && describe(22) == "22% (oryginalna)" &&
                describe(5) == "5%" && slint::select_bundled_translation("en"), "and translates it");
    }

    // The calibration page takes CalibrateJoystickInteractive's three
    // positions from an SDL virtual flight stick and writes j.cal for the
    // game.
    {
        CalibrationSession session(window);
        session.show(fixture, -1);
        require(window->get_page() == 2, "Calibrate… replaces the tabs with the calibration page");
        const int index = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_FLIGHT_STICK, 2, 2, 0);
        SDL_Joystick *stick = index >= 0 ? SDL_JoystickOpen(index) : nullptr;
        require(stick != nullptr, "attach a virtual flight stick");
        const auto move = [&session, &window](SDL_Joystick *which, Sint16 x, Sint16 y, bool button) {
            SDL_JoystickSetVirtualAxis(which, 0, x);
            SDL_JoystickSetVirtualAxis(which, 1, y);
            SDL_JoystickSetVirtualButton(which, 0, button);
            session.tick();
            return window->get_calibration_stick();
        };
        const auto device = [&window](size_t slot) {
            return std::string(window->get_calibration_devices()->row_data(slot).value_or(slint::SharedString()));
        };
        move(stick, 0, 0, false);
        require(window->get_calibration_connected() && !window->get_calibration_stored() &&
                !device(0).empty() && device(1).empty(),
                "the page finds joystick 1 and uses its full range without j.cal");
        require(move(stick, 7277, -7279, false).step_x == 2 && window->get_calibration_stick().step_y == -2 &&
                move(stick, 7276, 0, false).step_x == 0 &&
                window->get_calibration_stick().zone_right == 32767 + 7278,
                "it reads the stick as the game does, through the original dead zone");
        session.calibrate();
        require(window->get_calibration_step() == CalibrationStep::UpperLeft, "calibration starts at the upper left");
        move(stick, -30000, -29000, false);
        move(stick, -28000, -31000, false);
        move(stick, -28000, -31000, true);
        require(window->get_calibration_step() == CalibrationStep::LowerRight &&
                window->get_calibration_stick().left == 2768 && window->get_calibration_stick().top == 1768,
                "a button press takes the furthest upper-left position reached");
        move(stick, 30000, 29000, true);
        require(window->get_calibration_step() == CalibrationStep::LowerRight, "a held button records nothing more");
        move(stick, 29500, 30500, false);
        session.record();
        require(window->get_calibration_step() == CalibrationStep::Centre, "Record position takes the lower right");
        move(stick, 100, -200, false);
        move(stick, 100, -200, true);
        const auto path = joystick_calibration_path(fixture);
        const auto stored = read_joystick_calibration(path);
        require(window->get_calibration_step() == CalibrationStep::Idle &&
                window->get_calibration_message() == CalibrationMessage::Saved &&
                window->get_calibration_stored() && path == fixture / "GAMEDAT" / "j.cal" && stored &&
                *stored == JoystickCalibration {0, 2768, 1768, 62768, 63268, 32868, 32568},
                "j.cal holds the joystick, both corners' extremes and the centre");
        // Each side now has its own steps: 30100 / 9 below the centre.
        require(move(stick, 100, 0, false).step_x == 0 && move(stick, -29996, 0, false).step_x == -9 &&
                move(stick, -6588, 0, false).step_x == -2 && move(stick, -6587, 0, false).step_x == 0,
                "the saved calibration sets each side");
        window->set_joystick_dead_zone(5);
        require(move(stick, 1594, 0, false).step_x == 0 && move(stick, 1595, 0, false).step_x == 1 &&
                window->get_calibration_stick().zone_right == 32868 + 1494,
                "a 5% dead zone starts step one just past its edge");
        window->set_joystick_dead_zone(22);
        // As in the original, a calibration whose corners do not straddle the
        // centre fails and keeps the previous one.
        move(stick, 0, 0, false);
        session.calibrate();
        for (int press = 0; press < 3; ++press) {
            move(stick, 0, 0, true);
            move(stick, 0, 0, false);
        }
        require(window->get_calibration_message() == CalibrationMessage::Failed &&
                read_joystick_calibration(path) == stored, "a stick that never moved fails to calibrate");

        // WC2 lets the launcher choose the joystick: the game makes it
        // joystick 1 and ignores a calibration for joystick 2, so the page
        // offers only the chosen one.
        const int secondIndex = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_FLIGHT_STICK, 2, 2, 0);
        SDL_Joystick *second = secondIndex >= 0 ? SDL_JoystickOpen(secondIndex) : nullptr;
        require(second != nullptr, "attach a second virtual flight stick");
        session.tick();
        require(!device(0).empty() && !device(1).empty(), "without a choice, both joysticks are offered");
        session.show(fixture, SDL_JoystickInstanceID(second));
        SDL_JoystickSetVirtualAxis(stick, 0, -20000);
        require(device(1).empty() && window->get_calibration_device() == 0 &&
                move(second, 20000, 0, false).x == 52768, "the chosen joystick is joystick 1, and the only one");
        session.calibrate();
        move(second, -30000, -30000, true);
        move(second, -30000, -30000, false);
        move(second, 30000, 30000, true);
        move(second, 30000, 30000, false);
        move(second, 0, 0, true);
        const auto chosen = read_joystick_calibration(path);
        require(window->get_calibration_message() == CalibrationMessage::Saved && chosen && (*chosen)[0] == 0,
                "its calibration is saved for joystick 1");
        session.show(fixture, -1);
        SDL_JoystickClose(second);
        SDL_JoystickDetachVirtual(secondIndex);

        window->invoke_calibration_reset();
        require(window->get_calibration_message() == CalibrationMessage::Removed &&
                !window->get_calibration_stored() && !fs::exists(path), "Reset removes j.cal");
        SDL_JoystickClose(stick);
        SDL_JoystickDetachVirtual(index);
        session.tick();
        require(!window->get_calibration_connected() && device(0).empty(),
                "an unplugged stick leaves the page waiting");
        session.hide();
        require(window->get_page() == 0, "Back returns to the tabs");
    }

    if (argc > 1) {
        window->show();
        const auto save = [&window, view, argv](int width, int height, float scale, const std::string &suffix) {
            view->dimensions = slint::PhysicalSize {{static_cast<uint32_t>(width), static_cast<uint32_t>(height)}};
            window->window().dispatch_scale_factor_change_event(scale);
            window->window().dispatch_resize_event(slint::LogicalSize {{width / scale, height / scale}});
            // Let the tabs and buttons finish their animations.
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            slint::platform::update_timers_and_animations();
            std::vector<slint::Rgb8Pixel> pixels(static_cast<size_t>(width) * height);
            view->pixels.render(pixels, width);
            SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(
                pixels.data(), width, height, 24, width * 3, SDL_PIXELFORMAT_RGB24);
            const std::string path = std::string(argv[1]) + suffix;
            require(surface && SDL_SaveBMP(surface, path.c_str()) == 0, "save a preview");
            SDL_FreeSurface(surface);
        };
        window->set_has_so1(true);
        window->set_has_so2(true);
        for (const std::string language : {"en", "es", "pl"}) {
            require(slint::select_bundled_translation(language), "translated launcher");
            for (int tab = 0; tab < 4; ++tab) {
                window->set_settings_tab(tab);
                save(800, 660, 1.0f, "-" + language + "-tab" + std::to_string(tab) + ".bmp");
            }
            window->set_settings_tab(2);
            for (int mode = 1; mode < 3; ++mode) {
                window->set_joystick_mode_index(mode);
                save(800, 660, 1.0f, "-" + language + "-pad" + std::to_string(mode) + ".bmp");
            }
            window->set_joystick_mode_index(0);
            window->set_settings_tab(3);
            for (int page = 1; page <= 3; ++page) {
                window->window().dispatch_pointer_scroll_event(slint::LogicalPosition {{400, 380}}, 0, -440);
                save(800, 660, 1.0f, "-" + language + "-keys" + std::to_string(page) + ".bmp");
            }
            window->window().dispatch_pointer_scroll_event(slint::LogicalPosition {{400, 380}}, 0, 8000);
            window->set_page(2);
            save(800, 660, 1.0f, "-" + language + "-calibration.bmp");
            window->set_page(0);
        }
        require(slint::select_bundled_translation("en"), "the launcher returns to English");
        // The Steam Deck's screen at the scale gamescope's launcher takes.
        for (int tab = 0; tab < 4; ++tab) {
            window->set_settings_tab(tab);
            save(1280, 800, 1.2f, "-deck-tab" + std::to_string(tab) + ".bmp");
        }
        window->set_page(2);
        save(1280, 800, 1.2f, "-deck-calibration.bmp");
        window->set_page(0);
        // A window smaller than the pages scrolls.
        window->set_settings_tab(2);
        save(640, 480, 1.0f, "-small.bmp");
        window->hide();
    }
    fs::remove_all(fixture, error);
    std::puts("PASS: the tabbed launcher shows WC2's controller and keyboard bindings and saves changes,"
              " a German WC2.EXE limits the language menu to German, the music choices fit the"
              " installation and are saved, the dead zone is saved, and the calibration page"
              " calibrates virtual sticks, the chosen one as joystick 1.");
    return 0;
}
