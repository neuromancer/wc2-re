#include "sdl_port.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

int main(int argumentCount, char **arguments)
{
    static const char payload[] = "Wing Commander";
    static const char unicodePathMarker[] =
        "path-\xc5\x93-\xe9\x81\x8a\xe6\x88\xb2";
    char absoluteCasePath[8192];
    char absolutePath[8192];
    char currentDirectory[4096];
    const char *casePath;
    const char *path;
    char resolvedPath[PATH_MAX];
    char text[32];
    FILE *stream;
    int file;
    int failed;
    int pathResolved;

    failed = 0;
    if (GetCurrentDirectoryA(sizeof(currentDirectory),
                             currentDirectory) == 0)
        return 1;
    if (argumentCount > 1 &&
        strcmp(arguments[1], "--unicode-path") == 0 &&
        strstr(currentDirectory, unicodePathMarker) == 0)
        failed = 1;
    if (!SetCurrentDirectoryA(currentDirectory))
        return 1;
    if (strcmp(_itoa(-42, text, 10), "-42") != 0)
        failed = 1;
    if (strcmp(_ultoa(0x1a2b, text, 16), "1a2b") != 0)
        failed = 1;
    if (strcmp(_strupr(text), "1A2B") != 0)
        failed = 1;

    path = "sdl-crt-smoke.tmp";
    if ((size_t)snprintf(absolutePath, sizeof(absolutePath), "%s/%s",
                         currentDirectory, path) >= sizeof(absolutePath))
        return 1;
    file = _open(absolutePath, 0x8301, 0x0180);
    if (file == -1)
        return 1;
    if (_write(file, payload, sizeof(payload)) != sizeof(payload))
        failed = 1;
    if (_filelength(file) != sizeof(payload))
        failed = 1;
    if (_close(file) != 0)
        failed = 1;
    if (chmod(path, S_IRUSR) != 0)
        failed = 1;
    file = _open(path, 0x8002);
#ifdef _WIN32
    /* The Windows shim preserves the CRT's read-only access rules. */
    if (file != -1 || errno != EACCES)
        failed = 1;
    if (chmod(path, S_IRUSR | S_IWUSR) != 0)
        failed = 1;
#else
    if (file == -1)
        failed = 1;
#endif
    if (file != -1 && _close(file) != 0)
        failed = 1;
    file = _open(path, 0x8101, 0x0180);
    if (file == -1)
        failed = 1;
    else if (_close(file) != 0)
        failed = 1;
    if (_unlink(absolutePath) != 0)
        failed = 1;

    casePath = "SDL-Case-Smoke.TMP";
    file = _open(casePath, 0x8301, 0x0180);
    if (file == -1)
        return 1;
    if (_close(file) != 0)
        failed = 1;
    pathResolved = SdlResolvePath("sdl-case-smoke.tmp",
                                     resolvedPath, sizeof(resolvedPath));
    if (!pathResolved)
        failed = 1;
#ifndef _WIN32
    else if (strcmp(resolvedPath, "./SDL-Case-Smoke.TMP") != 0)
        failed = 1;
#endif
    if (pathResolved) {
        if ((size_t)snprintf(absoluteCasePath, sizeof(absoluteCasePath),
                             "%s/%s", currentDirectory,
                             resolvedPath) >= sizeof(absoluteCasePath))
            return 1;
        stream = fopen(absoluteCasePath, "rb");
        if (stream == 0)
            failed = 1;
        else if (fclose(stream) != 0)
            failed = 1;
    }
    file = _open("sdl-case-smoke.tmp", 0x8000);
    if (file == -1)
        failed = 1;
    else if (_close(file) != 0)
        failed = 1;
    if (_unlink(casePath) != 0)
        failed = 1;
    return failed;
}
