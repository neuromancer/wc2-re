/* Checksum-guarded corrections to mission data that the original games
 * cannot load.  Each disables only a reference to ship art that no release
 * includes; none substitutes different art.  Unknown or modified missions
 * pass through untouched. */
static const unsigned char missionDisabledEntryBytes[] = { 0xff, 0xff };

/* WC2 series 8, fourth mission (the K'Tithrak Mang strike): navs 6 and 7,
 * "Minefield", each preload resource 32 as object type 0x33 and list one
 * mission ship (13 and 14) of that type.  SHIP.V32 is absent from both the
 * DOS data and Kilrathi Saga, so retail exits with a disk error if the
 * player comes within either nav's 10,000-unit radius.  That takes flying
 * well off course: neither nav is on the mission's flight path, and no
 * autopilot leg passes within about 31,000 units.  The DOS release started
 * at nav 6 (WC2 Origin s8 m3 l as6) stops with "ship.v32, packet 38".
 * Every other minefield uses hazard object type 6 instead.  Disable the two preloads and remove
 * the two ships from the navs, leaving them empty.  The decoded nav packet
 * is identical in both releases. */
static const SdlDataEdit wc2NavEdits[] = {
    { 0x8cb3, 2, 2, 0 },
    { 0x8cc5, 2, 2, 0 },
    { 0x8d18, 2, 2, 0 },
    { 0x8d2a, 2, 2, 0 }
};

/* SO1 S4M2, Mines nav point: the original MODULE.001 preloads SHIP.V37,
 * which is absent from both the DOS data and Origin's KS expansion archive.
 * This nav has no ships, wave commands or following waves. Disable only that
 * unused preload; do not substitute different ship art. The decoded nav packet
 * is identical in both releases. Keep unknown/modified missions untouched.
 * Original KS archive: https://download.wcnews.com/files/ks/wcksso1.zip */
static const SdlDataEdit so1NavEdits[] = {
    { 0x44e5, 2, 2, 0 }
};

static const SdlDataPatch missionPacketPatches[] = {
    { "MODULE.000", 1, 64640, 0xed5c09edU,
      64640, 0xb5dcc479U,
      wc2NavEdits, SDL_arraysize(wc2NavEdits),
      missionDisabledEntryBytes, sizeof(missionDisabledEntryBytes) },
    { "MODULE.001", 1, 64640, 0x9b98a523U,
      64640, 0xd740f6e3U,
      so1NavEdits, SDL_arraysize(so1NavEdits),
      missionDisabledEntryBytes, sizeof(missionDisabledEntryBytes) }
};
