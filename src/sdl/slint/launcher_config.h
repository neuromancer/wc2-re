#ifndef WC2_SDL_SLINT_LAUNCHER_CONFIG_H
#define WC2_SDL_SLINT_LAUNCHER_CONFIG_H

#include "launcher_api.h"

#include <filesystem>

namespace wc2_launcher {

std::filesystem::path launcher_config_path();
void load_launcher_config(const std::filesystem::path &path,
                          SdlLauncherOptions &options);
bool save_launcher_config(const std::filesystem::path &path,
                          const SdlLauncherOptions &options);

} // namespace wc2_launcher

#endif
