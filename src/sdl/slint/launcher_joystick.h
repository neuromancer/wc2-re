#ifndef WC2_SLINT_LAUNCHER_JOYSTICK_H
#define WC2_SLINT_LAUNCHER_JOYSTICK_H

#include "launcher_axes.h"
#include "../pad_bindings.h"

#include <SDL.h>

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace wc2_launcher {

// j.cal's words, in src/sdl/joystick_calibration.h's order.
using JoystickCalibration = std::array<std::uint16_t, 7>;

struct JoystickPosition {
    bool connected = false;
    // The game's positions, 0 to 65535.
    int x = 0;
    int y = 0;
    bool button = false;
};

// A joystick as the Joystick tab shows it: a mapped game controller's
// controls, or another joystick's buttons, hats and axes, numbered as
// src/sdl/pad_bindings.h numbers them.
struct PadSnapshot {
    bool connected = false;
    // SDL maps it as a game controller.
    bool mapped = false;
    // It calls itself a joystick, or SDL knows it as a flight stick or a
    // throttle.
    bool joystick = false;
    std::string name;
    int buttons = 0;
    int hats = 0;
    int axes = 0;
    // What each axis is; empty when the platform does not say.
    std::vector<AxisKind> axisKinds;
    // Each control held now; a mapped controller's sticks count as held
    // past half their travel.
    std::array<bool, SDL_PAD_CONTROLS> held {};
    // Another joystick's axes, -1 to 1.
    std::array<float, SDL_PAD_STICK_AXES> axisValues {};
};

// The first two SDL joysticks, numbered and read as the game does in
// src/sdl/joystick.c, for calibration and the live Joystick tab. After wc1-re's reader, with
// WC2's joystick choice: the chosen joystick is joystick 1.
class JoystickReader {
public:
    JoystickReader() = default;
    JoystickReader(const JoystickReader &) = delete;
    JoystickReader &operator=(const JoystickReader &) = delete;
    ~JoystickReader();

    // Starts SDL's joystick support; on failure SDL_GetError() says why.
    bool start();
    void stop();
    // The joystick chosen on the Joystick tab, or -1 for automatic.
    void prefer(SDL_JoystickID instance);
    // Read numbered inputs even when SDL also has a gamepad mapping.
    void use_raw(bool raw);
    // Reads the devices, opening new ones and closing those unplugged.
    void update();
    // Each slot's device name; empty for a free slot.
    std::array<std::string, 2> names() const;
    JoystickPosition read(int slot) const;
    PadSnapshot snapshot(int slot, bool steamDeck = false) const;

private:
    struct Device {
        SDL_GameController *controller = nullptr;
        SDL_Joystick *joystick = nullptr;
        SDL_JoystickID instance = -1;
        JoystickDescription description;
    };

    static void close(Device &device);
    void open(int index, Device &device);

    std::array<Device, 2> devices {};
    SDL_JoystickID preferred = -1;
    bool active = false;
    bool raw = false;
};

// GAMEDAT/j.cal in an installation, matching an existing file's case;
// empty without GAMEDAT.
std::filesystem::path joystick_calibration_path(const std::filesystem::path &gameDirectory);
std::optional<JoystickCalibration> read_joystick_calibration(const std::filesystem::path &path);
bool write_joystick_calibration(const std::filesystem::path &path,
                                const JoystickCalibration &calibration, std::string &error);
bool remove_joystick_calibration(const std::filesystem::path &path, std::string &error);

} // namespace wc2_launcher

#endif
