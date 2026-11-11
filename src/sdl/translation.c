/* SDL-only localization. Editable UTF-8 PO files are compiled to WC2 bytes by
 * bin/compileTranslations.py. Resources are translated in memory: original
 * scene commands, conditions, timing and animation remain byte-for-byte intact. */
#include "game.h"

#define TRANSLATION_LIMIT (16U * 1024U * 1024U)

typedef struct TranslationMessage {
    const char *context;
    const char *id;
    const char *text;
} TranslationMessage;

typedef struct TranslationFont {
    const unsigned char *glyphs;
    Uint32 crc;
    unsigned short count;
    unsigned char height;
    unsigned char ink;
    unsigned char background;
} TranslationFont;

typedef struct TranslationArtwork {
    const char *filename;
    const unsigned char *edits;
    Uint32 sourceSize, sourceCrc, targetSize, targetCrc, count;
    unsigned short section;
} TranslationArtwork;

static unsigned char *catalog;
static TranslationMessage *messages;
static size_t messageCount;
static TranslationFont fonts[3];
static TranslationArtwork *artwork;
static size_t artworkCount;

static unsigned int ReadTranslation16(const unsigned char *p)
{
    return p[0] | ((unsigned int)p[1] << 8);
}

static Uint32 ReadTranslation32(const unsigned char *p)
{
    return ReadTranslation16(p) | ((Uint32)ReadTranslation16(p + 2) << 16);
}

static const char *TranslationBasename(const char *filename)
{
    const char *name = filename;
    for (; *filename; filename++) {
        if (*filename == '/' || *filename == '\\')
            name = filename + 1;
    }
    return name;
}

void SdlShutdownTranslations(void)
{
    free(messages);
    free(artwork);
    SDL_free(catalog);
    messages = 0;
    catalog = 0;
    messageCount = 0;
    artwork = 0;
    artworkCount = 0;
    memset(fonts, 0, sizeof(fonts));
}

