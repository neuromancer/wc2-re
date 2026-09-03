#include "sdl_port.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#undef fopen
#endif

#ifndef _WIN32

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <strings.h>
#include <sys/stat.h>

static int SdlAppendPathComponent(char *path, size_t pathSize,
                                     const char *component)
{
    size_t componentLength;
    size_t pathLength;
    int needsSeparator;

    pathLength = strlen(path);
    componentLength = strlen(component);
    needsSeparator = pathLength != 1 || path[0] != '/';
    if (pathLength + (size_t)needsSeparator + componentLength + 1 > pathSize)
        return 0;
    if (needsSeparator)
        path[pathLength++] = '/';
    memcpy(path + pathLength, component, componentLength + 1);
    return 1;
}

static int SdlAppendResolvedComponent(char *path, size_t pathSize,
                                         const char *component)
{
    DIR *directory;
    struct dirent *entry;
    int result;

    if (strcmp(component, ".") == 0 || strcmp(component, "..") == 0)
        return SdlAppendPathComponent(path, pathSize, component);
    directory = opendir(path);
    if (directory == 0)
        return SdlAppendPathComponent(path, pathSize, component);
    while ((entry = readdir(directory)) != 0) {
        if (strcasecmp(entry->d_name, component) == 0) {
            result = SdlAppendPathComponent(path, pathSize,
                                               entry->d_name);
            closedir(directory);
            return result;
        }
    }
    closedir(directory);
    return SdlAppendPathComponent(path, pathSize, component);
}

int SdlResolvePath(const char *path, char *resolved,
                      unsigned long resolvedSize)
{
    char component[PATH_MAX];
    char normalized[PATH_MAX];
    char prefix[PATH_MAX];
    const char *cursor;
    char *separator;
    size_t pathLength;

    if (path == 0 || resolved == 0 || resolvedSize == 0)
        return 0;
    pathLength = strlen(path);
    if (pathLength >= sizeof(normalized))
        return 0;
    memcpy(normalized, path, pathLength + 1);
    separator = normalized;
    while (*separator != '\0') {
        if (*separator == '\\')
            *separator = '/';
        separator++;
    }
    if (normalized[0] == '/')
        strcpy(prefix, "/");
    else
        strcpy(prefix, ".");
    cursor = normalized;
    while (*cursor == '/')
        cursor++;
    while (*cursor != '\0') {
        const char *end;
        size_t componentLength;

        end = cursor;
        while (*end != '\0' && *end != '/')
            end++;
        componentLength = (size_t)(end - cursor);
        if (componentLength >= sizeof(component))
            return 0;
        memcpy(component, cursor, componentLength);
        component[componentLength] = '\0';
        if (!SdlAppendResolvedComponent(prefix, sizeof(prefix), component))
            return 0;
        cursor = end;
        while (*cursor == '/')
            cursor++;
    }
    pathLength = strlen(prefix);
    if (pathLength + 1 > resolvedSize)
        return 0;
    memcpy(resolved, prefix, pathLength + 1);
    return 1;
}

int SdlChangeDirectory(const char *path)
{
    char resolved[PATH_MAX];

    if (!SdlResolvePath(path, resolved, sizeof(resolved)))
        return -1;
    return chdir(resolved);
}

/*
 *  The game removes a file by the name it writes, but a case-sensitive host
 *  can be holding it under the case the install shipped.  Resolve first, the
 *  way SdlOpen does, or the removal quietly misses and the next create
 *  runs into whatever was left behind.
 */
int SdlUnlink(const char *path)
{
    char hostPath[PATH_MAX];

    if (!SdlResolvePath(path, hostPath, sizeof(hostPath)))
        return -1;
    return unlink(hostPath);
}

