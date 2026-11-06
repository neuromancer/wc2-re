/*
 *  ix stream-file methods. The filename is inferred from the class;
 *  no diagnostic string names this source file in the retail image.
 *
 *  Retail address range: 0x0046DD00 - 0x0046DF07 (6 functions).
 *  The initial 16-byte boundary follows ten padding bytes after the
 *  thread helpers. See docs/ORDER.md for compilation-unit evidence.
 */
#include "ix.h"

/* Function start: 0x46DD00 */
unsigned int IxStreamFile::ix_stream_file_read(void *newDestination,
                                                unsigned int bytes)
{
    ix_stream_file_wait();
    EnterCriticalSection(&g_csStreamerFileFifo_005c4c00);
    if (position + bytes > size)
        bytes = size - position;
    if (bytes > 0) {
        destination = (unsigned char *)newDestination;
        remaining = bytes;
        flags |= IX_STREAM_FILE_READING;
        if (previous != 0)
            previous->next = next;
        if (next == 0)
            g_pPumpIdleFiles_005c4b48 = previous;
        else
            next->previous = previous;
        previous = g_pPlayerReadQueue_005c4bd4;
        next = 0;
        if (g_pPlayerReadQueue_005c4bd4 != 0)
            g_pPlayerReadQueue_005c4bd4->next = this;
        g_pPlayerReadQueue_005c4bd4 = this;
    }
    LeaveCriticalSection(&g_csStreamerFileFifo_005c4c00);
    SetEvent(g_hStreamRouseEvent_005c4c3c);
    return bytes;
}

/* Function start: 0x46DE09 */
void IxStreamFile::ix_stream_file_seek(unsigned int newPosition)
{
    ix_stream_file_wait();
    if (newPosition >= 0)
        position = size < newPosition ? size : newPosition;
    else
        position = 0;
}

/* Function start: 0x46DE5B */
unsigned int IxStreamFile::ix_stream_file_tell(void)
{
    ix_stream_file_wait();
    return position;
}

/* Function start: 0x46DE7F */
unsigned int IxStreamFile::ix_stream_file_size(void)
{
    return size;
}

/* Function start: 0x46DE9B */
void IxStreamFile::ix_stream_file_wait(void)
{
    if ((flags & IX_STREAM_FILE_READING) != 0) {
        WaitForSingleObject(completionEvent, INFINITE);
        ResetEvent(completionEvent);
    }
}

/* Function start: 0x46DED9 */
int IxStreamFile::ix_stream_file_is_reading(void)
{
    return (flags & IX_STREAM_FILE_READING) != 0;
}
