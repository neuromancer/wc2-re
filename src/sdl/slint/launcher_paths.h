#ifndef WC2_SLINT_LAUNCHER_PATHS_H
#define WC2_SLINT_LAUNCHER_PATHS_H
#include "launcher.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace wc2_launcher {

struct DirectoryStatus {
    bool valid;
    LauncherMessage message;
    std::string detail = {};
};

struct GameInstallation {
    std::string directory;
    LauncherMessage edition;
};

std::string path_to_utf8(const std::filesystem::path &path);
std::filesystem::path path_from_utf8(std::string_view value);
DirectoryStatus validate_game_directory(std::string_view directoryText);
// Add only validated data roots, resolving aliases to avoid duplicate choices.
// Returns the existing/new entry's index, or -1 for an invalid directory.
int add_game_installation(std::vector<GameInstallation> &installations,
                          std::string_view directory);
std::vector<GameInstallation> discover_game_installations();

} // namespace wc2_launcher

#endif