int SdlOpen(const char *path, int flags, ...)
{
    int hostFlags;
    int hostMode;
    int file;
    int openError;
    char hostPath[PATH_MAX];
    struct stat status;

    hostFlags = flags & 3;
    if ((flags & 0x0008) != 0)
        hostFlags |= O_APPEND;
    if ((flags & 0x0100) != 0)
        hostFlags |= O_CREAT;
    if ((flags & 0x0200) != 0)
        hostFlags |= O_TRUNC;
    if ((flags & 0x0400) != 0)
        hostFlags |= O_EXCL;

    hostMode = S_IRUSR | S_IWUSR;
    if ((flags & 0x0100) != 0) {
        va_list arguments;
        int dosMode;

        va_start(arguments, flags);
        dosMode = va_arg(arguments, int);
        va_end(arguments);
        hostMode = 0;
        if ((dosMode & 0x0100) != 0)
            hostMode |= S_IRUSR;
        if ((dosMode & 0x0080) != 0)
            hostMode |= S_IWUSR;
    }

    if (!SdlResolvePath(path, hostPath, sizeof(hostPath)))
        return -1;
    file = open(hostPath, hostFlags, hostMode);
    /* Mutable files copied from the original media may still be read-only. */
    if (file == -1 && errno == EACCES &&
        (hostFlags & O_ACCMODE) != O_RDONLY &&
        ((hostFlags & O_ACCMODE) == O_WRONLY ||
         (hostFlags & (O_CREAT | O_TRUNC)) != 0)) {
        openError = errno;
        if (stat(hostPath, &status) == 0 && S_ISREG(status.st_mode) &&
            chmod(hostPath, status.st_mode | S_IWUSR) == 0)
            file = open(hostPath, hostFlags, hostMode);
        else
            errno = openError;
    }
    if (file == -1 && (hostFlags & O_ACCMODE) == O_RDWR &&
        (hostFlags & O_CREAT) == 0) {
        hostFlags = (hostFlags & ~O_ACCMODE) | O_RDONLY;
        file = open(hostPath, hostFlags, hostMode);
    }
    return file;
}

long SdlFileLength(int file)
{
    struct stat status;

    if (fstat(file, &status) != 0)
        return -1;
    return (long)status.st_size;
}

static char *SdlLowercaseDigits(char *text)
{
    char *cursor;

    cursor = text;
    while (*cursor != '\0') {
        *cursor = (char)tolower((unsigned char)*cursor);
        cursor++;
    }
    return text;
}

char *SdlItoa(int value, char *text, int radix)
{
    return SdlLtoa((long)value, text, radix);
}

char *SdlLtoa(long value, char *text, int radix)
{
    if (radix == 10)
        return SDL_ltoa(value, text, radix);
    else if (radix == 16)
        return SdlLowercaseDigits(
            SDL_ultoa((unsigned long)value, text, radix));
    text[0] = '\0';
    return text;
}

char *SdlUltoa(unsigned long value, char *text, int radix)
{
    if (radix == 10)
        return SDL_ultoa(value, text, radix);
    else if (radix == 16)
        return SdlLowercaseDigits(SDL_ultoa(value, text, radix));
    text[0] = '\0';
    return text;
}

/*
 *  MSVC's _findfirst/_findnext directory walk.  WC2 uses it to enumerate the
 *  pilot and saved-game files, so the shim only has to honour the shell-style
 *  patterns the game actually passes ("*.PLT", "SAVE*.*", ...) and fill in the
 *  name and size fields; the timestamps stay zero because nothing reads them.
 */
typedef struct SdlFindHandle {
    DIR *directory;
    char base[PATH_MAX];
    char pattern[NAME_MAX + 1];
} SdlFindHandle;

static int SdlMatchPattern(const char *pattern, const char *name)
{
    while (*pattern != '\0') {
        if (*pattern == '*') {
            pattern++;
            if (*pattern == '\0')
                return 1;
            while (*name != '\0') {
                if (SdlMatchPattern(pattern, name))
                    return 1;
                name++;
            }
            return SdlMatchPattern(pattern, name);
        }
        if (*name == '\0')
            return 0;
        if (*pattern != '?' &&
            toupper((unsigned char)*pattern) != toupper((unsigned char)*name))
            return 0;
        pattern++;
        name++;
    }
    return *name == '\0';
}

static int SdlFillFound(SdlFindHandle *handle,
                           struct _finddata_t *found)
{
    struct dirent *entry;
    char full[PATH_MAX];
    struct stat status;

    while ((entry = readdir(handle->directory)) != 0) {
        if (!SdlMatchPattern(handle->pattern, entry->d_name))
            continue;
        memset(found, 0, sizeof(*found));
        strncpy(found->name, entry->d_name, sizeof(found->name) - 1);
        if ((size_t)snprintf(full, sizeof(full), "%s/%s",
                             handle->base, entry->d_name) < sizeof(full) &&
            stat(full, &status) == 0)
            found->size = (long)status.st_size;
        return 1;
    }
    return 0;
}

/* The game truncates the handle to a short before handing it back -- see
 * OpenDiskDataFile -- so it cannot be a pointer.  Hand out small slot indices
 * instead; the game never has more than one walk open at a time. */
