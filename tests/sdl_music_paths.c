#include "game.h"

#include <sys/stat.h>

int main(void)
{
    const char *fixture = "sdl-gamedat-music.tmp";
    const char *names[] = {"GAMEFLOW.STR", "GAMETWO.STR", "SPACEFLT.STR"};
    const char *requests[] = {"gameflow.str", "gametwo.str", "spaceflt.str"};
    const char *longComponent =
        "long-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    const char *gameData;
    const char *directory;
    unsigned int streamData[32] = {0x4d525453, 1};
    char originalDirectory[4096];
    char currentDirectory[4096];
    char path[256];
    int scenario;
    int depth;
    int level;
    int location;
    int fileIndex;
    int file;
    int failed;

    if (GetCurrentDirectoryA(sizeof(originalDirectory), originalDirectory) == 0)
        return 1;
#ifdef _WIN32
    if (_mkdir(fixture) != 0)
#else
    if (mkdir(fixture, 0700) != 0)
#endif
        return 1;
    if (_chdir(fixture) != 0)
        return 1;

    /* Minimal STRM header and one empty audio chunk. Open and parse it using
     * the real streamer, without playing audio or loading any game data. */
    streamData[2] = (22050U << 16) | (16U << 8) | 2U;
    streamData[3] = 4096;
    streamData[5] = 104;
    streamData[6] = 1;
    streamData[11] = 4096;
    streamData[26] = sizeof(streamData);
    streamData[27] = sizeof(streamData);
    for (fileIndex = 0; fileIndex < 32; fileIndex++)
        streamData[fileIndex] = SDL_SwapLE32(streamData[fileIndex]);
    SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
    if (SDL_Init(0) != 0 || ix_streamer_init() != 0)
        return 1;
    g_nAudioEnabled_0049c244 = 1;
    failed = 0;

    for (scenario = 0; scenario < 4; scenario++) {
        gameData = scenario == 1 ? "GameDat" : "GAMEDAT";
        depth = scenario < 2 ? 0 : scenario - 1;
#ifndef _WIN32
        /* Also exceed the old 256-byte current-directory buffer on POSIX.
         * Windows fixtures stay within the host CRT's legacy path limit. */
        if (scenario == 3)
            depth = 3;
#else
        if (scenario == 3)
            depth = 1;
#endif
        for (level = 0; level < depth; level++) {
#ifdef _WIN32
            if (_mkdir(longComponent) != 0)
#else
            if (mkdir(longComponent, 0700) != 0)
#endif
                return 1;
            if (_chdir(longComponent) != 0)
                return 1;
        }
        for (location = 0; location < 2; location++) {
            directory = location == 0 ? gameData : "STREAMS";
#ifdef _WIN32
            if (_mkdir(directory) != 0)
#else
            if (mkdir(directory, 0700) != 0)
#endif
                return 1;
        }
        for (fileIndex = 0; fileIndex < 3; fileIndex++) {
            snprintf(path, sizeof(path), "STREAMS/%s", names[fileIndex]);
            file = _open(path, 0x8301, 0x0180);
            if (file == -1 ||
                _write(file, streamData, sizeof(streamData)) != sizeof(streamData))
                return 1;
            _close(file);
        }

        for (location = 0; location < 2; location++) {
            if (location == 1 && _chdir(gameData) != 0)
                return 1;
            if (GetCurrentDirectoryA(sizeof(currentDirectory), currentDirectory) == 0)
                return 1;
            for (fileIndex = 0; fileIndex < 3; fileIndex++) {
                Streamer_open(requests[fileIndex]);
                /* Both FILE_OPEN and HAS_AUDIO must be set. */
                if ((g_dwStreamerState_005c4c38 & 0x82) != 0x82) {
                    fprintf(stderr, "music path case %d/%d: %s was not opened\n",
                            scenario, location, requests[fileIndex]);
                    failed = 1;
                }
                if ((g_dwStreamerState_005c4c38 & 2) != 0)
                    ix_streamer_close_stream_file();
                if (strcmp(SdlDescribeWorkingDirectory(), currentDirectory) != 0) {
                    fprintf(stderr, "music lookup changed the working directory\n");
                    return 1;
                }
            }
        }
        if (scenario == 3) {
            /* A real open failure should report the complete attempted path. */
            Streamer_open("missing.str");
            if ((g_dwStreamerState_005c4c38 & 2) != 0) {
                fprintf(stderr, "a missing music file was reported as open\n");
                failed = 1;
            }
        }
        if (_chdir("..") != 0)
            return 1;
        for (fileIndex = 0; fileIndex < 3; fileIndex++) {
            snprintf(path, sizeof(path), "STREAMS/%s", names[fileIndex]);
            if (_unlink(path) != 0)
                return 1;
        }
#ifdef _WIN32
        if (_rmdir(gameData) != 0 || _rmdir("STREAMS") != 0)
#else
        if (rmdir(gameData) != 0 || rmdir("STREAMS") != 0)
#endif
            return 1;
        for (level = 0; level < depth; level++) {
            if (_chdir("..") != 0)
                return 1;
#ifdef _WIN32
            if (_rmdir(longComponent) != 0)
#else
            if (rmdir(longComponent) != 0)
#endif
                return 1;
        }
    }
    /* STREAMS no longer exists. Do not pass a null directory to sprintf. */
    Streamer_open("gameflow.str");
    if ((g_dwStreamerState_005c4c38 & 2) != 0) {
        fprintf(stderr, "a missing STREAMS directory was reported as open\n");
        failed = 1;
    }
    ix_streamer_destroy();
    SDL_Quit();
    if (!SetCurrentDirectoryA(originalDirectory))
        return 1;
#ifdef _WIN32
    if (_rmdir(fixture) != 0)
#else
    if (rmdir(fixture) != 0)
#endif
        return 1;
    return failed;
}