int SdlLoadTranslationCatalog(const char *path)
{
    SDL_RWops *file;
    Sint64 size;
    size_t position;
    size_t index;
    size_t fontCount;

    SdlShutdownTranslations();
    file = SDL_RWFromFile(path, "rb");
    if (!file)
        return 0;
    size = SDL_RWsize(file);
    if (size < 12 || size > TRANSLATION_LIMIT) {
        SDL_RWclose(file);
        SDL_SetError("Invalid translation catalog size");
        return 0;
    }
    catalog = SDL_malloc((size_t)size);
    if (!catalog) {
        SDL_RWclose(file);
        return 0;
    }
    if (SDL_RWread(file, catalog, 1, (size_t)size) != (size_t)size) {
        SDL_RWclose(file);
        goto invalid;
    }
    SDL_RWclose(file);
    if (memcmp(catalog, "WCT2", 4) != 0)
        goto invalid;
    messageCount = ReadTranslation32(catalog + 4);
    fontCount = ReadTranslation32(catalog + 8);
    if (!messageCount || messageCount > (size_t)size / 12 || fontCount > 3)
        goto invalid;
    messages = calloc(messageCount, sizeof(*messages));
    if (!messages)
        goto invalid;
    position = 12;
    for (index = 0; index < messageCount; index++) {
        unsigned int lengths[3];
        const char *fields[3];
        size_t field;
        if ((size_t)size - position < 6)
            goto invalid;
        for (field = 0; field < 3; field++)
            lengths[field] = ReadTranslation16(catalog + position + field * 2);
        position += 6;
        for (field = 0; field < 3; field++) {
            size_t length = lengths[field];
            if (length < (field == 0 ? 1U : 2U) || length > (size_t)size - position ||
                memchr(catalog + position, 0, length) != catalog + position + length - 1)
                goto invalid;
            fields[field] = (const char *)catalog + position;
            position += length;
        }
        if (lengths[1] != 21 || strncmp(fields[1], "wc2:", 4) != 0 ||
            strspn(fields[1] + 4, "0123456789abcdef") != 16)
            goto invalid;
        messages[index].context = fields[0];
        messages[index].id = fields[1];
        messages[index].text = fields[2];
        if (index) {
            int order = strcmp(messages[index - 1].context, fields[0]);
            if (order > 0 || (order == 0 && strcmp(messages[index - 1].id, fields[1]) >= 0))
                goto invalid;
        }
    }
    for (index = 0; index < fontCount; index++) {
        TranslationFont *font;
        size_t glyph;
        unsigned char seen[256] = {0};
        if ((size_t)size - position < 10 || catalog[position] >= 3)
            goto invalid;
        font = &fonts[catalog[position]];
        if (font->glyphs)
            goto invalid;
        font->height = catalog[position + 1];
        font->ink = catalog[position + 2];
        font->background = catalog[position + 3];
        font->crc = ReadTranslation32(catalog + position + 4);
        font->count = ReadTranslation16(catalog + position + 8);
        position += 10;
        if (!font->height || font->height > 32 || font->count > 256)
            goto invalid;
        font->glyphs = catalog + position;
        for (glyph = 0; glyph < font->count; glyph++) {
            size_t pixels;
            if ((size_t)size - position < 2 || seen[catalog[position]]++)
                goto invalid;
            pixels = catalog[position + 1] * font->height;
            if (catalog[position + 1] > 32 || pixels > (size_t)size - position - 2)
                goto invalid;
            position += pixels + 2;
        }
    }
    /* Optional sparse image changes. Validate every splice before retaining
     * pointers into the file. */
    if (position != (size_t)size) {
        if ((size_t)size - position < 8 || memcmp(catalog + position, "WCA1", 4))
            goto invalid;
        artworkCount = ReadTranslation32(catalog + position + 4);
        position += 8;
        if (!artworkCount || artworkCount > 256)
            goto invalid;
        artwork = calloc(artworkCount, sizeof(*artwork));
        if (!artwork)
            goto invalid;
        for (index = 0; index < artworkCount; index++) {
            TranslationArtwork *patch = &artwork[index];
            size_t nameLength, edit, previous = 0;
            Sint64 resultSize;
            if ((size_t)size - position < 24)
                goto invalid;
            nameLength = ReadTranslation16(catalog + position);
            patch->section = ReadTranslation16(catalog + position + 2);
            patch->sourceSize = ReadTranslation32(catalog + position + 4);
            patch->sourceCrc = ReadTranslation32(catalog + position + 8);
            patch->targetSize = ReadTranslation32(catalog + position + 12);
            patch->targetCrc = ReadTranslation32(catalog + position + 16);
            patch->count = ReadTranslation32(catalog + position + 20);
            position += 24;
            if (nameLength < 2 || nameLength > 64 || nameLength > (size_t)size - position ||
                memchr(catalog + position, 0, nameLength) != catalog + position + nameLength - 1 ||
                !patch->sourceSize || patch->sourceSize > TRANSLATION_LIMIT ||
                !patch->targetSize || patch->targetSize > TRANSLATION_LIMIT ||
                !patch->count || patch->count > 65536 || patch->section >= 32768)
                goto invalid;
            patch->filename = (const char *)catalog + position;
            position += nameLength;
            patch->edits = catalog + position;
            resultSize = patch->sourceSize;
            for (edit = 0; edit < patch->count; edit++) {
                size_t offset, removed, inserted;
                if ((size_t)size - position < 12)
                    goto invalid;
                offset = ReadTranslation32(catalog + position);
                removed = ReadTranslation32(catalog + position + 4);
                inserted = ReadTranslation32(catalog + position + 8);
                position += 12;
                if (offset < previous || offset > patch->sourceSize ||
                    removed > patch->sourceSize - offset || inserted > (size_t)size - position)
                    goto invalid;
                previous = offset + removed;
                resultSize += (Sint64)inserted - removed;
                position += inserted;
            }
            if (resultSize != patch->targetSize)
                goto invalid;
        }
    }
    if (position != (size_t)size)
        goto invalid;
    return 1;
invalid:
    SdlShutdownTranslations();
    SDL_SetError("Invalid WC2 translation catalog: %s", path);
    return 0;
}