#define SDL_FIND_SLOTS 8
static SdlFindHandle *g_apFindSlots[SDL_FIND_SLOTS];

long SdlFindFirst(const char *pattern, struct _finddata_t *found)
{
    SdlFindHandle *handle;
    char resolved[PATH_MAX];
    const char *leaf;
    size_t baseLength;
    int slot;

    if (pattern == 0 || found == 0)
        return -1;
    for (slot = 0; slot < SDL_FIND_SLOTS; slot++) {
        if (g_apFindSlots[slot] == 0)
            break;
    }
    if (slot == SDL_FIND_SLOTS)
        return -1;
    if (!SdlResolvePath(pattern, resolved, (unsigned long)sizeof(resolved)))
        return -1;
    handle = (SdlFindHandle *)calloc(1, sizeof(*handle));
    if (handle == 0)
        return -1;
    leaf = strrchr(resolved, '/');
    if (leaf == 0) {
        strcpy(handle->base, ".");
        leaf = resolved;
    } else {
        baseLength = (size_t)(leaf - resolved);
        if (baseLength == 0)
            baseLength = 1;
        if (baseLength >= sizeof(handle->base)) {
            free(handle);
            return -1;
        }
        memcpy(handle->base, resolved, baseLength);
        handle->base[baseLength] = '\0';
        leaf++;
    }
    strncpy(handle->pattern, leaf, sizeof(handle->pattern) - 1);
    handle->directory = opendir(handle->base);
    if (handle->directory == 0) {
        free(handle);
        return -1;
    }
    if (!SdlFillFound(handle, found)) {
        closedir(handle->directory);
        free(handle);
        return -1;
    }
    g_apFindSlots[slot] = handle;
    return slot + 1;
}

/* MSVC returns -1 from _findfirst when nothing matched, and callers hand that
 * straight back to _findnext and _findclose without checking; both are
 * expected to shrug it off. */
static SdlFindHandle *SdlFindWalk(long handle)
{
    if (handle < 1 || handle > SDL_FIND_SLOTS)
        return 0;
    return g_apFindSlots[handle - 1];
}

int SdlFindNext(long handle, struct _finddata_t *found)
{
    SdlFindHandle *walk;

    walk = SdlFindWalk(handle);
    if (walk == 0 || found == 0)
        return -1;
    return SdlFillFound(walk, found) ? 0 : -1;
}

int SdlFindClose(long handle)
{
    SdlFindHandle *walk;

    walk = SdlFindWalk(handle);
    if (walk == 0)
        return -1;
    g_apFindSlots[handle - 1] = 0;
    closedir(walk->directory);
    free(walk);
    return 0;
}


char *SdlStrupr(char *text)
{
    char *cursor;

    cursor = text;
    while (*cursor != '\0') {
        *cursor = (char)toupper((unsigned char)*cursor);
        cursor++;
    }
    return text;
}

#else

/* SDL, Slint and NFD exchange paths as UTF-8.  Keep the Windows CRT's
 * locale-dependent narrow path functions out of that boundary. */
static wchar_t *SdlUtf8ToWide(const char *text)
{
    if (text == 0)
        return 0;
    return (wchar_t *)SDL_iconv_string(
        "WCHAR_T", "UTF-8", text, strlen(text) + 1);
}

int SdlChangeDirectory(const char *path)
{
    wchar_t *widePath;
    int result;

    widePath = SdlUtf8ToWide(path);
    if (widePath == 0)
        return -1;
    result = _wchdir(widePath);
    SDL_free(widePath);
    return result;
}

int SdlUnlink(const char *path)
{
    wchar_t *widePath;
    int result;

    widePath = SdlUtf8ToWide(path);
    if (widePath == 0)
        return -1;
    result = _wunlink(widePath);
    SDL_free(widePath);
    return result;
}

int SdlOpen(const char *path, int flags, ...)
{
    wchar_t *widePath;
    int file;

    widePath = SdlUtf8ToWide(path);
    if (widePath == 0)
        return -1;
    if ((flags & 0x0100) != 0) {
        va_list arguments;
        int mode;

        va_start(arguments, flags);
        mode = va_arg(arguments, int);
        va_end(arguments);
        file = _wopen(widePath, flags, mode);
    } else {
        file = _wopen(widePath, flags);
    }
    SDL_free(widePath);
    return file;
}

