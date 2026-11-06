/* Build utility for MSPDB41.DLL, not part of the reconstructed game.
 * PDBOpen accepts the initial signature and starts new databases at age 0.
 * Each later write-open increments that age; read-opens leave it unchanged.
 * Create a seed at age 21 so LINK's write-open produces retail age 22.
 */
#include <windows.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef BOOL (__cdecl *OpenPdb)(char *, char *, DWORD, long *, char *, void **);
typedef DWORD (__cdecl *QueryPdb)(void *);
typedef BOOL (__cdecl *ClosePdb)(void *);

int main(int argc, char **argv)
{
    HMODULE library;
    OpenPdb openPdb;
    QueryPdb querySignature;
    QueryPdb queryAge;
    QueryPdb queryVersion;
    ClosePdb closePdb;
    void *pdb;
    unsigned long values[2];
    unsigned long signature;
    unsigned long age;
    unsigned long version;
    unsigned long iteration;
    DWORD attributes;
    DWORD fileError;
    long error;
    char message[1024];
    char *end;
    int index;
    int verifyOnly;

    if (argc != 6 ||
        (strcmp(argv[1], "create") != 0 && strcmp(argv[1], "verify") != 0)) {
        fprintf(stderr, "usage: createPdbSeed {create|verify} DLL PDB signature age\n");
        return 2;
    }
    verifyOnly = strcmp(argv[1], "verify") == 0;
    for (index = 0; index < 2; index++) {
        errno = 0;
        values[index] = strtoul(argv[index + 4], &end, 0);
        if (argv[index + 4][0] < '0' || argv[index + 4][0] > '9' ||
            *end != '\0' || errno == ERANGE) {
            fprintf(stderr, "signature and age must be unsigned integers\n");
            return 2;
        }
    }
    if (values[0] == 0 || values[1] > 4096) {
        fprintf(stderr, "signature must be nonzero; seed age must be at most 4096\n");
        return 2;
    }

    attributes = GetFileAttributesA(argv[3]);
    fileError = GetLastError();
    if (!verifyOnly && (attributes != 0xffffffff ||
        (fileError != ERROR_FILE_NOT_FOUND && fileError != ERROR_PATH_NOT_FOUND))) {
        fprintf(stderr, "seed output already exists or cannot be inspected: %s\n", argv[3]);
        return 1;
    }
    library = LoadLibraryA(argv[2]);
    if (library == 0) {
        fprintf(stderr, "cannot load %s: error %lu\n", argv[2], GetLastError());
        return 1;
    }
    openPdb = (OpenPdb)GetProcAddress(library, "PDBOpen");
    querySignature = (QueryPdb)GetProcAddress(library, "PDBQuerySignature");
    queryAge = (QueryPdb)GetProcAddress(library, "PDBQueryAge");
    queryVersion = (QueryPdb)GetProcAddress(library, "PDBQueryImplementationVersion");
    closePdb = (ClosePdb)GetProcAddress(library, "PDBClose");
    if (!openPdb || !querySignature || !queryAge || !queryVersion || !closePdb) {
        fprintf(stderr, "missing MSPDB41 API exports\n");
        FreeLibrary(library);
        return 1;
    }

    for (iteration = verifyOnly ? values[1] : 0; iteration <= values[1]; iteration++) {
        error = 0;
        message[0] = '\0';
        pdb = 0;
        if (!openPdb(argv[3], verifyOnly ? "r" : "w", values[0],
                     &error, message, &pdb)) {
            fprintf(stderr, "PDBOpen failed: %ld %s\n", error, message);
            FreeLibrary(library);
            return 1;
        }
        signature = querySignature(pdb);
        age = queryAge(pdb);
        version = queryVersion(pdb);
        if (!closePdb(pdb)) {
            fprintf(stderr, "PDBClose failed\n");
            FreeLibrary(library);
            return 1;
        }
        if (signature != values[0] || age != iteration || version != 19950814) {
            fprintf(stderr, "unexpected PDB identity: signature=%08lX age=%lu version=%lu\n",
                    signature, age, version);
            FreeLibrary(library);
            return 1;
        }
    }
    FreeLibrary(library);
    printf("%s PDB signature=0x%08lX age=%lu\n",
           verifyOnly ? "Verified" : "Created", values[0], values[1]);
    return 0;
}
