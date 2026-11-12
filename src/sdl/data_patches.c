/* SDL-only corrections to decoded Origin packets. Adapted from wc1-re.
 * No disk writes.
 * Both GetPacketSize and PacketLoad go through OpenPacketSection, so callers
 * allocating their own buffers see exactly the same size as the reader.
 *
 * Patches identify a filename, packet section, length and CRC32.
 * Unknown or already corrected assets pass through unchanged. Edits
 * are relative to the original decoded section.
 * See data_patches.h and bin/buildDataPatches.py for the documented fixes,
 * mission_patches.h for mission data the original games cannot load, and
 * cutscene_patches.h for cutscene shots whose motion is counted in drawn
 * frames (bin/buildCutscenePatches.py). */
#include "game.h"

typedef struct SdlDataEdit {
    unsigned int offset;
    unsigned int removeSize;
    unsigned int insertSize;
    unsigned int literalOffset;
} SdlDataEdit;

typedef struct SdlDataPatch {
    const char *filename;
    unsigned int section;
    unsigned int sourceSize;
    Uint32 sourceCrc;
    unsigned int resultSize;
    Uint32 resultCrc;
    const SdlDataEdit *edits;
    size_t editCount;
    const unsigned char *literals;
    size_t literalSize;
} SdlDataPatch;

#include "data_patches.h"
#include "mission_patches.h"
#include "cutscene_patches.h"

/* All packet patch tables, searched in this order. */
static const SdlDataPatch *SdlPacketPatch(size_t index)
{
    if (index < SDL_arraysize(packetPatches))
        return &packetPatches[index];
    index -= SDL_arraysize(packetPatches);
    if (index < SDL_arraysize(missionPacketPatches))
        return &missionPacketPatches[index];
    return &cutscenePacketPatches[index - SDL_arraysize(missionPacketPatches)];
}

static const char *SdlDataBasename(const char *filename)
{
    const char *basename = filename;
    const char *cursor;

    for (cursor = filename; *cursor != '\0'; cursor++) {
        if (*cursor == '/' || *cursor == '\\')
            basename = cursor + 1;
    }
    return basename;
}

static int SdlApplyDataEdits(const SdlDataPatch *patch,
                             const unsigned char *source,
                             unsigned char *result)
{
    size_t sourcePosition = 0;
    size_t resultPosition = 0;
    size_t index;

    for (index = 0; index < patch->editCount; index++) {
        const SdlDataEdit *edit = &patch->edits[index];
        size_t copySize;

        if (edit->offset < sourcePosition ||
            edit->offset > patch->sourceSize ||
            edit->removeSize > patch->sourceSize - edit->offset ||
            edit->literalOffset > patch->literalSize ||
            edit->insertSize > patch->literalSize - edit->literalOffset)
            return 0;
        copySize = edit->offset - sourcePosition;
        if (copySize > patch->resultSize - resultPosition)
            return 0;
        memcpy(result + resultPosition, source + sourcePosition, copySize);
        resultPosition += copySize;
        if (edit->insertSize > patch->resultSize - resultPosition)
            return 0;
        memcpy(result + resultPosition,
               patch->literals + edit->literalOffset, edit->insertSize);
        resultPosition += edit->insertSize;
        sourcePosition = edit->offset + edit->removeSize;
    }
    if (patch->sourceSize - sourcePosition !=
        patch->resultSize - resultPosition)
        return 0;
    memcpy(result + resultPosition, source + sourcePosition,
           patch->sourceSize - sourcePosition);
    return SDL_crc32(0, result, patch->resultSize) == patch->resultCrc;
}

int SdlPatchPacketSection(const char *filename, short section,
                         PacketSectionHandle *handle)
{
    const SdlDataPatch *patch;
    const char *basename = SdlDataBasename(SdlCampaignPath(filename));
    unsigned char *stored = 0;
    unsigned char *decoded = 0;
    unsigned char *result = 0;
    size_t decodedSize;
    size_t readSize;
    size_t writtenSize;
    size_t index;
    Uint32 crc;
    int count;
    int success = 0;
    short error = g_nPacketError;
    const size_t patchCount = SDL_arraysize(packetPatches) +
        SDL_arraysize(missionPacketPatches) + SDL_arraysize(cutscenePacketPatches);

    for (index = 0; index < patchCount; index++) {
        patch = SdlPacketPatch(index);
        if (section == patch->section &&
            SDL_strcasecmp(basename, patch->filename) == 0 &&
            (handle->compression == 1 || handle->compression == 0x20 ||
             handle->dataSize == patch->sourceSize))
            break;
    }
    if (index == patchCount)
        return 1;

    /* Bound temporary storage before reading an archive supplied by the
     * player. A short read must never hash uninitialized memory. */
    g_nPacketError = 6;
    if (handle->dataSize == 0 || handle->dataSize > 16U * 1024U * 1024U)
        return 0;
    stored = malloc(handle->dataSize);
    if (stored == 0) {
        g_nPacketError = 1;
        return 0;
    }
    if (_lseek(handle->file, handle->dataOffset, SEEK_SET) == -1)
        goto done;
    readSize = 0;
    while (readSize < handle->dataSize) {
        count = _read(handle->file, stored + readSize,
                      handle->dataSize - readSize);
        if (count <= 0)
            goto done;
        readSize += count;
    }
    decodedSize = handle->dataSize;
    if (handle->compression == 1 || handle->compression == 0x20) {
        if (readSize < 4)
            goto done;
        decodedSize = (Uint32)stored[0] | ((Uint32)stored[1] << 8) |
            ((Uint32)stored[2] << 16) | ((Uint32)stored[3] << 24);
    }
    for (index = 0; index < patchCount; index++) {
        patch = SdlPacketPatch(index);
        if (section == patch->section && decodedSize == patch->sourceSize &&
            SDL_strcasecmp(basename, patch->filename) == 0)
            break;
    }
    if (index == patchCount) {
        success = 1;
        goto done;
    }
    if (handle->compression == 1 || handle->compression == 0x20) {
        decoded = malloc(decodedSize);
        if (decoded == 0) {
            g_nPacketError = 1;
            goto done;
        }
        if (!SdlDecompressOriginLzw(stored + 4, readSize - 4, decoded,
                                    decodedSize, &writtenSize))
            goto done;
    } else {
        decoded = stored;
        stored = 0;
    }
    crc = SDL_crc32(0, decoded, decodedSize);
    for (index = 0; index < patchCount; index++) {
        patch = SdlPacketPatch(index);
        if (section == patch->section && decodedSize == patch->sourceSize &&
            crc == patch->sourceCrc &&
            SDL_strcasecmp(basename, patch->filename) == 0)
            break;
    }
    if (index == patchCount) {
        success = 1;
        goto done;
    }
    result = malloc(patch->resultSize);
    if (result == 0) {
        g_nPacketError = 1;
        goto done;
    }
    if (!SdlApplyDataEdits(patch, decoded, result))
        goto done;
    success = 1;

done:
    free(stored);
    free(decoded);
    if (_lseek(handle->file, handle->dataOffset, SEEK_SET) == -1) {
        g_nPacketError = 6;
        success = 0;
    }
    if (!success) {
        /* Never silently use a larger original after a size query succeeded
         * with a smaller patch: the caller may already own that small buffer. */
        free(result);
        return 0;
    }
    if (result != 0) {
        handle->patchedData = result;
        handle->dataSize = patch->resultSize;
        handle->compression = 0;
    }
    g_nPacketError = error;
    return 1;
}
