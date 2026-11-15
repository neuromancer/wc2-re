#include "launcher.h"
#include "launcher_api.h"
#include "launcher_config.h"
#include "launcher_joystick.h"
#include "launcher_paths.h"
#include "wc2_logo.h"

#include <nfd.h>
#include <slint.h>
#include <SDL.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

namespace fs = std::filesystem;

using namespace wc2_launcher;

struct LauncherJoystick {
    SDL_JoystickID instanceId;
    SDL_JoystickGUID guid;
    std::string path;
};

int supported_language(int language)
{
    return language == SDL_LAUNCHER_LANGUAGE_SPANISH ||
                   language == SDL_LAUNCHER_LANGUAGE_POLISH
               ? language
               : SDL_LAUNCHER_LANGUAGE_ENGLISH;
}

// Slint bundles translations/<locale>/LC_MESSAGES/launcher.po.
const char *launcher_locale(int language)
{
    switch (supported_language(language)) {
    case SDL_LAUNCHER_LANGUAGE_SPANISH:
        return "es";
    case SDL_LAUNCHER_LANGUAGE_POLISH:
        return "pl";
    default:
        return "en";
    }
}

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

void set_language(LauncherWindow &window, int language)
{
    // The German patch's own text replaces what Spanish and Polish translate.
    if (window.get_german_text())
        language = SDL_LAUNCHER_LANGUAGE_ENGLISH;
    const auto unnamed = window.get_unnamed_joystick();
    if (!slint::select_bundled_translation(launcher_locale(language)))
        return;
    window.set_language_index(supported_language(language));
    auto devices = window.get_joystick_devices();
    for (size_t i = 0; devices && i < devices->row_count(); ++i) {
        const auto value = devices->row_data(i);
        if (i == 0)
            devices->set_row_data(i, window.get_automatic_joystick());
        else if (value && std::string(*value) == std::to_string(i) + " — " + std::string(unnamed))
            devices->set_row_data(i, slint::SharedString(std::to_string(i) + " — " + std::string(window.get_unnamed_joystick())));
    }
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
    window.set_has_so2(status.valid && validate_game_directory(directory, 2).valid);
    window.set_german_text(status.valid && status.germanText);
    if (window.get_german_text() && window.get_language_index() != SDL_LAUNCHER_LANGUAGE_ENGLISH)
        set_language(window, SDL_LAUNCHER_LANGUAGE_ENGLISH);
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

using KeyboardBindings = slint::VectorModel<std::shared_ptr<slint::Model<InputBinding>>>;

/* The keyboard and mouse tab's rows and the bindings shown for them. */
struct BindingTable {
    std::shared_ptr<slint::Model<InputRow>> rows;
    std::shared_ptr<KeyboardBindings> bindings;
};

BindingTable binding_table(const LauncherWindow &controls)
{
    return {controls.get_keyboard_rows(),
            std::dynamic_pointer_cast<KeyboardBindings>(controls.get_keyboard_bindings())};
}

/* A copy of the original bindings, row by row, that the keyboard and mouse
   tab can change. */
std::shared_ptr<KeyboardBindings> default_bindings(const slint::Model<InputRow> &rows)
{
    auto table = std::make_shared<KeyboardBindings>();
    for (size_t row = 0; row < rows.row_count(); ++row) {
        std::vector<InputBinding> bindings;
        const auto data = rows.row_data(row);
        if (data && data->bindings) {
            for (size_t slot = 0; slot < data->bindings->row_count(); ++slot)
                bindings.push_back(data->bindings->row_data(slot).value_or(InputBinding {}));
        }
        table->push_back(std::make_shared<slint::VectorModel<InputBinding>>(std::move(bindings)));
    }
    return table;
}

/* Input names are stored percent-encoded outside a-z, 0-9 and "-", so "+"
   joins a modifier to its key and "," separates bindings (see
   src/sdl/keymap.c). */
std::string encode_input(std::string_view input)
{
    static constexpr char digits[] = "0123456789abcdef";
    std::string text;
    for (const unsigned char character : input) {
        if ((character >= 'a' && character <= 'z') || (character >= '0' && character <= '9') ||
            character == '-') {
            text += static_cast<char>(character);
        } else {
            text += '%';
            text += digits[character >> 4];
            text += digits[character & 15];
        }
    }
    return text;
}

std::optional<std::string> decode_input(std::string_view text)
{
    std::string input;
    for (size_t index = 0; index < text.size(); ++index) {
        if (text[index] != '%') {
            input += text[index];
            continue;
        }
        unsigned int byte = 0;
        if (index + 2 >= text.size() ||
            std::from_chars(text.data() + index + 1, text.data() + index + 3, byte, 16).ptr !=
                text.data() + index + 3)
            return std::nullopt;
        input += static_cast<char>(byte);
        index += 2;
    }
    if (input.empty())
        return std::nullopt;
    return input;
}

std::string encode_binding(const InputBinding &binding)
{
    std::string text;
    for (size_t part = 0; binding.inputs && part < binding.inputs->row_count(); ++part) {
        if (part > 0)
            text += '+';
        text += encode_input(std::string_view(binding.inputs->row_data(part).value_or(
            slint::SharedString())));
    }
    return text;
}

std::string encode_bindings(const slint::Model<InputBinding> &bindings)
{
    std::string text;
    for (size_t slot = 0; slot < bindings.row_count(); ++slot) {
        if (slot > 0)
            text += ',';
        text += encode_binding(bindings.row_data(slot).value_or(InputBinding {}));
    }
    return text;
}

/* The rows whose bindings differ from the original keys, as the launcher
   settings' [keys] lines. */
std::string changed_key_bindings(const LauncherWindow &controls)
{
    std::string text;
    const auto table = binding_table(controls);
    for (size_t row = 0; table.bindings && row < table.rows->row_count() &&
                         row < table.bindings->row_count(); ++row) {
        const auto data = table.rows->row_data(row);
        const auto current = table.bindings->row_data(row).value_or(nullptr);
        if (!data || data->id.empty() || data->fixed || !data->bindings || !current)
            continue;
        const auto value = encode_bindings(*current);
        if (value != encode_bindings(*data->bindings))
            text += std::string(std::string_view(data->id)) + '=' + value + '\n';
    }
    return text;
}

/* How many bindings a row takes: its own limit, or the tab's. */
size_t binding_limit(const LauncherWindow &controls, const InputRow &row)
{
    return static_cast<size_t>(row.limit > 0 ? row.limit : controls.get_max_bindings());
}

/* The index of the row with this id, or -1. */
int table_row(const BindingTable &table, std::string_view id)
{
    for (size_t row = 0; row < table.rows->row_count(); ++row) {
        const auto data = table.rows->row_data(row);
        if (data && std::string_view(data->id) == id)
            return static_cast<int>(row);
    }
    return -1;
}

/* One row's current bindings, if it exists. */
std::shared_ptr<slint::VectorModel<InputBinding>> table_bindings(const BindingTable &table, int row)
{
    if (!table.bindings || row < 0 || static_cast<size_t>(row) >= table.bindings->row_count())
        return nullptr;
    return std::dynamic_pointer_cast<slint::VectorModel<InputBinding>>(
        table.bindings->row_data(row).value_or(nullptr));
}

/* The missile and roll rows show the game's two mouse buttons: whatever the
   guns and afterburner rows hold now. */
void refresh_mouse_rows(const BindingTable &table)
{
    const auto button = [&table](std::string_view id) -> std::optional<slint::SharedString> {
        const auto bindings = table_bindings(table, table_row(table, id));
        if (!bindings || bindings->row_count() == 0)
            return std::nullopt;
        const auto binding = bindings->row_data(0).value_or(InputBinding {});
        if (!binding.inputs || binding.inputs->row_count() == 0)
            return std::nullopt;
        return binding.inputs->row_data(0);
    };
    const auto gesture = [](std::vector<slint::SharedString> inputs) {
        InputBinding binding;
        binding.inputs = std::make_shared<slint::VectorModel<slint::SharedString>>(std::move(inputs));
        binding.joiner = "+";
        binding.fixed = true;
        return std::vector<InputBinding> {binding};
    };
    const auto guns = button("mouse-guns");
    const auto afterburner = button("mouse-afterburner");
    const auto missile = table_row(table, "mouse-missile");
    const auto roll = table_row(table, "mouse-roll");
    if (!table.bindings || missile < 0 || roll < 0)
        return;
    table.bindings->set_row_data(missile, std::make_shared<slint::VectorModel<InputBinding>>(
        guns && afterburner ? gesture({*guns, *afterburner}) : std::vector<InputBinding> {}));
    table.bindings->set_row_data(roll, std::make_shared<slint::VectorModel<InputBinding>>(
        afterburner ? gesture({*afterburner, "mouse-move"}) : std::vector<InputBinding> {}));
}

/* Puts saved [keys] lines into the window's current bindings. A saved
   binding that matches a fixed original one keeps it, with its hint. */
void apply_key_bindings(const LauncherWindow &controls, std::string_view saved)
{
    const auto table = binding_table(controls);
    const auto &rows = table.rows;
    while (table.bindings && !saved.empty()) {
        const auto end = saved.find('\n');
        const auto line = saved.substr(0, end);
        saved = end == std::string_view::npos ? std::string_view() : saved.substr(end + 1);
        const auto separator = line.find('=');
        if (separator == std::string_view::npos)
            continue;
        const auto id = line.substr(0, separator);
        auto value = line.substr(separator + 1);
        for (size_t row = 0; row < rows->row_count() && row < table.bindings->row_count(); ++row) {
            const auto data = rows->row_data(row);
            if (!data || data->fixed || std::string_view(data->id) != id)
                continue;
            const auto limit = binding_limit(controls, *data);
            const bool unbound = value.empty();
            std::vector<InputBinding> bindings;
            while (!value.empty() && bindings.size() < limit) {
                const auto comma = value.find(',');
                auto text = value.substr(0, comma);
                value = comma == std::string_view::npos ? std::string_view() : value.substr(comma + 1);
                std::vector<slint::SharedString> inputs;
                while (!text.empty()) {
                    const auto plus = text.find('+');
                    if (const auto input = decode_input(text.substr(0, plus)))
                        inputs.emplace_back(*input);
                    text = plus == std::string_view::npos ? std::string_view() : text.substr(plus + 1);
                }
                const auto mouse = [](const slint::SharedString &input) {
                    const std::string_view name(input);
                    return name.starts_with("mouse-") || name.starts_with("wheel-");
                };
                if (inputs.empty() || inputs.size() > 2 ||
                    (data->mouse_only && (inputs.size() > 1 || !std::string_view(inputs[0]).starts_with("mouse-"))) ||
                    (inputs.size() > 1 && (mouse(inputs[0]) || mouse(inputs[1]))))
                    continue;
                InputBinding binding;
                binding.inputs = std::make_shared<slint::VectorModel<slint::SharedString>>(inputs);
                binding.joiner = inputs.size() > 1 ? "+" : "";
                for (size_t slot = 0; data->bindings && slot < data->bindings->row_count(); ++slot) {
                    const auto original = data->bindings->row_data(slot).value_or(InputBinding {});
                    if (original.fixed && encode_binding(original) == encode_binding(binding))
                        binding = original;
                }
                bindings.push_back(binding);
            }
            /* An empty value leaves the input unbound; nothing valid in a
               value leaves the row as it was. */
            if (unbound || !bindings.empty())
                table.bindings->set_row_data(row, std::make_shared<slint::VectorModel<InputBinding>>(std::move(bindings)));
            break;
        }
    }
    refresh_mouse_rows(table);
}

/* The keyboard and mouse tab's bindings start from the saved ones, and
   changed_key_bindings() reads them back for the game. */
void prepare_controls(const slint::ComponentHandle<LauncherWindow> &window,
                      std::string_view savedBindings = {})
{
#ifdef __APPLE__
    window->set_apple_keys(true);
#endif
    window->set_keyboard_bindings(default_bindings(*window->get_keyboard_rows()));
    apply_key_bindings(*window, savedBindings);
    window->on_binding_changed(
        [self = slint::ComponentWeakHandle<LauncherWindow>(window)](
            int row, int slot, InputBinding binding) {
            const auto launcher = self.lock();
            if (!launcher)
                return;
            const auto table = binding_table(**launcher);
            const auto bindings = table_bindings(table, row);
            const auto data = table.rows->row_data(row);
            if (!bindings || !data || slot < 0)
                return;
            /* The guns and afterburner buttons cannot share a button: giving
               one the other's swaps them. */
            const std::string id(std::string_view(data->id));
            if (id == "mouse-guns" || id == "mouse-afterburner") {
                const auto other = table_bindings(
                    table, table_row(table, id == "mouse-guns" ? "mouse-afterburner" : "mouse-guns"));
                if (other && other->row_count() > 0 &&
                    encode_binding(other->row_data(0).value_or(InputBinding {})) == encode_binding(binding)) {
                    if (bindings->row_count() > 0)
                        other->set_row_data(0, bindings->row_data(0).value_or(InputBinding {}));
                    else
                        other->erase(0);
                }
            }
            if (static_cast<size_t>(slot) < bindings->row_count())
                bindings->set_row_data(slot, binding);
            else if (bindings->row_count() < binding_limit(**launcher, *data))
                bindings->push_back(binding);
            refresh_mouse_rows(table);
        });
    window->on_binding_removed(
        [self = slint::ComponentWeakHandle<LauncherWindow>(window)](int row, int slot) {
            const auto launcher = self.lock();
            if (!launcher)
                return;
            const auto table = binding_table(**launcher);
            const auto bindings = table_bindings(table, row);
            if (bindings && slot >= 0 && static_cast<size_t>(slot) < bindings->row_count())
                bindings->erase(slot);
            refresh_mouse_rows(table);
        });
    window->on_bindings_reset(
        [self = slint::ComponentWeakHandle<LauncherWindow>(window)]() {
            const auto launcher = self.lock();
            if (!launcher)
                return;
            (*launcher)->set_keyboard_bindings(
                default_bindings(*(*launcher)->get_keyboard_rows()));
        });
}

#ifdef __linux__
/* Gamescope, the compositor of Steam's Game Mode and other gaming sessions,
   shows one window at a time, scaled to fit its output. A launcher larger
   than the output was shrunk and left beyond the X pointer's reach, so the
   cursor missed the buttons (issue #65); there the launcher fills the output.
   Gamescope sets both variables for the programs it runs. */
bool running_under_gamescope()
{
    const char *desktop = std::getenv("XDG_CURRENT_DESKTOP");

    return std::getenv("GAMESCOPE_WAYLAND_DISPLAY") != nullptr ||
           (desktop != nullptr && std::string_view(desktop) == "gamescope");
}

/* Winit takes the scale from the panel's physical size, about 2.25 on the
   Steam Deck, where the pages would not fit its 1280x800 screen. The scale
   that fits the launcher's size (launcher.slint) to the output replaces it,
   unless SLINT_SCALE_FACTOR already chooses one. */
void fit_scale_to_output()
{
    constexpr double pageWidth = 800.0;
    constexpr double pageHeight = 660.0;
    SDL_DisplayMode mode;

    if (std::getenv("SLINT_SCALE_FACTOR") != nullptr || SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
        return;
    if (SDL_GetDesktopDisplayMode(0, &mode) == 0 && mode.w > 0 && mode.h > 0) {
        const double scale = std::clamp(std::min(mode.w / pageWidth, mode.h / pageHeight), 0.5, 4.0);
        char text[16];

        std::snprintf(text, sizeof(text), "%.2f", std::floor(scale * 20.0) / 20.0);
        setenv("SLINT_SCALE_FACTOR", text, 0);
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
#endif

int dead_zone_percent(float value)
{
    return std::clamp(static_cast<int>(std::lround(value)), 0, SDL_JOYSTICK_DEAD_ZONE_MAXIMUM);
}

/* The calibration page: CalibrateJoystickInteractive's upper-left,
   lower-right and centre positions (0x418770), read from SDL while the page
   shows the stick, the calibration in use and what the game reads through the
   dead zone. A corner takes the furthest position reached before the button,
   so a round gate's edges count; the original took the position at the press.
   After wc1-re's; a joystick chosen on the Joystick tab is joystick 1. */
class CalibrationSession {
public:
    explicit CalibrationSession(const slint::ComponentHandle<LauncherWindow> &launcher)
        : window(launcher)
    {
        window->on_calibration_start([this]() { calibrate(); });
        window->on_calibration_record([this]() { record(); });
        window->on_calibration_cancel([this]() {
            window->set_calibration_step(CalibrationStep::Idle);
            window->set_calibration_message(CalibrationMessage::None);
            show_view();
        });
        window->on_calibration_reset([this]() {
            std::string error;
            if (remove_joystick_calibration(path, error)) {
                stored.reset();
                window->set_calibration_message(CalibrationMessage::Removed);
            } else {
                window->set_calibration_message(CalibrationMessage::CannotRemove);
                window->set_calibration_message_detail(slint::SharedString(error));
            }
            show_view();
        });
        window->on_calibration_device_selected([this](int) {
            window->set_calibration_message(CalibrationMessage::None);
            buttonHeld = true;
            show_view();
        });
    }

    // preferred is the joystick chosen on the Joystick tab, or -1.
    void show(const fs::path &gameDirectory, SDL_JoystickID preferred)
    {
        path = joystick_calibration_path(gameDirectory);
        stored = path.empty() ? std::nullopt : read_joystick_calibration(path);
        window->set_calibration_step(CalibrationStep::Idle);
        window->set_calibration_message(CalibrationMessage::None);
        joysticks.prefer(preferred);
        if (preferred >= 0)
            window->set_calibration_device(0);
        else if (stored && (*stored)[0] < 2)
            window->set_calibration_device((*stored)[0]);
        if (!joysticks.start()) {
            window->set_calibration_message(CalibrationMessage::Unavailable);
            window->set_calibration_message_detail(slint::SharedString(SDL_GetError()));
        }
        buttonHeld = true;
        tick();
        timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(16), [this]() { tick(); });
        window->set_page(2);
    }

    // Stops reading the joysticks and leaves the page for the settings.
    void hide()
    {
        timer.stop();
        window->set_calibration_step(CalibrationStep::Idle);
        joysticks.stop();
        if (window->get_page() == 2)
            window->set_page(0);
    }

    // One reading of the joystick; the page's timer calls it.
    void tick()
    {
        joysticks.update();
        const auto names = joysticks.names();
        if (names != shownNames) {
            shownNames = names;
            window->set_calibration_devices(std::make_shared<slint::VectorModel<slint::SharedString>>(
                std::vector<slint::SharedString> {slint::SharedString(names[0]), slint::SharedString(names[1])}));
        }
        position = joysticks.read(slot());
        window->set_calibration_connected(position.connected);
        if (!position.connected) {
            window->set_calibration_step(CalibrationStep::Idle);
            buttonHeld = true;
            show_view();
            return;
        }
        const auto step = window->get_calibration_step();
        if (step == CalibrationStep::UpperLeft) {
            minimumX = std::min(minimumX, position.x);
            minimumY = std::min(minimumY, position.y);
        } else if (step == CalibrationStep::LowerRight) {
            maximumX = std::max(maximumX, position.x);
            maximumY = std::max(maximumY, position.y);
        }
        // As WaitForJoystickButtonPress, after WaitForJoystickButtonRelease.
        const bool pressed = position.button && !buttonHeld;
        buttonHeld = position.button;
        if (pressed && step != CalibrationStep::Idle)
            record();
        else
            show_view();
    }

    void calibrate()
    {
        if (!position.connected)
            return;
        window->set_calibration_message(CalibrationMessage::None);
        minimumX = position.x;
        minimumY = position.y;
        buttonHeld = position.button;
        window->set_calibration_step(CalibrationStep::UpperLeft);
        show_view();
    }

    void record()
    {
        switch (window->get_calibration_step()) {
        case CalibrationStep::UpperLeft:
            minimumX = std::min(minimumX, position.x);
            minimumY = std::min(minimumY, position.y);
            maximumX = position.x;
            maximumY = position.y;
            window->set_calibration_step(CalibrationStep::LowerRight);
            break;
        case CalibrationStep::LowerRight:
            maximumX = std::max(maximumX, position.x);
            maximumY = std::max(maximumY, position.y);
            window->set_calibration_step(CalibrationStep::Centre);
            break;
        case CalibrationStep::Centre:
            window->set_calibration_step(CalibrationStep::Idle);
            save(position.x, position.y);
            break;
        default:
            break;
        }
        show_view();
    }

private:
    slint::ComponentHandle<LauncherWindow> window;
    JoystickReader joysticks;
    slint::Timer timer;
    fs::path path;
    std::optional<JoystickCalibration> stored;
    std::array<std::string, 2> shownNames;
    JoystickPosition position;
    int minimumX = 0;
    int minimumY = 0;
    int maximumX = 0;
    int maximumY = 0;
    bool buttonHeld = true;

    int slot() const
    {
        return std::clamp(window->get_calibration_device(), 0, 1);
    }

    // The original checked only that each maximum exceeds its minimum.
    void save(int centreX, int centreY)
    {
        if (!SdlJoystickAxisCalibrationValid(minimumX, centreX, maximumX) ||
            !SdlJoystickAxisCalibrationValid(minimumY, centreY, maximumY)) {
            window->set_calibration_message(CalibrationMessage::Failed);
            return;
        }
        const JoystickCalibration calibration = {
            static_cast<std::uint16_t>(slot()),
            static_cast<std::uint16_t>(minimumX), static_cast<std::uint16_t>(minimumY),
            static_cast<std::uint16_t>(maximumX), static_cast<std::uint16_t>(maximumY),
            static_cast<std::uint16_t>(centreX), static_cast<std::uint16_t>(centreY)
        };
        std::string error;
        if (path.empty() || !write_joystick_calibration(path, calibration, error)) {
            window->set_calibration_message(CalibrationMessage::CannotSave);
            window->set_calibration_message_detail(slint::SharedString(error));
            return;
        }
        stored = calibration;
        window->set_calibration_message(CalibrationMessage::Saved);
    }

    // SdlAcceptJoystickCalibration's test, for the joystick shown.
    bool calibration_in_use() const
    {
        return stored && (*stored)[0] == slot() &&
               SdlJoystickAxisCalibrationValid((*stored)[1], (*stored)[5], (*stored)[3]) &&
               SdlJoystickAxisCalibrationValid((*stored)[2], (*stored)[6], (*stored)[4]);
    }

    void show_view()
    {
        SdlJoystickAxis x;
        SdlJoystickAxis y;
        const bool calibrated = calibration_in_use();
        if (calibrated) {
            const auto &words = *stored;
            SdlCalibrateJoystickAxis(&x, words[1], words[5], words[3], SDL_JOYSTICK_STEPS);
            SdlCalibrateJoystickAxis(&y, words[2], words[6], words[4], SDL_JOYSTICK_STEPS);
        } else {
            SdlDefaultJoystickAxis(&x);
            SdlDefaultJoystickAxis(&y);
        }
        const int percent = dead_zone_percent(window->get_joystick_dead_zone());
        const int steps = SdlJoystickDeadZoneSteps(percent);
        const auto step = window->get_calibration_step();
        StickView view;
        view.x = position.x;
        view.y = position.y;
        // While recording, the outline follows the extremes reached.
        view.left = step != CalibrationStep::Idle ? minimumX : x.minimum;
        view.top = step != CalibrationStep::Idle ? minimumY : y.minimum;
        view.right = step == CalibrationStep::LowerRight || step == CalibrationStep::Centre ? maximumX : x.maximum;
        view.bottom = step == CalibrationStep::LowerRight || step == CalibrationStep::Centre ? maximumY : y.maximum;
        view.centre_x = x.centre;
        view.centre_y = y.centre;
        view.zone_left = x.centre - SdlJoystickDeadZoneEdge(x.centre - x.minimum, x.lowScale, percent);
        view.zone_right = x.centre + SdlJoystickDeadZoneEdge(x.maximum - x.centre, x.highScale, percent);
        view.zone_top = y.centre - SdlJoystickDeadZoneEdge(y.centre - y.minimum, y.lowScale, percent);
        view.zone_bottom = y.centre + SdlJoystickDeadZoneEdge(y.maximum - y.centre, y.highScale, percent);
        view.step_x = SdlJoystickAxisStep(&x, SdlShapeJoystickPosition(&x, position.x, percent), steps);
        view.step_y = SdlJoystickAxisStep(&y, SdlShapeJoystickPosition(&y, position.y, percent), steps);
        window->set_calibration_stick(view);
        window->set_calibration_stored(calibrated);
    }
};

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

#ifdef __linux__
    const bool gamescope = running_under_gamescope();
    if (gamescope)
        fit_scale_to_output();
#endif
    auto window = LauncherWindow::create();
#ifdef __linux__
    if (gamescope)
        window->window().set_fullscreen(true);
#endif
    slint::select_bundled_translation(launcher_locale(options->language));
    // The keyboard and mouse tab takes the saved bindings when it first opens.
    bool controlsReady = false;
    // The calibration page reads joysticks only while it shows.
    std::unique_ptr<CalibrationSession> calibration;
    const auto stop_calibration = [&calibration]() {
        if (calibration)
            calibration->hide();
    };
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
    window->set_joystick_dead_zone(static_cast<float>(options->joystickDeadZone));
    // Keep the subsystem alive through the SDL handoff so IDs stay valid.
    SDL_SetMainReady();
    const bool joystickInitialized = SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0;
    std::vector<LauncherJoystick> joysticks;
    if (joystickInitialized)
        joysticks = list_joysticks(*window);
    refresh_window(*window, installations);

    window->on_language_selected([&window, &installations](int language) {
        set_language(*window, language);
        refresh_window(*window, installations);
    });

    window->on_installation_selected([&window, &installations, stop_calibration](int index) {
        if (index > 0 && static_cast<size_t>(index) <= installations.size())
            window->set_game_directory(slint::SharedString(installations[index - 1].directory));
        refresh_window(*window, installations);
        // Its j.cal belongs to the installation it was opened for.
        stop_calibration();
    });
    window->on_controls_requested([&window, &controlsReady, options]() {
        if (controlsReady)
            return;
        prepare_controls(window, options->keyBindings);
        controlsReady = true;
    });
    window->on_calibration_requested([&window, &calibration, &joysticks]() {
        if (!calibration)
            calibration = std::make_unique<CalibrationSession>(window);
        const int deviceIndex = window->get_joystick_device_index() - 1;
        const SDL_JoystickID preferred =
            deviceIndex >= 0 && static_cast<size_t>(deviceIndex) < joysticks.size()
                ? joysticks[deviceIndex].instanceId : -1;
        calibration->show(path_from_utf8(std::string(window->get_game_directory())), preferred);
    });
    window->on_calibration_closed(stop_calibration);
    window->window().on_close_requested([stop_calibration]() {
        stop_calibration();
        return slint::CloseRequestResponse::HideWindow;
    });
    window->on_launch_requested([&window, &accepted, &installations, &selectedCampaign,
                                 &controlsReady, options, stop_calibration](int campaign) {
        refresh_window(*window, installations);
        if (!window->get_path_valid() ||
            (campaign == 1 && !window->get_has_so1()) ||
            (campaign == 2 && !window->get_has_so2()) ||
            campaign < 0 || campaign > 2)
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
        /* The changed bindings go to the game and the launcher settings. */
        if (controlsReady) {
            const auto keyBindings = changed_key_bindings(*window);
            if (keyBindings.size() < sizeof(options->keyBindings))
                std::memcpy(options->keyBindings, keyBindings.c_str(), keyBindings.size() + 1);
            else
                std::fprintf(stderr, "Too many changed keys to keep.\n");
        }
        accepted = true;
        /* Hide while the event loop still runs: on Linux the request takes
           effect only when the loop processes it, so hiding after the loop
           left the launcher on screen during the game. */
        stop_calibration();
        window->hide();
        slint::quit_event_loop();
    });
    window->on_cancel_requested([stop_calibration]() {
        stop_calibration();
        slint::quit_event_loop();
    });

    window->show();
    /* NFD_Init records Cocoa's activation policy. Initialize it from the
     * callback, after winit launches NSApplication, so NFD_Quit cannot restore
     * the temporary pre-launch Prohibited policy before SDL takes over. */
    window->on_browse_requested([&window, &pickerInitialized, &installations, stop_calibration]() {
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
        stop_calibration();
    });
    slint::run_event_loop();
    if (pickerInitialized)
        NFD_Quit();
    stop_calibration();
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
    options->joystickDeadZone = dead_zone_percent(window->get_joystick_dead_zone());
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
