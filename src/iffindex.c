/*
 *  Scene chunk decoding.
 *
 *  Retail object 0x458A90-0x458E0F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x458A90 */
signed char DecodeSceneStructChunk(unsigned char **cursor,
                                   SceneResourceTable **resource)
{
    short ptrSize;
    unsigned int chunkSize;
    unsigned char *record;
    void **slots;
    short index;
    unsigned short step;
    unsigned short recordCount;

    record = *cursor;
    record += 4;
    chunkSize = *(unsigned int *)record;
    record += 4;
    SwapSceneChunkSizeEndian((int *)&chunkSize);
    step = *(unsigned short *)record;
    record += 2;
    recordCount = (unsigned short)((chunkSize - 2) / step);
    (*resource)->type = 0;
    (*resource)->count = recordCount;
    ptrSize = HOST_POINTER_SIZE;
    slots = AllocateScenePointerTable(
        recordCount, ptrSize, 0, "Cannot Allocate STRC stuff");
    (*resource)->data = slots;
    for (index = 1; index <= recordCount; index++) {
        *slots = IdentityHandle(record);
        slots++;
        record += step;
    }
    *cursor += ((chunkSize + 1) & ~1) + 8;
    return 1;
}

/* Function start: 0x458B8C */
signed char DecodeSceneOffsetChunk(unsigned char **cursor,
                                   SceneResourceTable **resource)
{
    void *pointer;
    unsigned int size;
    void **dest;
    short entryIndex;
    unsigned char *data;
    unsigned short count;
    int offset;

    data = *cursor;
    data += 4;
    size = *(unsigned int *)data;
    data += 4;
    SwapSceneChunkSizeEndian((int *)&size);
    count = *(unsigned short *)data;
    data += 2;
    (*resource)->type = 1;
    (*resource)->count = count;
    dest = AllocateScenePointerTable(
        count, HOST_POINTER_SIZE, 0, "Cannot Alloc OFST stuff");
    (*resource)->data = dest;
    for (entryIndex = 1; entryIndex <= count; entryIndex++) {
        offset = *(int *)data;
        if (offset != -1)
            pointer = data + offset;
        else
            pointer = 0;
        data += 4;
        *dest = pointer;
        dest++;
    }
    *cursor += ((size + 1) & ~1) + 8;
    return 1;
}

/* Function start: 0x458C81 */
signed char DecodeSceneSymbolChunk(unsigned char **cursor,
                                   SceneResourceTable **resource)
{
    unsigned int size;
    short ptrSize;
    unsigned char *string;
    unsigned char *data;
    void **table;
    unsigned char *begin;
    short remaining;
    short entryCount;

    data = *cursor;
    begin = data;
    data += 4;
    size = *(unsigned int *)data;
    data += 4;
    SwapSceneChunkSizeEndian((int *)&size);
    begin = data;
    entryCount = 0;
    ptrSize = HOST_POINTER_SIZE;
    do {
        string = (unsigned char *)DosStrchr((char *)data, 0) + 1;
        entryCount++;
        data = string;
    } while (string - begin < (int)size);
    data = begin;
    (*resource)->type = 2;
    (*resource)->count = entryCount;
    table = AllocateScenePointerTable(
        entryCount, ptrSize, 0, "Cannot Alloc SYMB stuff");
    (*resource)->data = table;
    string = begin;
    remaining = entryCount;
    do {
        *table = IdentityHandle(string);
        table++;
        string = (unsigned char *)DosStrchr((char *)string, 0) + 1;
        remaining--;
    } while (remaining != 0);
    *cursor += ((size + 1) & ~1) + 8;
    return 1;
}

/* Function start: 0x458D94 */
signed char DecodeSceneFileChunk(unsigned char **cursor,
                                 SceneResourceTable **resource)
{
    unsigned int size;
    unsigned char *data;

    data = *cursor;
    data += 4;
    size = *(unsigned int *)data;
    data += 4;
    SwapSceneChunkSizeEndian((int *)&size);
    (*resource)->count = *(unsigned short *)data;
    data += 2;
    (*resource)->data = data;
    (*resource)->type = 3;
    *cursor += ((size + 1) & ~1) + 8;
    return 1;
}