int SdlSetLanguage(int language)
{
    char *base;
    char path[4096];
    int result;

    if (language == 0) {
        SdlShutdownTranslations();
        return 1;
    }
    if (language != 1)
        return 0;
    base = SDL_GetBasePath();
    if (!base)
        return 0;
    result = SDL_snprintf(path, sizeof(path), "%stranslations/es.dat", base);
    SDL_free(base);
    if (result < 0 || result >= (int)sizeof(path))
        return 0;
    return SdlLoadTranslationCatalog(path);
}

static const char *FindTranslation(const char *context, const char *source)
{
    size_t first = 0;
    size_t end = messageCount;
    Uint64 hash = 14695981039346656037ULL;
    const unsigned char *byte = (const unsigned char *)source;
    char id[21];
    if (!messageCount || !source)
        return source;
    /* FNV-1a over the original CP437 bytes, matching message_key(). */
    for (; *byte; byte++) {
        hash ^= *byte;
        hash *= 1099511628211ULL;
    }
    SDL_snprintf(id, sizeof(id), "wc2:%08x%08x",
                 (unsigned int)(hash >> 32), (unsigned int)hash);
    while (first < end) {
        size_t middle = first + (end - first) / 2;
        int order = strcmp(context, messages[middle].context);
        if (order == 0)
            order = strcmp(id, messages[middle].id);
        if (order < 0)
            end = middle;
        else if (order > 0)
            first = middle + 1;
        else
            return messages[middle].text;
    }
    return source;
}

char *SdlTranslateText(const char *context, const char *source)
{
    return (char *)FindTranslation(context, source);
}

short SdlCinematicFontCharacter(short character)
{
    if (catalog) {
        /* The credits use ARTDATA.V00/7's sprite alphabet, not FONTS.FNT. */
        switch ((unsigned char)character) {
        case 0xe1: return '[';
        case 0xe9: return '\\';
        case 0xed: return ']';
        case 0xf3: return '^';
        case 0xfa: return '_';
        case 0xf1: return '=';
        case 0xa1: return '>';
        case 0xbf: return '<';
        }
    }
    return character;
}

static int TranslateFont(short section, const unsigned char *source, size_t size,
                          unsigned char **output, size_t *outputSize)
{
    TranslationFont *font = &fonts[section];
    const unsigned char *glyph = font->glyphs;
    unsigned char *result;
    size_t total = size;
    size_t index;

    if (size < 772 || ReadTranslation16(source) != font->height ||
        source[2] != font->ink || source[3] != font->background ||
        SDL_crc32(0, source, size) != font->crc) {
        SDL_SetError("Translation requires the original WC2 fonts");
        return 0;
    }
    for (index = 0; index < font->count; index++) {
        total += glyph[1] * font->height;
        glyph += 2 + glyph[1] * font->height;
    }
    if (total > 65535)
        return 0;
    result = malloc(total);
    if (!result)
        return 0;
    memcpy(result, source, size);
    glyph = font->glyphs;
    for (index = 0; index < font->count; index++) {
        size_t pixels = glyph[1] * font->height;
        unsigned char character = glyph[0];
        result[4 + character] = glyph[1];
        result[260 + character] = (unsigned char)size;
        result[516 + character] = (unsigned char)(size >> 8);
        memcpy(result + size, glyph + 2, pixels);
        size += pixels;
        glyph += pixels + 2;
    }
    *output = result;
    *outputSize = total;
    return 1;
}

