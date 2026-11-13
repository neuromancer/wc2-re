/* Exercises the launcher's controls window on an offscreen Slint window, after
 * wc1-re's launcher check: the game controller's labels for each mode, the
 * keyboard and mouse table and the action list that the game shares
 * (src/sdl/keymap_actions.h), saving and reading back changed bindings, and
 * the guns and afterburner buttons' swap. With an argument it also saves BMP
 * previews named after it. */
#include "../src/sdl/slint/slint_static.h"
#include "../src/sdl/slint/launcher.cpp"
#include <slint-platform.h>
#include <SDL.h>
#include "../src/sdl/keymap_actions.h"
#include <chrono>
#include <cstdlib>
#include <thread>

struct TestWindow : slint::platform::WindowAdapter {
    slint::platform::SoftwareRenderer pixels {slint::platform::SoftwareRenderer::RepaintBufferType::NewBuffer};
    slint::PhysicalSize dimensions {{760, 660}};
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
    window->set_joystick_mode_index(0);
    window->set_joystick_axes_index(0);

    auto controlsWindow = create_controls_window(window);
    // Each window gets its own adapter; keep the one this window draws on.
    TestWindow *controlsView = platform->view;
    const auto binding = [&controlsWindow](PadControl control) {
        return std::string(controlsWindow->invoke_pad_binding(control));
    };
    controlsWindow->set_joystick_mode(0);
    require(binding(PadControl::A) == "fire guns" && binding(PadControl::B) == "hold to roll and change speed" &&
            binding(PadControl::RightTrigger) == "fire guns" &&
            binding(PadControl::LeftTrigger) == "hold to roll and change speed" &&
            binding(PadControl::DpadUp).empty() && binding(PadControl::Start) == "pause" &&
            binding(PadControl::Back) == "cycle views",
            "original two-button controller bindings, the triggers standing in for A and B");
    controlsWindow->set_joystick_mode(1);
    require(binding(PadControl::B) == "fire missile" && binding(PadControl::Y) == "hold to roll and change speed" &&
            binding(PadControl::DpadUp) == "full speed" && binding(PadControl::DpadDown) == "immediate stop" &&
            binding(PadControl::LeftShoulder) == "gun type" &&
            binding(PadControl::RightShoulder) == "weapon system" && binding(PadControl::RightStick).empty(),
            "WCAT two-axis controller bindings");
    controlsWindow->set_joystick_mode(2);
    controlsWindow->set_joystick_axes(2);
    require(binding(PadControl::Y) == "change target" && binding(PadControl::LeftStick) == "roll and pitch" &&
            binding(PadControl::RightStick) == "yaw, throttle position" &&
            binding(PadControl::LeftTrigger) == "fire missile" &&
            binding(PadControl::RightTrigger) == "fire guns", "WCAT four-axis HOTAS-yaw controller bindings");
    require(controlsWindow->get_mode_names()->row_count() == 3 && controlsWindow->get_axis_names()->row_count() == 7,
            "controls window layout names");

    // The window's own selectors follow the launcher's, and change them.
    controlsWindow->set_joystick_mode(1);
    controlsWindow->invoke_layout_changed();
    require(window->get_joystick_mode_index() == 1, "the controls window's selectors change the launcher's");

    // One table row per game input, each with its keys and mouse inputs.
    const auto rows = controlsWindow->get_keyboard_rows();
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
    const auto current = [&controlsWindow, &inputs](int row) {
        std::string text;
        const auto table = controlsWindow->get_keyboard_bindings();
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
            "the launcher copies the original bindings for the window");

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
        const auto limit = static_cast<size_t>(row && row->limit > 0 ? row->limit : controlsWindow->get_max_bindings());
        require(row && (!row->bindings || row->bindings->row_count() <= limit),
                "no game input starts with more bindings than it takes");
    }

    // Saved bindings come back as the same [keys] lines.
    // In the table's order: the mouse rows come before the wingman's.
    const std::string saved = "pitch-down=k\nroll-left=\nmouse-guns=mouse-right\n"
                              "mouse-afterburner=mouse-left\nattack-my-target=y\nfull-screen=f11\n";
    auto reopened = create_controls_window(window, saved);
    require(changed_key_bindings(*reopened) == saved, "saved bindings read back unchanged");
    require(changed_key_bindings(*controlsWindow).empty(), "an untouched window changes nothing");

    // Giving the guns row the afterburner's button swaps the two.
    const int gunsRow = find_row("mouse-guns");
    InputBinding right;
    right.inputs = std::make_shared<slint::VectorModel<slint::SharedString>>(
        std::vector<slint::SharedString> {"mouse-right"});
    controlsWindow->invoke_binding_changed(gunsRow, 0, right);
    require(current(gunsRow) == "mouse-right" && current(find_row("mouse-afterburner")) == "mouse-left" &&
            current(missileRow) == "mouse-right|mouse-left" && current(rollRow) == "mouse-left|mouse-move",
            "the guns and afterburner buttons swap");
    controlsWindow->invoke_bindings_reset();
    require(changed_key_bindings(*controlsWindow).empty(), "restore defaults resets every row");

    if (argc > 1) {
        controlsWindow->show();
        controlsView->dimensions = slint::PhysicalSize {{1040, 660}};
        controlsWindow->window().dispatch_resize_event(slint::LogicalSize {{1040, 660}});
        const auto save = [controlsView, argv](const std::string &suffix) {
            // Let the page buttons finish their colour animations.
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            slint::platform::update_timers_and_animations();
            std::vector<slint::Rgb8Pixel> pixels(1040 * 660);
            controlsView->pixels.render(pixels, 1040);
            SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(
                pixels.data(), 1040, 660, 24, 1040 * 3, SDL_PIXELFORMAT_RGB24);
            const std::string path = std::string(argv[1]) + suffix;
            require(surface && SDL_SaveBMP(surface, path.c_str()) == 0, "save a preview");
            SDL_FreeSurface(surface);
        };
        for (const std::string language : {"en", "es", "pl"}) {
            require(slint::select_bundled_translation(language), "translated controls window");
            for (int mode = 0; mode < 3; ++mode) {
                controlsWindow->set_joystick_mode(mode);
                controlsWindow->set_page(0);
                save("-" + language + "-pad" + std::to_string(mode) + ".bmp");
            }
            // The four-axis layout labels the right stick by its axis layout.
            for (int axes = 0; axes < 7; ++axes) {
                controlsWindow->set_joystick_axes(axes);
                save("-" + language + "-axes" + std::to_string(axes) + ".bmp");
            }
            controlsWindow->set_page(1);
            for (int page = 1; page <= 6; ++page) {
                save("-" + language + "-keys" + std::to_string(page) + ".bmp");
                controlsWindow->window().dispatch_pointer_scroll_event(
                    slint::LogicalPosition {{500, 300}}, 0, -440);
            }
            controlsWindow->window().dispatch_pointer_scroll_event(slint::LogicalPosition {{500, 300}}, 0, 8000);
        }
    }
    std::puts("PASS: the controls window shows WC2's controller and keyboard bindings and saves changes.");
    return 0;
}