FILE *SdlFopen(const char *path, const char *mode)
{
    FILE *file;
    wchar_t *wideMode;
    wchar_t *widePath;

    widePath = SdlUtf8ToWide(path);
    wideMode = SdlUtf8ToWide(mode);
    if (widePath == 0 || wideMode == 0) {
        SDL_free(wideMode);
        SDL_free(widePath);
        return 0;
    }
    file = _wfopen(widePath, wideMode);
    SDL_free(wideMode);
    SDL_free(widePath);
    return file;
}

int SdlResolvePath(const char *path, char *resolved,
                      unsigned long resolvedSize)
{
    size_t pathLength;

    if (path == 0 || resolved == 0 || resolvedSize == 0)
        return 0;
    pathLength = strlen(path);
    if (pathLength + 1 > resolvedSize)
        return 0;
    memcpy(resolved, path, pathLength + 1);
    return 1;
}

#endif

/*
 *  Shared by every host.  Rewriting the formats and gating the trace are not
 *  POSIX-specific concerns -- see the note in sdl_port.h.
 */
/*
 *  Rewrite a printf format the way MSVC would have read it: drop the far/near
 *  pointer size modifiers, which are meaningless in a flat model but which
 *  clang treats as conversions in their own right.  Anything the state machine
 *  does not recognise is copied through untouched.
 */
static const char *SdlPortableFormat(const char *format, char *scratch,
                                        size_t scratchSize)
{
    const char *read;
    char *write;
    char *limit;
    int inConversion;

    if (strchr(format, 'F') == 0 && strchr(format, 'N') == 0)
        return format;
    read = format;
    write = scratch;
    limit = scratch + scratchSize - 1;
    inConversion = 0;
    while (*read != '\0' && write < limit) {
        if (!inConversion) {
            inConversion = *read == '%';
            *write++ = *read++;
            continue;
        }
        if (*read == '%') {
            inConversion = 0;
            *write++ = *read++;
            continue;
        }
        if (*read == 'F' || *read == 'N') {
            read++;
            continue;
        }
        if (strchr("diouxXeEfgGaAcspn", *read) != 0)
            inConversion = 0;
        *write++ = *read++;
    }
    *write = '\0';
    return scratch;
}

int SdlVsnprintf(char *buffer, size_t size, const char *format,
                    va_list arguments)
{
    char scratch[1024];

    return vsnprintf(buffer, size,
                     SdlPortableFormat(format, scratch, sizeof(scratch)),
                     arguments);
}

int SdlSnprintf(char *buffer, size_t size, const char *format, ...)
{
    va_list arguments;
    int written;

    va_start(arguments, format);
    written = SdlVsnprintf(buffer, size, format, arguments);
    va_end(arguments);
    return written;
}

int SdlTraceEnabled(void)
{
    static int state = -1;

    if (state < 0)
        state = getenv("INPUT_TRACE") != 0;
    return state;
}

void SdlTracef(const char *format, ...)
{
    va_list arguments;

    if (!SdlTraceEnabled())
        return;
    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    va_end(arguments);
    fflush(stderr);
}

int SdlPrintf(const char *format, ...)
{
    char scratch[1024];
    va_list arguments;
    int written;

    va_start(arguments, format);
    written = vprintf(SdlPortableFormat(format, scratch, sizeof(scratch)),
                      arguments);
    va_end(arguments);
    return written;
}

int SdlFprintf(FILE *stream, const char *format, ...)
{
    char scratch[1024];
    va_list arguments;
    int written;

    va_start(arguments, format);
    written = vfprintf(stream,
                       SdlPortableFormat(format, scratch, sizeof(scratch)),
                       arguments);
    va_end(arguments);
    return written;
}

/*
 *  There is no console to read on any host, so the acknowledgement wait
 *  becomes a pump of the SDL event queue.
 */
int SdlGetChar(void)
{
    SdlPumpEvents();
    return 0;
}

int SdlFlushAll(void)
{
    fflush(0);
    return 0;
}

/*
 *  The game changes into its data directory on the way in, so a file it
 *  cannot open says more with the directory it was looking in.
 */
#define SDL_WORKING_DIRECTORY_SIZE 4096

const char *SdlDescribeWorkingDirectory(void)
{
    static char directory[SDL_WORKING_DIRECTORY_SIZE];

    if (GetCurrentDirectoryA((DWORD)sizeof(directory), directory) == 0)
        strcpy(directory, "?");
    return directory;
}
