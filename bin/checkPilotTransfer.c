/* Where WC2's personnel database finds WC1's saved games for a pilot transfer
 * (issue #71), in a scratch folder: wc1-re's launcher.ini in the settings
 * folder this port shares, then WC1 beside WC2's own install, as GOG lays out
 * its Kilrathi Saga and DOS copies and the Kilrathi Saga's own CD does. The
 * path the transfer builds from the folder found opens through the port's
 * file layer, and its records list WC1's pilots as [T]ransfer reads them. No
 * game assets are needed. */
#include "game.h"

#include <ftw.h>
#include <sys/stat.h>
#include <unistd.h>

static char root[512];

static int RemoveEntry(const char *path, const struct stat *status, int type,
                       struct FTW *walk)
{
    (void)status;
    (void)type;
    (void)walk;
    return remove(path);
}

static void Require(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

/* A path under the scratch folder. */
static const char *Path(const char *relative)
{
    static char paths[4][1024];
    static int next;
    char *path = paths[next++ % 4];

    snprintf(path, sizeof(paths[0]), "%s/%s", root, relative);
    return path;
}

static void MakeFolders(const char *relative)
{
    char path[1024];
    char *separator;

    snprintf(path, sizeof(path), "%s", Path(relative));
    for (separator = path + strlen(root) + 1; *separator != '\0'; separator++) {
        if (*separator == '/') {
            *separator = '\0';
            mkdir(path, 0700);
            *separator = '/';
        }
    }
    mkdir(path, 0700);
}

static void WriteText(const char *relative, const char *text)
{
    FILE *file = fopen(Path(relative), "wb");

    Require(file != NULL, "write a settings file");
    fputs(text, file);
    fclose(file);
}

/* Eight WC1 games: two WC1 pilots, a Secret Missions pilot, and empty slots. */
static void WriteSaves(const char *relative, const char *callsign)
{
    SaveGameDiskRecord records[8];
    FILE *file;
    int slot;

    memset(records, 0, sizeof(records));
    for (slot = 0; slot < 3; slot++) {
        PilotRecord *pilot = &records[slot * 2].pilots[8];

        records[slot * 2].occupied = 1;
        records[slot * 2].campaign.campaignIndex = (short)(slot == 2);
        snprintf(pilot->name, sizeof(pilot->name), "Pilot%d", slot);
        snprintf(pilot->callsign, sizeof(pilot->callsign), "%s%d", callsign, slot);
        pilot->rank = (short)(slot + 3);
        pilot->missions = (short)(20 + slot);
        pilot->kills = (short)(40 + slot);
    }
    file = fopen(Path(relative), "wb");
    Require(file != NULL, "write SAVEGAME.WLD");
    Require(fwrite(records, sizeof(records), 1, file) == 1, "fill SAVEGAME.WLD");
    fclose(file);
}

/* SdlFindLegacyGameData with the scratch settings folder and WC2 install. */
static const char *Find(int dosVersion, const char *preferences, const char *install)
{
    static char found[0x400];
    char settings[1024];

    snprintf(settings, sizeof(settings), "%s/", Path(preferences));
    if (!SdlFindLegacyGameData(dosVersion, settings, install ? Path(install) : NULL,
                               found, sizeof(found)))
        return NULL;
    return found;
}

static int Is(const char *found, const char *relative)
{
    return found != NULL && strcmp(found, Path(relative)) == 0;
}

int main(void)
{
    SaveGameDiskRecord record;
    const char *found;
    char pattern[0x420];
    struct _finddata_t findData;
    long search;
    int file;
    int listed;
    int slot;

    Require(sizeof(SaveGameDiskRecord) == 0x33c, "a WC1 game is 0x33c bytes");
    snprintf(root, sizeof(root), "%s/wc2-pilot-transfer-XXXXXX",
             getenv("TMPDIR") != NULL ? getenv("TMPDIR") : "/tmp");
    Require(mkdtemp(root) != NULL, "make a scratch folder");
    /* mkdtemp keeps TMPDIR's spelling; compare with the port's resolved one. */
    Require(SdlResolvePath(root, pattern, sizeof(pattern)), "resolve the scratch folder");
    snprintf(root, sizeof(root), "%s", pattern);

    /* GOG: Wing Commander I with Kilrathi_Saga and DOS beside Wing Commander II. */
    MakeFolders("GOG/Wing Commander I/Kilrathi_Saga/GAMEDAT");
    MakeFolders("GOG/Wing Commander I/DOS/GAMEDAT");
    MakeFolders("GOG/Wing Commander II/Kilrathi_Saga/GAMEDAT");
    MakeFolders("GOG/Wing Commander II/DOS/GAMEDAT");
    MakeFolders("GOG/Wing Commander II/GAMEDAT");
    WriteSaves("GOG/Wing Commander I/Kilrathi_Saga/GAMEDAT/SAVEGAME.WLD", "Saga");
    WriteSaves("GOG/Wing Commander I/DOS/GAMEDAT/SAVEGAME.WLD", "Dos");
    MakeFolders("empty");
    found = Find(0, "empty", "GOG/Wing Commander II/Kilrathi_Saga");
    Require(Is(found, "GOG/Wing Commander I/Kilrathi_Saga/GAMEDAT"),
            "from WC2's Kilrathi Saga, [W]ing Commander is WC1's Kilrathi Saga copy");
    Require(Is(Find(1, "empty", "GOG/Wing Commander II/Kilrathi_Saga"),
               "GOG/Wing Commander I/DOS/GAMEDAT"),
            "and the [D]os version is WC1's DOS copy");
    Require(Is(Find(0, "empty", "GOG/Wing Commander II"),
               "GOG/Wing Commander I/Kilrathi_Saga/GAMEDAT") &&
                Is(Find(1, "empty", "GOG/Wing Commander II/DOS"),
                   "GOG/Wing Commander I/DOS/GAMEDAT"),
            "WC2's DOS copies find the same folders");

    /* wc1-re's launcher.ini comes first for [W]ing Commander: its own section
     * and key, quotes optional, any case for GAMEDAT and SAVEGAME.WLD. */
    MakeFolders("wc1-re's WC1/gamedat");
    WriteSaves("wc1-re's WC1/gamedat/savegame.wld", "Port");
    MakeFolders("prefs");
    snprintf(pattern, sizeof(pattern),
             "\xef\xbb\xbf[pad]\ngame_directory=\"%s\"\n[launcher]\n"
             "game_directory_old=\"%s\"\n  game_directory = \"%s\"  \r\n",
             Path("GOG"), Path("GOG"), Path("wc1-re's WC1"));
    WriteText("prefs/launcher.ini", pattern);
    Require(Is(Find(0, "prefs", "GOG/Wing Commander II/Kilrathi_Saga"),
               "wc1-re's WC1/gamedat"),
            "[W]ing Commander takes the WC1 that wc1-re's launcher keeps");
    Require(Is(Find(1, "prefs", "GOG/Wing Commander II/Kilrathi_Saga"),
               "GOG/Wing Commander I/DOS/GAMEDAT"),
            "and the [D]os version still the DOS copy beside WC2");
    snprintf(pattern, sizeof(pattern), "[launcher]\ngame_directory=%s\n", Path("GOG"));
    WriteText("prefs/launcher.ini", pattern);
    Require(Is(Find(0, "prefs", "GOG/Wing Commander II/Kilrathi_Saga"),
               "GOG/Wing Commander I/Kilrathi_Saga/GAMEDAT"),
            "a launcher folder without WC1's games leaves the one beside WC2");

    /* The Kilrathi Saga's CD install: wc1 beside wc2. Other GOG spellings. */
    MakeFolders("Saga/WC1/GAMEDAT");
    MakeFolders("Saga/WC2/GAMEDAT");
    WriteSaves("Saga/WC1/GAMEDAT/SAVEGAME.WLD", "Disc");
    Require(Is(Find(0, "empty", "Saga/WC2"), "Saga/WC1/GAMEDAT") &&
                Find(1, "empty", "Saga/WC2") == NULL,
            "the Kilrathi Saga's wc2 finds its wc1, with no DOS copy");
    MakeFolders("Galaxy/Wing Commander/Kilrathi_Saga/GAMEDAT");
    MakeFolders("Galaxy/Wing Commander 2/Kilrathi_Saga/GAMEDAT");
    WriteSaves("Galaxy/Wing Commander/Kilrathi_Saga/GAMEDAT/SAVEGAME.WLD", "Galaxy");
    Require(Is(Find(0, "empty", "Galaxy/Wing Commander 2/Kilrathi_Saga"),
               "Galaxy/Wing Commander/Kilrathi_Saga/GAMEDAT"),
            "Wing Commander 2 finds Wing Commander");
    MakeFolders("Lone/Wing Commander II/GAMEDAT");
    Require(Find(0, "empty", "Lone/Wing Commander II") == NULL &&
                Find(1, "empty", "Lone/Wing Commander II") == NULL &&
                Find(0, "missing", NULL) == NULL,
            "without WC1 nothing is found");

    /* The transfer appends a backslash and savegame.wld to the folder, finds
     * the file, then lists each WC1 game's pilot of campaign 0. */
    found = Find(1, "empty", "GOG/Wing Commander II/Kilrathi_Saga");
    snprintf(pattern, sizeof(pattern), "%s\\%s", found, "savegame.wld");
    search = _findfirst(pattern, &findData);
    Require(search != -1, "the transfer's path finds SAVEGAME.WLD");
    _findclose(search);
    file = _open(pattern, 0x8000);
    Require(file >= 0, "and opens it");
    listed = 0;
    for (slot = 0; slot < 8; slot++) {
        Require(_read(file, &record, 0x33c) == 0x33c, "read a WC1 game");
        if (record.occupied != 0 && record.campaign.campaignIndex == 0) {
            Require(strcmp(record.pilots[8].callsign, listed ? "Dos1" : "Dos0") == 0 &&
                        record.pilots[8].missions == 20 + listed &&
                        record.pilots[8].kills == 40 + listed,
                    "a WC1 pilot keeps their callsign, missions and kills");
            listed++;
        }
    }
    _close(file);
    Require(listed == 2, "the transfer lists WC1's two pilots, not Secret Missions'");

    nftw(root, RemoveEntry, 16, FTW_DEPTH | FTW_PHYS);
    puts("PASS: WC2 finds WC1's saved games through wc1-re's launcher, beside its "
         "GOG or Kilrathi Saga install, and the transfer reads their pilots.");
    return 0;
}
