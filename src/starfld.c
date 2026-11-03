/*
 *  Fireworks and constellation fields.
 *
 *  Retail object 0x45A2C0-0x45AC4F (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_asConstellationVelocity_0049cae0[16] = {
    8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1
};
short g_asConstellationFrame_0049cb00[16] = {
    0, 16, 16, 0, 4, 4, 20, 20, 24, 8, 8, 24, 28, 12, 12, 28
};
short g_nConstellationDirection_0049cb20 = -1;

/* Function start: 0x45A2C0 */
void InitializeFireworks(void)
{
    short i;

    for (i = 0; i < 0x1e; i++)
        g_aFireworks_005c8df0[i].frame = -1;
}

/* Function start: 0x45A300 */
short TheEndFireWorks(Viewport *viewport, short count)
{
    short inactiveCount = 0;
    short index;

    inactiveCount = 0;
    for (index = (short)(count - 1); index >= 0; index--) {
        if (g_aFireworks_005c8df0[index].frame == -1) {
            inactiveCount++;
            continue;
        }
        DrawSpriteDefault(
            viewport, g_aFireworks_005c8df0[index].x,
            g_aFireworks_005c8df0[index].y,
            g_pTitleFireworkShape_005c8f58,
            (short)(g_aFireworks_005c8df0[index].frame +
                    g_aFireworks_005c8df0[index].variant * 8));
        if (g_aFireworks_005c8df0[index].frame++ == 7) {
            g_aFireworks_005c8df0[index].frame = -1;
            ((void (__cdecl *)(int, short))FlushSoundEffectsAndLog)(
                g_aFireworks_005c8df0[index].soundHandle, index);
        } else if (g_aFireworks_005c8df0[index].frame == 1) {
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

    g_pConstellationViewport_005c8f5c = viewport;
    g_nConstellationDirection_0049cb20 = direction;
    width = (short)(g_pConstellationViewport_005c8f5c->right -
                    g_pConstellationViewport_005c8f5c->left);
    height = (short)(g_pConstellationViewport_005c8f5c->bottom -
                     g_pConstellationViewport_005c8f5c->top);
    g_nConstellationParticleCount_005c9014 =
        (short)(density * 16 / 16);
    for (particle = 0; particle < g_nConstellationParticleCount_005c9014;
         particle++) {
        if (g_nConstellationDirection_0049cb20 != 0) {
            randomSlot = RandomInRange(0, 15);
            g_aConstellationParticles_005c8f60[particle].x =
                (short)(g_pConstellationViewport_005c8f5c->left +
                        RandomInRange(0, width));
            g_aConstellationParticles_005c8f60[particle].y =
                (short)(g_pConstellationViewport_005c8f5c->top +
                        RandomInRange(0, height));
            g_aConstellationParticles_005c8f60[particle].deltaX =
                (short)(g_asConstellationVelocity_0049cae0[randomSlot] *
                        g_nConstellationDirection_0049cb20);
            g_aConstellationParticles_005c8f60[particle].frame =
                (short)(g_asConstellationFrame_0049cb00[randomSlot] +
                        RandomInRange(0, 3));
        } else {
            g_aConstellationParticles_005c8f60[particle].x =
                (short)(g_pConstellationViewport_005c8f5c->left - 1);
            g_aConstellationParticles_005c8f60[particle].frame = 0;
        }
    }
}

/* Function start: 0x45A5C7 */
signed char IsConstellationParticleOutsideViewport(
    ConstellationParticle particle)
{
    if (g_pConstellationViewport_005c8f5c->left > particle.x ||
        g_pConstellationViewport_005c8f5c->right < particle.x ||
        g_pConstellationViewport_005c8f5c->top > particle.y ||
        g_pConstellationViewport_005c8f5c->bottom < particle.y)
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

    height = (short)(g_pConstellationViewport_005c8f5c->bottom -
                     g_pConstellationViewport_005c8f5c->top);
    for (i = 0; i < g_nConstellationParticleCount_005c9014;
         i++) {
        DrawSpriteDefault(g_pConstellationViewport_005c8f5c,
                          g_aConstellationParticles_005c8f60[i].x,
                          g_aConstellationParticles_005c8f60[i].y,
                          g_pConstellationShape_005d2c4c,
                          g_aConstellationParticles_005c8f60[i].frame);
        g_aConstellationParticles_005c8f60[i].x = (short)(
            g_aConstellationParticles_005c8f60[i].x +
            g_aConstellationParticles_005c8f60[i].deltaX);
        g_aConstellationParticles_005c8f60[i].y = (short)(
            g_aConstellationParticles_005c8f60[i].y +
            g_aConstellationParticles_005c8f60[i].deltaY);
        phase = (short)(
            (g_aConstellationParticles_005c8f60[i].frame + 1) % 4);
        g_aConstellationParticles_005c8f60[i].frame = (short)(
            (g_aConstellationParticles_005c8f60[i].frame & 0xfc) +
            phase);
        switch (g_nConstellationDirection_0049cb20) {
        case -1:
            if (IsConstellationParticleOutsideViewport(
                    g_aConstellationParticles_005c8f60[i]) != 0) {
                randomIndex = RandomInRange(0, 15);
                speed = g_asConstellationVelocity_0049cae0[randomIndex];
                g_aConstellationParticles_005c8f60[i].x = (short)(
                    g_pConstellationViewport_005c8f5c->right -
                    RandomInRange(0, speed));
                g_aConstellationParticles_005c8f60[i].y = (short)(
                    g_pConstellationViewport_005c8f5c->top +
                    RandomInRange(0, height));
                g_aConstellationParticles_005c8f60[i].deltaX =
                    (short)-speed;
                g_aConstellationParticles_005c8f60[i].deltaY = 0;
                g_aConstellationParticles_005c8f60[i].frame =
                    (short)(g_asConstellationFrame_0049cb00[randomIndex] +
                            RandomInRange(0, 3));
            }
            break;
        case 0:
            if (IsConstellationParticleOutsideViewport(
                    g_aConstellationParticles_005c8f60[i]) != 0) {
                randomIndex = RandomInRange(0, 15);
                speed = g_asConstellationVelocity_0049cae0[randomIndex];
                width = (short)(g_pConstellationViewport_005c8f5c->right -
                                g_pConstellationViewport_005c8f5c->left);
                height = (short)(g_pConstellationViewport_005c8f5c->bottom -
                                 g_pConstellationViewport_005c8f5c->top);
                g_aConstellationParticles_005c8f60[i].x = (short)(
                    g_pConstellationViewport_005c8f5c->left +
                    RandomInRange(0, width));
                g_aConstellationParticles_005c8f60[i].y = (short)(
                    g_pConstellationViewport_005c8f5c->top +
                    RandomInRange(0, height));
                midX = (short)(
                    (g_pConstellationViewport_005c8f5c->right -
                     g_pConstellationViewport_005c8f5c->left) / 2 +
                    g_pConstellationViewport_005c8f5c->left);
                midY = (short)(
                    (g_pConstellationViewport_005c8f5c->bottom -
                     g_pConstellationViewport_005c8f5c->top) / 2 +
                    g_pConstellationViewport_005c8f5c->top);
                if (g_aConstellationParticles_005c8f60[i].x < midX &&
                    g_aConstellationParticles_005c8f60[i].y < midY) {
                    g_aConstellationParticles_005c8f60[i].deltaX =
                        (short)-speed;
                    g_aConstellationParticles_005c8f60[i].deltaY =
                        (short)-speed;
                } else if (
                    g_aConstellationParticles_005c8f60[i].x < midX &&
                    g_aConstellationParticles_005c8f60[i].y >= midY) {
                    g_aConstellationParticles_005c8f60[i].deltaX =
                        (short)-speed;
                    g_aConstellationParticles_005c8f60[i].deltaY = speed;
                } else if (
                    g_aConstellationParticles_005c8f60[i].x >= midX &&
                    g_aConstellationParticles_005c8f60[i].y < midY) {
                    g_aConstellationParticles_005c8f60[i].deltaX = speed;
                    g_aConstellationParticles_005c8f60[i].deltaY =
                        (short)-speed;
                } else {
                    g_aConstellationParticles_005c8f60[i].deltaX = speed;
                    g_aConstellationParticles_005c8f60[i].deltaY = speed;
                }
                g_aConstellationParticles_005c8f60[i].frame =
                    (short)(g_asConstellationFrame_0049cb00[randomIndex] +
                            RandomInRange(0, 3));
            }
            break;
        case 1:
            if (IsConstellationParticleOutsideViewport(
                    g_aConstellationParticles_005c8f60[i]) != 0) {
                randomIndex = RandomInRange(0, 15);
                speed = g_asConstellationVelocity_0049cae0[randomIndex];
                g_aConstellationParticles_005c8f60[i].deltaX = speed;
                g_aConstellationParticles_005c8f60[i].deltaY = 0;
                g_aConstellationParticles_005c8f60[i].x = (short)(
                    g_pConstellationViewport_005c8f5c->left +
                    RandomInRange(0, speed));
                g_aConstellationParticles_005c8f60[i].y = (short)(
                    g_pConstellationViewport_005c8f5c->top +
                    RandomInRange(0, height));
                g_aConstellationParticles_005c8f60[i].frame =
                    (short)(g_asConstellationFrame_0049cb00[randomIndex] +
                            RandomInRange(0, 3));
            }
            break;
        }
    }
}
