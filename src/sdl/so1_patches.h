/* SO1 S4M2, Mines nav point: the original MODULE.001 preloads SHIP.V37,
 * which is absent from both the DOS data and Origin's KS expansion archive.
 * This nav has no ships, wave commands or following waves. Disable only that
 * unused preload; do not substitute different ship art. The decoded nav packet
 * is identical in both releases. Keep unknown/modified missions untouched.
 * Original KS archive: https://download.wcnews.com/files/ks/wcksso1.zip */
static const unsigned char so1NavBytes[] = { 0xff, 0xff };
static const SdlDataEdit so1NavEdits[] = {
    { 0x44e5, 2, 2, 0 }
};
static const SdlDataPatch so1PacketPatches[] = {
    { "MODULE.001", 1, 64640, 0x9b98a523U,
      64640, 0xd740f6e3U,
      so1NavEdits, SDL_arraysize(so1NavEdits), so1NavBytes, sizeof(so1NavBytes) }
};
