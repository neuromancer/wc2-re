cmake_minimum_required(VERSION 3.21)

project(wc2_slint_gui VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# The launcher is compiled into one load-on-demand module. Keep Slint itself
# static and enable only the window backend and software renderer it needs.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SLINT_FEATURE_ACCESSIBILITY OFF CACHE BOOL "" FORCE)
set(SLINT_FEATURE_INTERPRETER OFF CACHE BOOL "" FORCE)
set(SLINT_FEATURE_RENDERER_FEMTOVG OFF CACHE BOOL "" FORCE)
set(SLINT_FEATURE_RENDERER_SOFTWARE ON CACHE BOOL "" FORCE)
set(SLINT_FEATURE_TESTING OFF CACHE BOOL "" FORCE)
set(NFD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(NFD_BUILD_SDL2_TESTS OFF CACHE BOOL "" FORCE)
set(NFD_INSTALL OFF CACHE BOOL "" FORCE)
if(UNIX AND NOT APPLE)
    # The portal backend avoids pulling GTK into the optional launcher.
    set(NFD_PORTAL ON CACHE BOOL "" FORCE)
endif()

include(FetchContent)
FetchContent_Declare(
    Slint
    GIT_REPOSITORY https://github.com/slint-ui/slint.git
    GIT_TAG v1.16.1
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR api/cpp
)
FetchContent_MakeAvailable(Slint)
FetchContent_Declare(
    NativeFileDialogExtended
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
    GIT_TAG v1.3.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(NativeFileDialogExtended)

add_library(wc2-slint-gui SHARED
    launcher.cpp
    wc2_logo.h
)
set_target_properties(wc2-slint-gui PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
target_compile_definitions(wc2-slint-gui PRIVATE WC2_SLINT_GUI_BUILD=1)
target_link_libraries(wc2-slint-gui PRIVATE Slint::Slint nfd::nfd)
slint_target_sources(wc2-slint-gui launcher.slint)

if(WIN32)
    target_link_libraries(wc2-slint-gui PRIVATE
        imm32
        ole32
        shell32
        uuid
        uxtheme
    )
endif()

# Corrosion cannot propagate framework metadata out of a Rust static library.
# These are the native frameworks used by Slint's macOS winit backend.
if(APPLE)
    find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
    find_library(CARBON_FRAMEWORK Carbon REQUIRED)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
    find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
    find_library(CORESERVICES_FRAMEWORK CoreServices REQUIRED)
    find_library(CORETEXT_FRAMEWORK CoreText REQUIRED)
    find_library(COREVIDEO_FRAMEWORK CoreVideo REQUIRED)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
    target_link_libraries(wc2-slint-gui PRIVATE
        ${APPKIT_FRAMEWORK}
        ${CARBON_FRAMEWORK}
        ${COREFOUNDATION_FRAMEWORK}
        ${COREGRAPHICS_FRAMEWORK}
        ${CORESERVICES_FRAMEWORK}
        ${CORETEXT_FRAMEWORK}
        ${COREVIDEO_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
        ${QUARTZCORE_FRAMEWORK}
    )
    target_link_options(wc2-slint-gui PRIVATE
        "LINKER:-exported_symbol,_SdlRunLauncherGui"
        "$<$<CONFIG:Release>:LINKER:-dead_strip>"
    )
elseif(UNIX)
    target_link_options(wc2-slint-gui PRIVATE
        "LINKER:--exclude-libs,ALL"
        "$<$<CONFIG:Release>:LINKER:--gc-sections>"
    )
endif()

if(MSVC)
    target_compile_options(wc2-slint-gui PRIVATE /W4)
else()
    target_compile_options(wc2-slint-gui PRIVATE -Wall -Wextra)
endif()

if(NOT WC2_GUI_OUTPUT_DIRECTORY)
    set(WC2_GUI_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
get_filename_component(
    WC2_GUI_OUTPUT_DIRECTORY
    "${WC2_GUI_OUTPUT_DIRECTORY}"
    ABSOLUTE
)
set_target_properties(wc2-slint-gui PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${WC2_GUI_OUTPUT_DIRECTORY}"
    LIBRARY_OUTPUT_DIRECTORY "${WC2_GUI_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY "${WC2_GUI_OUTPUT_DIRECTORY}"
)
foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set_target_properties(wc2-slint-gui PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY_${CONFIG} "${WC2_GUI_OUTPUT_DIRECTORY}"
        LIBRARY_OUTPUT_DIRECTORY_${CONFIG} "${WC2_GUI_OUTPUT_DIRECTORY}"
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG} "${WC2_GUI_OUTPUT_DIRECTORY}"
    )
endforeach()
if(WIN32)
    set_target_properties(wc2-slint-gui PROPERTIES PREFIX "")
endif()