static int TranslateArtwork(const char *name, short section,
                            const unsigned char *source, size_t size,
                            unsigned char **output, size_t *outputSize)
{
    size_t index;
    for (index = 0; index < artworkCount; index++) {
        const TranslationArtwork *patch = &artwork[index];
        const unsigned char *edit = patch->edits;
        unsigned char *result;
        size_t i, input = 0, used = 0;
        if (patch->section != section || SDL_strcasecmp(name, patch->filename))
            continue;
        if (size != patch->sourceSize || SDL_crc32(0, source, size) != patch->sourceCrc) {
            return 1;
        }
        result = malloc(patch->targetSize);
        if (!result)
            return 0;
        for (i = 0; i < patch->count; i++) {
            size_t offset = ReadTranslation32(edit);
            size_t removed = ReadTranslation32(edit + 4);
            size_t inserted = ReadTranslation32(edit + 8);
            size_t copied = offset - input;
            edit += 12;
            if (copied > patch->targetSize - used || inserted > patch->targetSize - used - copied) {
                free(result);
                return 0;
            }
            memcpy(result + used, source + input, copied);
            used += copied;
            memcpy(result + used, edit, inserted);
            used += inserted;
            input = offset + removed;
            edit += inserted;
        }
        if (size - input != patch->targetSize - used) {
            free(result);
            return 0;
        }
        memcpy(result + used, source + input, size - input);
        if (SDL_crc32(0, result, patch->targetSize) != patch->targetCrc) {
            free(result);
            return 0;
        }
        *output = result;
        *outputSize = patch->targetSize;
        return 1;
    }
    return 1;
}

static Uint32 ReadTranslationBig32(const unsigned char *p)
{
    return ((Uint32)p[0] << 24) | ((Uint32)p[1] << 16) |
        ((Uint32)p[2] << 8) | p[3];
}

static void WriteTranslationBig32(unsigned char *p, size_t value)
{
    p[0] = (unsigned char)(value >> 24);
    p[1] = (unsigned char)(value >> 16);
    p[2] = (unsigned char)(value >> 8);
    p[3] = (unsigned char)value;
}

/* TEXT/SYMB is a NUL-separated string table. Rebuild only that chunk and
 * its enclosing lengths; script opcodes, resource indices and timing stay
 * unchanged. Unknown layouts pass through without being interpreted. */
static int TranslateCutscene(const char *name, short section,
                             const unsigned char *source, size_t size,
                             unsigned char **output, size_t *outputSize)
{
    size_t position = 12;

    if (size < 12 || memcmp(source, "FORM", 4) ||
        memcmp(source + 8, "CSCP", 4) ||
        ReadTranslationBig32(source + 4) != size - 8)
        return 1;
    while (position + 12 <= size) {
        size_t length = ReadTranslationBig32(source + position + 4);
        size_t oldLength, newLength = 0, cursor, index, used, total;
        const unsigned char *strings;
        unsigned char *result;
        int changed = 0;
        char context[96];

        if (length < 4 || length > size - position - 8)
            return 1;
        if (memcmp(source + position, "FORM", 4) ||
            memcmp(source + position + 8, "TEXT", 4)) {
            position += 8 + ((length + 1) & ~(size_t)1);
            continue;
        }
        if (length < 12 || memcmp(source + position + 12, "SYMB", 4))
            return 1;
        oldLength = ReadTranslationBig32(source + position + 16);
        if (oldLength > length - 12 || ((oldLength + 1) & ~(size_t)1) != length - 12)
            return 1;
        strings = source + position + 20;
        for (cursor = 0, index = 0; cursor < oldLength; index++) {
            const char *original = (const char *)strings + cursor;
            const char *end = memchr(original, 0, oldLength - cursor);
            const char *translated;
            size_t added;
            if (!end)
                return 1;
            SDL_snprintf(context, sizeof(context), "cutscene/%s/packet/%d/text/%u",
                         name, section, (unsigned int)index);
            translated = FindTranslation(context, original);
            added = strlen(translated) + 1;
            if (added > TRANSLATION_LIMIT - newLength)
                return 0;
            newLength += added;
            changed |= translated != original;
            cursor += (size_t)(end - original) + 1;
        }
        if (!changed)
            return 1;
        total = size - ((oldLength + 1) & ~(size_t)1) + ((newLength + 1) & ~(size_t)1);
        if (total > TRANSLATION_LIMIT)
            return 0;
        result = malloc(total);
        if (!result)
            return 0;
        memcpy(result, source, position + 20);
        used = position + 20;
        for (cursor = 0, index = 0; cursor < oldLength; index++) {
            const char *original = (const char *)strings + cursor;
            const char *translated;
            size_t added;
            SDL_snprintf(context, sizeof(context), "cutscene/%s/packet/%d/text/%u",
                         name, section, (unsigned int)index);
            translated = FindTranslation(context, original);
            added = strlen(translated) + 1;
            memcpy(result + used, translated, added);
            used += added;
            cursor += strlen(original) + 1;
        }
        if (newLength & 1)
            result[used++] = 0;
        cursor = position + 8 + length;
        memcpy(result + used, source + cursor, size - cursor);
        WriteTranslationBig32(result + 4, total - 8);
        WriteTranslationBig32(result + position + 4, 12 + ((newLength + 1) & ~(size_t)1));
        WriteTranslationBig32(result + position + 16, newLength);
        *output = result;
        *outputSize = total;
        return 1;
    }
    return 1;
}

