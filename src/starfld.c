/*
 *  Fireworks and constellation fields.
 *
 *  Retail object 0x45A2C0-0x45AC4F (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_asConstellationVelocity[16] = {
    8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1
}; /* 0x0049cae0 */
short g_asConstellationFrame[16] = {
    0, 16, 16, 0, 4, 4, 20, 20, 24, 8, 8, 24, 28, 12, 12, 28
}; /* 0x0049cb00 */
short g_nConstellationDirection = -1; /* 0x0049cb20 */

/* Function start: 0x45A2C0 */
void InitializeFireworks(void)
{
    short i;

    for (i = 0; i < 0x1e; i++)
        g_16_aFireworks[i].frame = -1;
}

/* Function start: 0x45A300 */
short TheEndFireWorks(Viewport *viewport, short count)
{
    short inactiveCount = 0;
    short index;

    inactiveCount = 0;
    for (index = (short)(count - 1); index >= 0; index--) {
        if (g_16_aFireworks[index].frame == -1) {
            inactiveCount++;
            continue;
        }
        DrawSpriteDefault(
            viewport, g_16_aFireworks[index].x,
            g_16_aFireworks[index].y,
            g_1_pTitleFireworkShape,
            (short)(g_16_aFireworks[index].frame +
                    g_16_aFireworks[index].variant * 8));
        if (g_16_aFireworks[index].frame++ == 7) {
            g_16_aFireworks[index].frame = -1;
            ((void (__cdecl *)(int, short))FlushSoundEffectsAndLog)(
                g_16_aFireworks[index].soundHandle, index);
        } else if (g_16_aFireworks[index].frame == 1) {
            PlaySfxWaveFileByNumber(0x25, -1, 0);
        }
    }
    return inactiveCount;
}

/* Function start: 0x45A441 */
void InitializeConstellationField(Viewport *viewport, short direction,
                                  short density)
{
    short width;
    short particle;
    short randomSlot;
    short height;

    g_pConstellationWindow = viewport;
    g_nConstellationDirection = direction;
    width = (short)(g_pConstellationWindow->right -
                    g_pConstellationWindow->left);
    height = (short)(g_pConstellationWindow->bottom -
                     g_pConstellationWindow->top);
    g_14_nConstellationParticleTotal =
        (short)(density * 16 / 16);
    for (particle = 0; particle < g_14_nConstellationParticleTotal;
         particle++) {
        if (g_nConstellationDirection != 0) {
            randomSlot = RandomInRange(0, 15);
            g_aConstellationParticlesTable[particle].x =
                (short)(g_pConstellationWindow->left +
                        RandomInRange(0, width));
            g_aConstellationParticlesTable[particle].y =
                (short)(g_pConstellationWindow->top +
                        RandomInRange(0, height));
            g_aConstellationParticlesTable[particle].deltaX =
                (short)(g_asConstellationVelocity[randomSlot] *
                        g_nConstellationDirection);
            g_aConstellationParticlesTable[particle].frame =
                (short)(g_asConstellationFrame[randomSlot] +
                        RandomInRange(0, 3));
        } else {
            g_aConstellationParticlesTable[particle].x =
                (short)(g_pConstellationWindow->left - 1);
            g_aConstellationParticlesTable[particle].frame = 0;
        }
    }
}

/* Function start: 0x45A5C7 */
signed char IsConstellationParticleOutsideViewport(
    ConstellationParticle particle)
{
    if (g_pConstellationWindow->left > particle.x ||
        g_pConstellationWindow->right < particle.x ||
        g_pConstellationWindow->top > particle.y ||
        g_pConstellationWindow->bottom < particle.y)
        return 1;
    return 0;
}

