/*
 *  Game version string.
 *
 *  A data-only unit: retail places the pointer and the literal it points at
 *  between loadmisn.c's and fl.c's data (0x49B528-0x49B534), and no code
 *  between those two units' functions belongs to it.
 */
#include "game.h"

char *g_pszGameVersion_0049b528 = "1.03F-95";