static int TranslationUsesPacket(const char *name, short section)
{
    size_t index;
    if (!SDL_strcasecmp(name, "FONTS.FNT"))
        return section >= 0 && section < 3 && fonts[section].glyphs;
    if (!SDL_strcasecmp(name, "SERIES.S00") || !SDL_strcasecmp(name, "INCIDENT.S00"))
        return 1;
    if (!SDL_strcasecmp(name, "MODULE.000") || !SDL_strcasecmp(name, "COMMUNIC.YOU") ||
        (strlen(name) == 12 && !SDL_strncasecmp(name, "COMMUNIC.T", 10) &&
         name[10] >= '0' && name[10] <= '2' && name[11] >= '0' && name[11] <= '9'))
        return 1;
    for (index = 0; index < artworkCount; index++) {
        if (section == artwork[index].section && !SDL_strcasecmp(name, artwork[index].filename))
            return 1;
    }
    return 0;
}

static int TranslateModule(short section, const unsigned char *source, size_t size,
                            unsigned char **output)
{
    size_t stride, offset, capacity, index;
    switch (section) {
    case 1: stride = 101; offset = 0; capacity = 30; break;
    case 2: stride = 64; offset = 4; capacity = 60; break;
    case 3: stride = 60; offset = 0; capacity = 19; break;
    case 4: case 5: stride = 40; offset = 0; capacity = 40; break;
    default: return 1;
    }
    if (!size || size % stride)
        return 1;
    for (index = 0; index < size / stride; index++) {
        const char *original = (const char *)source + index * stride + offset;
        const char *translated;
        char context[96];
        if (!memchr(original, 0, capacity))
            continue;
        SDL_snprintf(context, sizeof(context), "module/MODULE.000/packet/%d/record/%u",
                     section, (unsigned int)index);
        translated = FindTranslation(context, original);
        if (translated == original || strlen(translated) >= capacity ||
            ((original[0] == '.') != (translated[0] == '.')))
            continue;
        if (!*output) {
            *output = malloc(size);
            if (!*output)
                return 0;
            memcpy(*output, source, size);
        }
        memset(*output + index * stride + offset, 0, capacity);
        memcpy(*output + index * stride + offset, translated, strlen(translated));
    }
    return 1;
}

static int SdlTranslatePacketData(const char *filename, short section,
                                 const unsigned char *source, size_t size,
                                 unsigned char **output, size_t *outputSize)
{
    const char *name = TranslationBasename(filename);
    *output = 0;
    *outputSize = size;
    /* The bundled campaign translation contains only the base game's data.
     * The expansions reuse some logical names with different text records. */
    if (!catalog || section < 0 || size > TRANSLATION_LIMIT)
        return 1;
    if (!SDL_strcasecmp(name, "FONTS.FNT") && section < 3 && fonts[section].glyphs)
        return TranslateFont(section, source, size, output, outputSize);
    if (g_nSdlCampaign != 0)
        return 1;
    if (!SDL_strcasecmp(name, "SERIES.S00"))
        return TranslateCutscene("SERIES.S00", section, source, size, output, outputSize);
    if (!SDL_strcasecmp(name, "INCIDENT.S00"))
        return TranslateCutscene("INCIDENT.S00", section, source, size, output, outputSize);
    if (!SDL_strcasecmp(name, "MODULE.000"))
        return TranslateModule(section, source, size, output);
    if (!SDL_strncasecmp(name, "COMMUNIC.", 9) && memchr(source, 0, size)) {
        char canonical[16], context[96];
        const char *translated;
        SDL_strlcpy(canonical, name, sizeof(canonical));
        SDL_strupr(canonical);
        SDL_snprintf(context, sizeof(context), "radio/%s/packet/%d", canonical, section);
        translated = FindTranslation(context, (const char *)source);
        if (translated != (const char *)source && strlen(translated) < 88) {
            *outputSize = strlen(translated) + 1;
            *output = malloc(*outputSize);
            if (!*output)
                return 0;
            memcpy(*output, translated, *outputSize);
        }
        return 1;
    }
    return TranslateArtwork(name, section, source, size, output, outputSize);
}