/* Function start: 0x45A634 */
void DrawConstellationField(void)
{
    short midX;
    short speed;
    short midY;
    short width;
    short i;
    short randomIndex;
    short height;
    short phase;

    height = (short)(g_pConstellationWindow->bottom -
                     g_pConstellationWindow->top);
    for (i = 0; i < g_14_nConstellationParticleTotal;
         i++) {
        DrawSpriteDefault(g_pConstellationWindow,
                          g_aConstellationParticlesTable[i].x,
                          g_aConstellationParticlesTable[i].y,
                          g_pConstellationSprite,
                          g_aConstellationParticlesTable[i].frame);
        g_aConstellationParticlesTable[i].x = (short)(
            g_aConstellationParticlesTable[i].x +
            g_aConstellationParticlesTable[i].deltaX);
        g_aConstellationParticlesTable[i].y = (short)(
            g_aConstellationParticlesTable[i].y +
            g_aConstellationParticlesTable[i].deltaY);
        phase = (short)(
            (g_aConstellationParticlesTable[i].frame + 1) % 4);
        g_aConstellationParticlesTable[i].frame = (short)(
            (g_aConstellationParticlesTable[i].frame & 0xfc) +
            phase);
        switch (g_nConstellationDirection) {
        case -1:
            if (IsConstellationParticleOutsideViewport(
                    g_aConstellationParticlesTable[i]) != 0) {
                randomIndex = RandomInRange(0, 15);
                speed = g_asConstellationVelocity[randomIndex];
                g_aConstellationParticlesTable[i].x = (short)(
                    g_pConstellationWindow->right -
                    RandomInRange(0, speed));
                g_aConstellationParticlesTable[i].y = (short)(
                    g_pConstellationWindow->top +
                    RandomInRange(0, height));
                g_aConstellationParticlesTable[i].deltaX =
                    (short)-speed;
                g_aConstellationParticlesTable[i].deltaY = 0;
                g_aConstellationParticlesTable[i].frame =
                    (short)(g_asConstellationFrame[randomIndex] +
                            RandomInRange(0, 3));
            }
            break;
        case 0:
            if (IsConstellationParticleOutsideViewport(
                    g_aConstellationParticlesTable[i]) != 0) {
                randomIndex = RandomInRange(0, 15);
                speed = g_asConstellationVelocity[randomIndex];
                width = (short)(g_pConstellationWindow->right -
                                g_pConstellationWindow->left);
                height = (short)(g_pConstellationWindow->bottom -
                                 g_pConstellationWindow->top);
                g_aConstellationParticlesTable[i].x = (short)(
                    g_pConstellationWindow->left +
                    RandomInRange(0, width));
                g_aConstellationParticlesTable[i].y = (short)(
                    g_pConstellationWindow->top +
                    RandomInRange(0, height));
                midX = (short)(
                    (g_pConstellationWindow->right -
                     g_pConstellationWindow->left) / 2 +
                    g_pConstellationWindow->left);
                midY = (short)(
                    (g_pConstellationWindow->bottom -
                     g_pConstellationWindow->top) / 2 +
                    g_pConstellationWindow->top);
                if (g_aConstellationParticlesTable[i].x < midX &&
                    g_aConstellationParticlesTable[i].y < midY) {
                    g_aConstellationParticlesTable[i].deltaX =
                        (short)-speed;
                    g_aConstellationParticlesTable[i].deltaY =
                        (short)-speed;
                } else if (
                    g_aConstellationParticlesTable[i].x < midX &&
                    g_aConstellationParticlesTable[i].y >= midY) {
                    g_aConstellationParticlesTable[i].deltaX =
                        (short)-speed;
                    g_aConstellationParticlesTable[i].deltaY = speed;
                } else if (
                    g_aConstellationParticlesTable[i].x >= midX &&
                    g_aConstellationParticlesTable[i].y < midY) {
                    g_aConstellationParticlesTable[i].deltaX = speed;
                    g_aConstellationParticlesTable[i].deltaY =
                        (short)-speed;
                } else {
                    g_aConstellationParticlesTable[i].deltaX = speed;
                    g_aConstellationParticlesTable[i].deltaY = speed;
                }
                g_aConstellationParticlesTable[i].frame =
                    (short)(g_asConstellationFrame[randomIndex] +
                            RandomInRange(0, 3));
            }
            break;
        case 1:
            if (IsConstellationParticleOutsideViewport(
                    g_aConstellationParticlesTable[i]) != 0) {
                randomIndex = RandomInRange(0, 15);
                speed = g_asConstellationVelocity[randomIndex];
                g_aConstellationParticlesTable[i].deltaX = speed;
                g_aConstellationParticlesTable[i].deltaY = 0;
                g_aConstellationParticlesTable[i].x = (short)(
                    g_pConstellationWindow->left +
                    RandomInRange(0, speed));
                g_aConstellationParticlesTable[i].y = (short)(
                    g_pConstellationWindow->top +
                    RandomInRange(0, height));
                g_aConstellationParticlesTable[i].frame =
                    (short)(g_asConstellationFrame[randomIndex] +
                            RandomInRange(0, 3));
            }
            break;
        }
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
FireworkState g_16_aFireworks[30]; /* 0x005c8df0 */
unsigned char *g_1_pTitleFireworkShape; /* 0x005c8f58 */
Viewport *g_pConstellationWindow; /* 0x005c8f5c */
ConstellationParticle g_aConstellationParticlesTable[18]; /* 0x005c8f60 */
short g_14_nConstellationParticleTotal; /* 0x005c9014 */
char g_nUnusedStarfldWord; /* 0x005c9016 */ /* never referenced */
short g_8_bSceneBackdropClicked; /* 0x005c9018 */
short g_1_nCockpitArchiveVariantNumber; /* 0x005c901a */