/* Keep WC2's name/value placeholders, but allow the CP1252 glyphs used by
 * external catalogs. Bound expansion to the existing 256-byte workspace. */
void SdlExpandCutsceneText(const unsigned char *source, char *destination)
{
    size_t used = 0;
    if (!destination)
        return;
    memset(destination, 0, 256);
    while (*source && used < 255) {
        unsigned char character = *source++;
        const char *replacement = 0;
        char number[16];
        short value = 0;
        int numeric = 0;
        switch (character) {
        case 0x85: replacement = g_stPilotNameVariables.lastName; break;
        case 0x86: replacement = g_stPilotNameVariables.callsign; break;
        case 0x89: replacement = g_stPilotNameVariables.firstName; break;
        case 0x40: value = g_117_nCutsceneFormatValueWord; numeric = 1; break;
        case 0x87: value = ((short *)g_pCampaignGlobals)[54]; numeric = 1; break;
        case 0x88: value = ((short *)g_pCampaignGlobals)[55]; numeric = 1; break;
        default: destination[used++] = (char)character; break;
        }
        if (numeric) {
            SDL_snprintf(number, sizeof(number), "%d", value);
            replacement = number;
        }
        if (replacement) {
            size_t length = strlen(replacement);
            if (length > 255 - used)
                length = 255 - used;
            memcpy(destination + used, replacement, length);
            used += length;
        }
    }
}

int SdlTranslatePacketSection(const char *filename, short section,
                              PacketSectionHandle *handle)
{
    const char *name = TranslationBasename(filename);
    unsigned char *stored = 0;
    unsigned char *decoded = 0;
    unsigned char *result = 0;
    size_t size = handle->dataSize;
    size_t resultSize = size;
    size_t readSize = 0;
    size_t written;
    int success = 0;

    if (!catalog || !TranslationUsesPacket(name, section))
        return 1;
    if (!size)
        return 1;
    if (size > TRANSLATION_LIMIT)
        goto done;
    if (handle->patchedData) {
        decoded = handle->patchedData;
    } else {
        stored = malloc(size);
        if (!stored || _lseek(handle->file, handle->dataOffset, SEEK_SET) == -1)
            goto done;
        while (readSize < size) {
            int count = _read(handle->file, stored + readSize, size - readSize);
            if (count <= 0)
                goto done;
            readSize += count;
        }
        if (handle->compression == 1 || handle->compression == 0x20) {
            if (size < 4)
                goto done;
            size = ReadTranslation32(stored);
            if (!size || size > TRANSLATION_LIMIT)
                goto done;
            decoded = malloc(size);
            if (!decoded || !SdlDecompressOriginLzw(stored + 4, readSize - 4, decoded, size, &written) || written != size)
                goto done;
        } else {
            decoded = stored;
        }
    }
    success = SdlTranslatePacketData(filename, section, decoded, size, &result, &resultSize);
done:
    if (decoded != stored && decoded != handle->patchedData)
        free(decoded);
    free(stored);
    if (_lseek(handle->file, handle->dataOffset, SEEK_SET) == -1)
        success = 0;
    if (!success) {
        free(result);
        g_nPacketError = 6;
        return 0;
    }
    if (result) {
        free(handle->patchedData);
        handle->patchedData = result;
        handle->dataSize = (unsigned int)resultSize;
        handle->compression = 0;
    }
    return 1;
}
