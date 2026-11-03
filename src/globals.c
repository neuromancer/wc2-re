/*
 *  Definitions for the globals declared in include/globals.h.
 *
 *  Provisional: the original scatters these across the modules that own them,
 *  so this file has no counterpart in the shipped image and contributes no
 *  code.  It exists so that regrouping functions cannot break the link.
 */
#include "game.h"

/* The raster library's tables live in src/raster.asm in the Win32 build;
 * the SDL build, which uses the C counterparts in src/sdl/raster.c, keeps
 * them here. */
#ifdef SDL_PORT
/* Quarter-wave 16.16 cosine table embedded with the original raster
 * assembly.  Indices are tenths of a degree from 0 through 90 degrees. */
const int g_anRLEQuarterCosine_00405cbb[901] = {
    65536, 65536, 65536, 65535, 65534, 65534, 65532, 65531,
    65530, 65528, 65526, 65524, 65522, 65519, 65516, 65514,
    65510, 65507, 65504, 65500, 65496, 65492, 65488, 65483,
    65479, 65474, 65469, 65463, 65458, 65452, 65446, 65440,
    65434, 65427, 65421, 65414, 65407, 65399, 65392, 65384,
    65376, 65368, 65360, 65352, 65343, 65334, 65325, 65316,
    65306, 65296, 65287, 65277, 65266, 65256, 65245, 65234,
    65223, 65212, 65201, 65189, 65177, 65165, 65153, 65140,
    65128, 65115, 65102, 65088, 65075, 65061, 65048, 65033,
    65019, 65005, 64990, 64975, 64960, 64945, 64930, 64914,
    64898, 64882, 64866, 64850, 64833, 64816, 64799, 64782,
    64765, 64747, 64729, 64711, 64693, 64675, 64656, 64637,
    64618, 64599, 64580, 64560, 64540, 64520, 64500, 64480,
    64459, 64439, 64418, 64397, 64375, 64354, 64332, 64310,
    64288, 64266, 64243, 64220, 64197, 64174, 64151, 64128,
    64104, 64080, 64056, 64032, 64007, 63983, 63958, 63933,
    63907, 63882, 63856, 63830, 63804, 63778, 63752, 63725,
    63698, 63671, 63644, 63617, 63589, 63562, 63534, 63505,
    63477, 63449, 63420, 63391, 63362, 63332, 63303, 63273,
    63243, 63213, 63183, 63152, 63122, 63091, 63060, 63029,
    62997, 62966, 62934, 62902, 62870, 62837, 62805, 62772,
    62739, 62706, 62672, 62639, 62605, 62571, 62537, 62503,
    62468, 62434, 62399, 62364, 62328, 62293, 62257, 62222,
    62186, 62149, 62113, 62076, 62040, 62003, 61966, 61928,
    61891, 61853, 61815, 61777, 61739, 61700, 61662, 61623,
    61584, 61544, 61505, 61465, 61426, 61386, 61346, 61305,
    61265, 61224, 61183, 61142, 61101, 61059, 61018, 60976,
    60934, 60892, 60849, 60807, 60764, 60721, 60678, 60635,
    60591, 60547, 60504, 60459, 60415, 60371, 60326, 60281,
    60236, 60191, 60146, 60100, 60055, 60009, 59963, 59917,
    59870, 59824, 59777, 59730, 59683, 59635, 59588, 59540,
    59492, 59444, 59396, 59347, 59299, 59250, 59201, 59152,
    59102, 59053, 59003, 58953, 58903, 58853, 58803, 58752,
    58701, 58650, 58599, 58548, 58497, 58445, 58393, 58341,
    58289, 58236, 58184, 58131, 58078, 58025, 57972, 57918,
    57865, 57811, 57757, 57703, 57649, 57594, 57539, 57485,
    57430, 57374, 57319, 57264, 57208, 57152, 57096, 57040,
    56983, 56927, 56870, 56813, 56756, 56699, 56641, 56583,
    56526, 56468, 56410, 56351, 56293, 56234, 56175, 56116,
    56057, 55998, 55938, 55879, 55819, 55759, 55699, 55638,
    55578, 55517, 55456, 55395, 55334, 55273, 55211, 55149,
    55087, 55025, 54963, 54901, 54838, 54775, 54713, 54650,
    54586, 54523, 54459, 54396, 54332, 54268, 54204, 54139,
    54075, 54010, 53945, 53880, 53815, 53749, 53684, 53618,
    53552, 53486, 53420, 53354, 53287, 53221, 53154, 53087,
    53020, 52952, 52885, 52817, 52750, 52682, 52613, 52545,
    52477, 52408, 52339, 52270, 52201, 52132, 52063, 51993,
    51923, 51854, 51784, 51713, 51643, 51573, 51502, 51431,
    51360, 51289, 51218, 51146, 51075, 51003, 50931, 50859,
    50787, 50714, 50642, 50569, 50496, 50423, 50350, 50277,
    50203, 50130, 50056, 49982, 49908, 49834, 49760, 49685,
    49610, 49536, 49461, 49386, 49310, 49235, 49159, 49084,
    49008, 48932, 48856, 48779, 48703, 48626, 48549, 48472,
    48395, 48318, 48241, 48163, 48086, 48008, 47930, 47852,
    47774, 47695, 47617, 47538, 47459, 47380, 47301, 47222,
    47143, 47063, 46983, 46904, 46824, 46744, 46663, 46583,
    46502, 46422, 46341, 46260, 46179, 46098, 46016, 45935,
    45853, 45771, 45689, 45607, 45525, 45443, 45360, 45278,
    45195, 45112, 45029, 44946, 44862, 44779, 44695, 44612,
    44528, 44444, 44360, 44275, 44191, 44107, 44022, 43937,
    43852, 43767, 43682, 43597, 43511, 43425, 43340, 43254,
    43168, 43082, 42995, 42909, 42823, 42736, 42649, 42562,
    42475, 42388, 42301, 42213, 42126, 42038, 41950, 41862,
    41774, 41686, 41598, 41509, 41421, 41332, 41243, 41154,
    41065, 40976, 40887, 40797, 40708, 40618, 40528, 40438,
    40348, 40258, 40167, 40077, 39986, 39896, 39805, 39714,
    39623, 39532, 39441, 39349, 39258, 39166, 39074, 38982,
    38890, 38798, 38706, 38614, 38521, 38428, 38336, 38243,
    38150, 38057, 37964, 37870, 37777, 37684, 37590, 37496,
    37402, 37308, 37214, 37120, 37026, 36931, 36837, 36742,
    36647, 36552, 36457, 36362, 36267, 36172, 36076, 35981,
    35885, 35789, 35693, 35597, 35501, 35405, 35309, 35212,
    35116, 35019, 34923, 34826, 34729, 34632, 34535, 34437,
    34340, 34242, 34145, 34047, 33949, 33852, 33754, 33655,
    33557, 33459, 33361, 33262, 33163, 33065, 32966, 32867,
    32768, 32669, 32570, 32470, 32371, 32271, 32172, 32072,
    31972, 31872, 31772, 31672, 31572, 31472, 31372, 31271,
    31171, 31070, 30969, 30868, 30767, 30666, 30565, 30464,
    30363, 30261, 30160, 30058, 29956, 29855, 29753, 29651,
    29549, 29447, 29344, 29242, 29140, 29037, 28935, 28832,
    28729, 28626, 28523, 28420, 28317, 28214, 28111, 28007,
    27904, 27800, 27697, 27593, 27489, 27385, 27281, 27177,
    27073, 26969, 26865, 26760, 26656, 26551, 26447, 26342,
    26237, 26132, 26027, 25922, 25817, 25712, 25607, 25502,
    25396, 25291, 25185, 25080, 24974, 24868, 24762, 24656,
    24550, 24444, 24338, 24232, 24125, 24019, 23913, 23806,
    23699, 23593, 23486, 23379, 23272, 23165, 23058, 22951,
    22844, 22737, 22629, 22522, 22415, 22307, 22200, 22092,
    21984, 21876, 21769, 21661, 21553, 21445, 21336, 21228,
    21120, 21012, 20903, 20795, 20686, 20578, 20469, 20360,
    20252, 20143, 20034, 19925, 19816, 19707, 19598, 19489,
    19380, 19270, 19161, 19051, 18942, 18832, 18723, 18613,
    18504, 18394, 18284, 18174, 18064, 17954, 17844, 17734,
    17624, 17514, 17403, 17293, 17183, 17072, 16962, 16851,
    16741, 16630, 16520, 16409, 16298, 16187, 16076, 15966,
    15855, 15744, 15633, 15521, 15410, 15299, 15188, 15077,
    14965, 14854, 14742, 14631, 14519, 14408, 14296, 14185,
    14073, 13961, 13849, 13738, 13626, 13514, 13402, 13290,
    13178, 13066, 12954, 12842, 12729, 12617, 12505, 12393,
    12280, 12168, 12055, 11943, 11831, 11718, 11605, 11493,
    11380, 11268, 11155, 11042, 10929, 10817, 10704, 10591,
    10478, 10365, 10252, 10139, 10026, 9913, 9800, 9687,
    9574, 9461, 9347, 9234, 9121, 9008, 8894, 8781,
    8668, 8554, 8441, 8327, 8214, 8100, 7987, 7873,
    7760, 7646, 7533, 7419, 7305, 7192, 7078, 6964,
    6850, 6737, 6623, 6509, 6395, 6281, 6167, 6054,
    5940, 5826, 5712, 5598, 5484, 5370, 5256, 5142,
    5028, 4914, 4800, 4686, 4572, 4457, 4343, 4229,
    4115, 4001, 3887, 3773, 3658, 3544, 3430, 3316,
    3201, 3087, 2973, 2859, 2744, 2630, 2516, 2401,
    2287, 2173, 2059, 1944, 1830, 1716, 1601, 1487,
    1372, 1258, 1144, 1029, 915, 801, 686, 572,
    458, 343, 229, 114, 0
};

/* IFF chunk identifiers embedded between the original raster routines. */
const unsigned char g_abIffBmhdChunkId_00406f74[4] = {
    'B', 'M', 'H', 'D'
};
const unsigned char g_abIffCmapChunkId_00406f78[4] = { 'C', 'M', 'A', 'P' };
const unsigned char g_abIffBodyChunkId_00406f7c[4] = { 'B', 'O', 'D', 'Y' };
#endif





const int g_nJoystickUnavailableSample_0048e054 = -1;
const int g_nViewportStaticRandomRange_0048e060 = 3;
const int g_nDigiSampleBytes_0048e0e0 = 0xfffa;
const unsigned short g_wDmaHalfBytes_0048e0e4 = 0x0fa0;
/* Raster library state: native storage is in src/raster.asm. */
#ifdef SDL_PORT
void *g_apRasterDriverCallbacks_00490278[13];
char g_szRasterDriverName_004902ac[13] = "MCGA.DLL";
unsigned char *g_pRLEEncodeBuffer_004902b9;
int g_nRLEPendingSkip_004902bd;
unsigned char *g_pRLEScanlineStart_004902c1;
unsigned char *g_pRLEScanCursor_004902c5;
unsigned char *g_pRLEOutputCursor_004902c9;
unsigned char *g_pRLELiteralStart_004902cd;
int g_nRLEEncodedMinimumX_004902e1;
int g_nRLEEncodedMinimumY_004902e5;
int g_nRLEEncodedMaximumX_004902e9;
int g_nRLEEncodedMaximumY_004902ed;
unsigned char g_abRasterDecodeBuffer_004907f1[0xd00];
unsigned char g_abRasterPaletteUsageScratch_004914f1[0x300];
unsigned char g_abRasterFadeAccumulator_004917f1[0x300];
const unsigned char g_abGifLZWCodeMasks_00491af1[9] = {
    0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff
};
const unsigned char g_abGifInterlaceRowSteps_00491afa[5] = {
    8, 8, 4, 2, 0
};
const unsigned char g_abGifInterlaceRowStarts_00491aff[5] = {
    0, 4, 2, 1, 0
};
RasterClip *g_pGifRasterTarget_00491b04;
unsigned char g_abRasterPaletteTranslation_00491b08[256];
RLETransformVertex g_aRLETransformVertices_00491c08[4];
int g_anRLESourceSteps_00491c58[4];
#endif

#ifdef SDL_PORT
/* Retail has no storage here: GetAdaptiveTurnRate reads series 0 from the
 * eight bytes before the difficulty table, which end joycal.c's literals
 * ("l" of "j.cal" and padding).  The port keeps those values. */
short g_aasSeriesMissionTurnRate_004930a0[1][4] = { { 108, 0, 0, 0 } };
#endif



short g_nNavMapCoordinateScaling_0049bb80 = 0;
short g_nNavMapScale_0049bb84 = 1;
short g_bBriefingMapActive_0049bcb0 = 0;
short g_nEnemyTauntCommandBase_0049b76c = 20;
short g_nCommMenuChoiceCount_0049b770 = -1;
short g_nCommMenuReuseMode_0049b774;
signed char g_cMessageSpeed_0049b778 = 2;
signed char g_cPendingCommMenuAction_0049b77c = 1;
signed char g_bRadioSilence_0049b780;
signed char g_bVideoImagesSuppressed_0049b784;
unsigned char *g_pCommPortraitResource_0049b788;
unsigned char *g_pCommVduFrameResource_0049b78c;
signed char g_cCommMenuRecipient_0049b790 = -1;
short g_nCommSpeakerObject_0049b794;
short g_nCommSpeakerRating_0049b798;
short g_nCommPortraitIndex_0049b79c = -1;
short g_bCommSpeechPlaying_0049b7a0;
char g_szSelectCommPrompt_0049b7c4[8] = "SELECT";
char g_szCommRecipientHeading_0049b7cc[36] =
    "VID-COM SYSTEM\n\nSend message to?\n\n";
char g_szEnemyTarget_0049b7f0[16] = "ENEMY TARGET";
char g_szUnidentifiedEnemyTarget_0049b800[16] = "UNIDENTIFIED";
char g_szUnidentifiedFriendlyTarget_0049b810[16] = "UNIDENTIFIED";
char g_szCommCompactHeadingPrefix_0049b820[20] =
    "VID-COM SYSTEM\nTo: ";
char g_szCommHeadingPrefix_0049b834[24] =
    "VID-COM SYSTEM\n\nTo: ";
char g_szUnidentifiedCommRecipient_0049b84c[16] = "UNIDENTIFIED";
char g_szCommHeadingNewline_0049b85c[4] = "\n";
char g_szChooseCommPrompt_0049b860[8] = "CHOOSE";
char g_szCommMenuChoiceFormat_0049b868[8] = "\n%d.%s";
char g_szCommSpeakerTextFormat_0049b888[8] = "%s: %Fs";
short g_bMissionShipInitializationInProgress_0049b890;
short g_bShipResourceReloadInProgress_0049b894;
void **g_apPacketReferenceGroups_0049b898[4];
unsigned char g_ucPendingEjectionTransition_0049b8ac = 0xff;
short g_bEjectionWaitForEnemyWave_0049b8b0;
short g_bEjectionAwaitingCommCommand_0049b8b4;
short g_nPendingEjectionSequenceCount_0049b8b8;
short g_bEjectionTriggerImmediately_0049b8bc;
short g_nEjectionSequenceState_0049b8c0;
signed char g_cQueuedNpcPortrait_0049b8c4 = -1;
signed char g_cQueuedNpcMessage_0049b8c8 = -1;
ShortVector g_aaFormationPositions_0049b8d0[5][8] = {
    {
        { 0, 0, 0 }, { -750, 0, 0 }, { 750, 0, 0 }, { 0, 0, -750 },
        { 0, 0, 750 }, { -750, 0, -750 }, { 750, 0, -750 },
        { 0, 0, -1500 }
    },
    {
        { 0, 0, 0 }, { 750, 0, 0 }, { -750, -100, -250 },
        { 1500, -100, -250 }, { -1500, -200, -500 },
        { -2250, -300, -750 }, { 2250, -200, -500 },
        { 3000, -300, -750 }
    },
    {
        { 0, 0, 0 }, { 750, 0, -500 }, { -750, 0, -500 },
        { 0, 0, -1000 }, { -1500, 0, -1000 }, { -750, 0, -1500 },
        { 1500, 0, -1000 }, { 750, 0, -1500 }
    },
    {
        { 0, 0, 0 }, { 750, 0, -250 }, { 0, 325, -500 },
        { 750, -325, -750 }, { 0, 325, -500 }, { 750, -325, -750 },
        { 0, 0, -1000 }, { 750, 0, -1250 }
    },
    {
        { 0, 0, 0 }, { 0, 0, -750 }, { -750, 0, -500 },
        { -750, 0, -1250 }, { 750, 0, -500 }, { 750, 0, -1250 },
        { 0, 500, -500 }, { 0, 500, -1250 }
    }
};
short g_anPilotTurnInterval_0049b9c0[8] = {
    5, 5, 4, 4, 3, 1, 1, 0
};
unsigned short g_wSoundBlasterPort_0049bb58 = 0x220;
unsigned short g_wSoundBlasterIrq_0049bb5c = 7;
unsigned short g_wDigiPlaying_0049bb60;
unsigned short g_wDmaAddressA_0049bb64;
unsigned short g_wDmaAddressB_0049bb68;
unsigned short g_wDmaCountA_0049bb6c;
unsigned short g_wDmaCountB_0049bb70;
int g_nDmaHalfSelect_0049bb74;
short g_nDigiLastBlock_0049bb78;
unsigned char *g_pNavLocationText_0049bc54;
char g_szBriefingMapTextFormat_0049bd30[12] = "%X%Y%F%s";
int g_bKeyboardMouseEnabled_0049be68;
short g_nMusicTrackComplete_0049be88 = 1;
short g_nMusicDriverMode_0049be8c = 1;
short g_nCutsceneMusicVolume_0049be90;
short g_nTitleMusicSequenceStage_0049be94;
int g_nCurrentMusicTrack_0049be98 = -1;
void *g_pLimitedMusicBufferA_0049bea0;
short g_nLimitedMusicBufferAState_0049bea4 = -1;
void *g_pLimitedMusicBufferB_0049bea6;
short g_nLimitedMusicBufferBState_0049beaa = -1;
short g_nWaitForMusicEnabled_0049beac = 1;
short g_bFlightSoundEffectsEnabled_0049beb0 = 1;
short g_nInitialFlightMusicPending_0049bf00 = 1;
short g_nCombatMusicActive_0049bf04;
short g_nInFlightMusicActive_0049bf08;
short g_nPassingShipSoundObject_0049bf10 = -1;
short g_nPassingShipSoundCountdown_0049bf14;
unsigned char g_abSoundEffectDefinitions_0049bf18[0x1b8] = {
    0, 104, 48, 64, 120, 0, 0, 0,
    2, 55, 48, 96, 2, 0, 0, 0,
    0, 39, 48, 64, 60, 0, 0, 0,
    0, 100, 48, 80, 30, 0, 0, 0,
    0, 7, 48, 127, 6, 0, 0, 0,
    0, 16, 48, 64, 30, 0, 0, 0,
    0, 9, 48, 64, 10, 0, 0, 0,
    0, 10, 48, 127, 15, 0, 0, 0,
    0, 11, 48, 64, 6, 0, 0, 0,
    0, 12, 48, 64, 10, 0, 0, 0,
    4, 13, 48, 64, 60, 0, 0, 0,
    4, 14, 64, 64, 60, 0, 0, 0,
    0, 15, 48, 64, 6, 0, 0, 0,
    8, 19, 48, 64, 60, 0, 0, 0,
    4, 20, 64, 64, 60, 0, 0, 0,
    0, 21, 48, 64, 60, 0, 0, 0,
    4, 22, 48, 64, 60, 0, 0, 0,
    2, 23, 119, 64, 5, 0, 52, 0,
    0, 24, 48, 96, 60, 0, 0, 0,
    2, 41, 24, 24, 2, 0, 100, 0,
    0, 42, 64, 64, 40, 0, 0, 0,
    0, 43, 64, 64, 40, 0, 0, 0,
    0, 61, 48, 64, 7, 0, 0, 0,
    0, 46, 48, 64, 7, 0, 0, 0,
    0, 47, 48, 64, 7, 0, 0, 0,
    0, 60, 48, 64, 20, 0, 0, 0,
    0, 64, 48, 127, 40, 0, 0, 0,
    0, 125, 48, 108, 40, 0, 0, 0,
    0, 54, 48, 96, 30, 0, 0, 0,
    0, 96, 64, 127, 40, 0, 0, 0,
    0, 106, 64, 86, 60, 0, 0, 0,
    8, 107, 48, 64, 40, 0, 0, 0,
    0, 110, 48, 96, 30, 0, 0, 0,
    0, 109, 48, 64, 80, 0, 0, 0,
    0, 111, 48, 64, 30, 0, 0, 0,
    0, 112, 48, 64, 60, 0, 0, 0,
    0, 128, 48, 64, 60, 0, 0, 0,
    4, 32, 64, 64, 60, 0, 0, 0,
    0, 126, 40, 96, 5, 0, 0, 0,
    0, 40, 48, 96, 180, 0, 0, 0,
    0, 44, 64, 96, 90, 0, 0, 0,
    0, 116, 48, 127, 20, 0, 0, 0,
    0, 117, 48, 127, 20, 0, 0, 0,
    0, 31, 64, 96, 200, 0, 0, 0,
    4, 84, 12, 127, 90, 0, 0, 0,
    0, 81, 64, 96, 20, 0, 0, 0,
    0, 18, 64, 84, 30, 0, 0, 0,
    4, 88, 12, 127, 90, 0, 0, 0,
    0, 68, 36, 96, 90, 0, 0, 0,
    0, 35, 36, 96, 60, 0, 0, 0,
    0, 35, 36, 96, 60, 0, 0, 0,
    0, 82, 60, 96, 60, 0, 0, 0,
    4, 77, 48, 96, 200, 0, 0, 0,
    4, 83, 48, 127, 90, 0, 0, 0,
    0, 92, 48, 96, 120
};
char g_szSfxWaveFormat_0049c22c[12] = "sfx%02i.wav";
int g_bDebugBreakEnabled_0049c238;
int g_bApplicationShutdownStarted_0049c23c;
int g_bF1KeyDown_0049c240;
int g_nAudioEnabled_0049c244 = 1;
int g_bInputEventQueueEnabled_0049c248;
unsigned int g_dwGuardedAllocationTotalBytes_0049c24c;
unsigned int g_dwGuardedAllocationBytes_0049c250;
unsigned int g_dwGuardedAllocationPeakBytes_0049c254;
int g_nInputPressCount_0049c258;
int g_bApplicationControllerActive_0049c25c;
int g_bShowFrameRate_0049c260;
int g_bConfigQuickModeEnabled_0049c264 = 1;
int g_bUseHardwarePalette_0049c268 = 1;
int g_bCockpitEnabled_0049c26c = 1;
int g_bSkipCampaignVideo_0049c270;
int g_bWindowInactive_0049c274;
int g_nDebugFileLoggingEnabled_0049c2d8;
FILE *g_pDebugLogFile_0049c2dc;
int g_bConfigMemoryOption_0049c2e0;
int g_bWindowMessagePumpActive_0049c2e4;
int g_nPendingMouseX_0049c2ec;
int g_nPendingMouseY_0049c2f0;
int g_bJoystickPrimaryButtonLatched_0049c2f8;
int g_bJoystickSecondaryButtonLatched_0049c2fc;
GuardedAllocation *g_pGuardedAllocationHead_0049c300;
FreedHeapBlock *g_pFreedHeapBlockHead_0049c304;
FreedHeapBlock *g_pFreedHeapBlockTail_0049c308;
char *g_pEmptyStartupArgumentVector_0049c470;
short g_nCockpitBackgroundBytes_0049c720;
PacketResourceDescriptor g_aCommon3SpaceResources_0049c728[11] = {
    { &g_aObjectTypeData_00496d30[43].shapeSet, 0, 0 },
    { &g_aObjectTypeData_00496d30[36].shapeSet, 0, 1 },
    { &g_aObjectTypeData_00496d30[7].shapeSet, 0, 6 },
    { &g_aObjectTypeData_00496d30[9].shapeSet, 0, 7 },
    { &g_aObjectTypeData_00496d30[8].shapeSet, 0, 8 },
    { &g_aObjectTypeData_00496d30[39].shapeSet, 0, 9 },
    { &g_aObjectTypeData_00496d30[41].shapeSet, 0, 10 },
    { &g_aObjectTypeData_00496d30[40].shapeSet, 0, 11 },
    { &g_aObjectTypeData_00496d30[42].shapeSet, 0, 12 },
    { &g_aObjectTypeData_00496d30[10].shapeSet, 0, 18 },
    { 0, 0, 0 }
};
PacketResourceDescriptor g_aMissionResourceDescriptors_0049c798[8] = {
    { &g_aObjectTypeData_00496d30[21].shapeSet, 0, 15 },
    { &g_aObjectTypeData_00496d30[20].shapeSet, 0, 20 },
    { &g_aObjectTypeData_00496d30[45].shapeSet, 0, 14 },
    { &g_aObjectTypeData_00496d30[37].shapeSet, 0, 2 },
    { &g_aObjectTypeData_00496d30[38].shapeSet, 0, 3 },
    { &g_aObjectTypeData_00496d30[35].shapeSet, 0, 4 },
    { &g_aObjectTypeData_00496d30[31].shapeSet, 0, 5 },
    { 0, 0, 0 }
};
PacketResourceDescriptor g_aHighMemoryCockpitResources_0049c7e8[5] = {
    { &g_pCockpitHudShape_005d21f4, 0, 0 },
    { &g_pPilotHandAnimationShape_005d2c64, "pilotanm.vga", 3 },
    { (unsigned char **)&g_pCockpitExplosionShape_0049b048, 0, 5 },
    { &g_pCockpitIndicatorShape_005d2c48, 0, 4 },
    { 0, 0, 0 }
};
PacketResourceDescriptor g_aCockpitCommonShapeResources_0049c820[5] = {
    { &g_pCockpitPanelShape_005d2cd4, 0, 10 },
    { &g_pCockpitHudShape_005d21f4, "cockpit.vga", 0 },
    { &g_pCockpitWeaponShape_005d2b54, 0, 12 },
    { &g_pCockpitIndicatorShape_005d2c48, "cockpit.vga", 4 },
    { 0, 0, 0 }
};
PacketResourceDescriptor g_aCockpitShipShapeResources_0049c858[8] = {
    { &g_pPendingCockpitDamageShape_0049b03c, 0, 8 },
    { &g_pCockpitPanelShape_005d2cd4, 0, 10 },
    { &g_pCockpitWeaponShape_005d2b54, 0, 12 },
    { &g_apCockpitShipShapes_005d1070[0], 0, 0 },
    { &g_apCockpitShipShapes_005d1070[1], 0, 1 },
    { &g_apCockpitShipShapes_005d1070[2], 0, 2 },
    { &g_apCockpitShipShapes_005d1070[3], 0, 3 },
    { 0, 0, 0 }
};
PacketResourceDescriptor g_aCockpitViewShapeResources_0049c8a8[5] = {
    { &g_apCockpitViewShapes_005d1040[0], 0, 4 },
    { &g_apCockpitViewShapes_005d1040[1], 0, 5 },
    { &g_apCockpitViewShapes_005d1040[2], 0, 6 },
    { &g_apCockpitViewShapes_005d1040[3], 0, 7 },
    { 0, 0, 0 }
};
short g_asConstellationObjectIndices_0049c8e0[4] = {
    -1, -1, -1, -1
};
short g_bCockpitResourcesActive_0049c8e8;
short g_b3SpaceObjectsActive_0049c8ec;
unsigned char *g_pGenericMissileShape_0049c8f0;
unsigned char *g_pGenericMissileExhaustShape_0049c8f4;
unsigned char g_abTaggedAllocationPrefix_0049ca40[8] = {
    'j', 'e', 'f', 'f', 0, 0, 0, 0
};
ShortRect g_stModalBounds_0049ca48 = {0x18, 0x28, 0x128, 0x3c};
ModalTextPanel *g_pModalTextPanel_0049ca50;
unsigned char *g_pCampaignChalkboardShape_0049ca54;
short g_bDisableChalkboardReplay_0049ca58;
short g_nPacketError_0049ca90;
short g_asConstellationVelocity_0049cae0[16] = {
    8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1
};
short g_asConstellationFrame_0049cb00[16] = {
    0, 16, 16, 0, 4, 4, 20, 20, 24, 8, 8, 24, 28, 12, 12, 28
};
short g_nConstellationDirection_0049cb20 = -1;
int g_nDebugOverlayConsoleCount_0049cb24;
DWORD g_dwDebugOverlayKey_0049cb28;
DWORD g_dwDebugOverlayKeyLatch_0049cb2c;
HHOOK g_hDebugKeyboardHook_0049cb30;
char g_szDebugOverlayFontName_0049cb34[8] = "Courier";
char g_szDebugOverlaySpinner_0049cb3c[5] = "-\\|/";
char g_szDebugOverlayNewline_0049cb44[4] = "\n";
short g_bRewritePacketExtensions_0049cb48;
unsigned char g_cSecondaryViewBufferColour_0049cb4c;
unsigned char g_bPrimaryViewBufferColour_0049cb50 = 15;
unsigned char g_abGamePaletteReservedColours_0049cb54[14] = {
    0x25, 0, 0, 0, 0x42, 0, 0, 0, 0x47, 0, 0, 0, 0x5d, 0
};
unsigned char g_ucPrimaryTextColour_0049cb64 = 0x5b;
unsigned char g_ucHostileCapitalScannerColour_0049cb68 = 0x46;
unsigned char g_ucHomeCarrierScannerColour_0049cb70 = 0xa9;
unsigned char g_ucPlayerNavLabelColour_0049cb74 = 0xa5;
unsigned char g_ucNavObjectiveMarkerColour_0049cb78 = 0xb6;
unsigned char g_ucDefaultTextColour_0049cb7c = 0x5c;
unsigned char g_ucAsteroidNavMarkerColour_0049cb84 = 0xf5;
unsigned char g_cPrimaryViewBufferColour_0049cb88 = 0xf8;
unsigned char g_abLegacyVideoModeColours_0049cb90[14] = {
    9, 14, 4, 2, 10, 12, 15, 8, 7, 5, 2, 6, 7, 0
};
char g_szInputCursorPacketName_0049cba0[12] = "arrow.vga";
short g_nExpandedShipMemoryKb_0049cbac;
short g_bOriginFxDriverActive_0049cbb0;
char g_szNotEnoughMemoryMessage_0049cbb8[] =
    "You do not have enough memory to play Wing Commander 2.\n"
    "Refer to your reference guide for assistance.";
int g_nMinimumMemoryNoMusic_0049cc20 = 0x43238;
int g_nMinimumMemoryFullMusic_0049cc24 = 0x49bb0;
int g_nMinimumMemoryAlternate_0049cc28 = 0x43238;
short g_bSpeechResourcesEnabled_0049cc2c;
signed char g_acMusicResourcePresent_0049cc30[66] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0
};
short g_bDirectCampaignSelection_0049cc74;
short g_bSkipCampaignScenes_0049cc78;
short g_bPumpMessagesDuringLoad_0049cc7c;
short g_bMemoryAdjustmentEnabled_0049cc84;
short g_nTargetCameraObject_0049d338 = -1;
unsigned char g_abGunDisplayConfiguration_0049d340[0xa4];
short g_nTargetCameraZoom_0049d3e4;
short g_nTargetCameraFrame_0049d3e8;
short g_nFleetOverviewYawVelocity_0049d3ec;
short g_nFleetOverviewPitchVelocity_0049d3f0;
short g_nFleetOverviewYaw_0049d3f4;
short g_nFleetOverviewPitch_0049d3f8;
int g_nTargetCameraSoundHandle_0049d3fc;
short g_nTargetCameraEffectObjectA_0049d468 = -1;
short g_nTargetCameraEffectObjectB_0049d46c = -1;


signed char g_bSceneEscapeRequested_0049d4b0;
InputEvent *g_pInputEventHead_0049d4b4;
InputEvent *g_pInputEventTail_0049d4b8;
int g_nInputEventPoolInitialized_0049d4bc;
int g_nInputDoubleClickDeadline_0049d4c0;
int g_nPreviousPrimaryButton_0049d4c4;
int g_nPreviousSecondaryButton_0049d4c8;
int g_nNextInputQueueFlushTick_0049d4cc;
unsigned char *g_pDrawnInputCursorShape_0049d4d0;
int g_nMouseCursorDrawDepth_0049d4d4 = 1;
short g_nScreenWidth_0049d4d8 = 320;
short g_nScreenHeight_0049d4dc = 200;
ScreenViewportGeometry g_aScreenViewportGeometry_0049d4e8[6] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 320, 128, 0, 24, 0, 24, -24576, -1 },
    { 320, 200, 0, 0, 0, 0, -1536, -1 }
};
signed char g_acDirectionShapeFrame_0049d558[
    DIRECTION_VIEW_COUNT * DIRECTION_SHAPE_TABLE_COUNT] = {
    0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2,
    14, 13, 12, 11, 10, 9, 8, 9, 10, 11, 12, 13,
    15, 16, 17, 18, 19, 20, 21, 20, 19, 18, 17, 16,
    28, 27, 26, 25, 24, 23, 22, 23, 24, 25, 26, 27,
    29, 30, 31, 32, 33, 34, 35, 34, 33, 32, 31, 30,
    36,
    0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2,
    14, 13, 12, 11, 10, 9, 8, 9, 10, 11, 12, 13,
    15, 16, 17, 18, 19, 20, 21, 20, 19, 18, 17, 16,
    14, 13, 12, 11, 10, 9, 8, 9, 10, 11, 12, 13,
    1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 0,
    0, 1, 2, 3, 1, 3, 2, 1, 2, 3, 1, 3, 2,
    4, 5, 6, 4, 6, 5, 4, 5, 6, 4, 6, 5,
    7, 8, 9, 7, 9, 8, 7, 8, 9, 7, 9, 8,
    10, 11, 12, 10, 12, 11, 10, 11, 12, 10, 12, 11,
    13, 14, 15, 13, 15, 14, 13, 14, 15, 13, 15, 14, 16
};
signed char g_acDirectionShapeFlip_0049d618[
    DIRECTION_VIEW_COUNT * DIRECTION_SHAPE_TABLE_COUNT] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0
};
int g_nNextInputPollTick_0049d6d4;
short g_nInputPollPeriod_0049d6d8;
int g_bFullScreenSpaceView_0049d718 = 1;
int g_nCockpitDisplayMode_0049d71c;
int g_anVolumeLevels_0049d720[11] = {
    0, 500, 1000, 2500, 5000, 11000,
    22500, 34000, 44000, 54000, 64000
};
int g_nSfxVolumeSetting_0049d74c = 0x14;
int g_nMusicVolumeSetting_0049d750 = 0x14;
int g_nTrainSimActive_0049d758;
int g_nArcadeState_0049d75c;
short g_nFrameSkipCountdown_0049d760 = 1;
short g_nFrameSkip_0049d764 = 0;
short g_nSpaceFramePeriod_0049d768 = 4;
short g_bViewportDirty_0049d76c;
short g_nOriginDevUnlock_0049d774;
short g_bCarrierLandingEnabled_0049d778 = 1;
short g_bPlayerDamageEnabled_0049d77c = 1;
short g_bPlayerCollisionEnabled_0049d780 = 1;
short g_nShowMemoryStatus_0049d784;
void *g_pMemoryAdjustment_0049d788;
signed char g_bParentDirectorySelected_0049d78c;
short g_nMissionEntryNavOverride_0049d790 = -1;
short g_bDirectMissionLaunch_0049d798;
short g_nDirectMission_0049d79a;
short g_nDirectSeries_0049d79c = 1;
short g_bSpaceViewBufferEnabled_0049d7a4;
short g_nCockpitControlState_0049d7ac;
InputDeviceSample g_stCurrentFlightInput_0049d7b0;
short g_wFlightInputButtonState_0049d7c0;
int g_bFlightButtonTwoHeld_0049d7c8;
int g_bFlightButtonOneHeld_0049d7cc;
short g_nCockpitControlGoal_0049d7d0;
short g_asMouseYawThresholds_0049d7d8[6] = {
    10, 37, 52, 57, 62, 1070
};
short g_asMousePitchThresholds_0049d7e8[6] = {
    5, 18, 27, 35, 38, 1040
};
signed char g_bSecondaryMouseButtonHeld_0049d7f4;
signed char g_bMouseMoveEventQueued_0049d7fc;
short g_nMouseYawInput_0049d800;
short g_nMousePitchInput_0049d804;
short g_aSpaceExplosionFlashPalette_0049d808[13][3] = {
    {63, 63, 35},
    {63, 63, 35},
    {63, 63, 63},
    {63, 63, 63},
    {63, 63, 63},
    {58, 58, 58},
    {50, 50, 50},
    {42, 42, 42},
    {33, 33, 33},
    {25, 25, 25},
    {17, 17, 17},
    {8, 8, 8},
    {0, 0, 0}
};
int g_bRenderingSpaceFrame_0049d858;
char g_szSpaceBuffer_0049d978[16] = "SPACE BUFFER";
/* Set while a mission is ending; ejection_sequence and death_sequence both
 * return immediately when it is clear. */
short g_bMissionEndPending_0049da4c = 1;
short g_bDeathSequenceActive_0049da50;
DebugOverlayConsole *g_pDebugOverlay_004a2548;
char g_szSystemDebugBuffer_004a2550[256];
WaveTableEntry *g_pSpeechWave_004a2650;
WaveTableEntry *g_pSnowStaticWave_004a2654;
IxSound *g_pSpeechSound_004a2658;
int g_nSpeechCompletionDelay_004a265c;
int g_bSpeechSoundActive_004a2660;
IxSound *g_pSnowStaticSound_004a2664;
int g_nNextSfx16PlaybackFrame_004a2668;
int g_bSpeechPlaybackComplete_004a266c;
RasterSurface g_stRasterSurface_004a2670;
unsigned char g_abShapeTransformScratch_004a2688[0xfa00];
RasterClip g_stRasterClip_004b2088;
unsigned char g_abSolidColourTranslation_004b2710[256];
unsigned char g_abShapeRLEScratch_004b2810[0x100000];
unsigned char *g_pMissionCampaignData_005988bc;
PilotRecord g_aPilotRecords_005988d0[9];
ConstellationObjectDefinition *g_pConstellationDefinitions_00598a28;
PilotRecord *g_apWingmanPilots_00598a30[8];
unsigned char *g_apRecRoomTextData_00598aa0[3];
unsigned char *g_apRecRoomSceneData_00598ae0[3];
unsigned char *g_pBriefingPacket_00598aec;
unsigned char *g_pBriefingTextData_00598af0;
short *g_pMouthAnimationCommands_00598af4;
unsigned char *g_pDebriefingSceneData_00598afc;
unsigned char *g_pBriefingSceneData_00598c00;
unsigned char *g_pConversationBackdropShape_00598c04;
short g_nConversationMedalIndex_00598c08;
short g_nConversationSceneType_00598c0a;
unsigned char *g_pTalkingHeadShape_00598c0c;
short g_nConversationTextColour_00598c10;
unsigned char *g_pBriefingAnimationShape_00598c14;
short *g_pFaceAnimationCommands_00598c18;
unsigned char *g_pBriefingBodyShape_00598c1c;
unsigned char *g_pBriefingPortraitShape_00598c24;
unsigned char *g_pDebriefingTextData_00598c28;
unsigned char *g_pBriefingCloseupShape_00598c2c;
unsigned char *g_pConversationOverlayShape_00598c30;
unsigned char g_bInputMode_0059a848;
short DAT_0059a856;
short DAT_0059a9f0;
MouseCursorState g_stMouseCursorState_0059ab10;
void (*DAT_0059ab2c)(void);
short DAT_0059ab34;
short DAT_0059ab64;
FixedVector g_aObjectViewPosition_0059afa0[SPACE_OBJECT_COUNT];
CampaignState g_stCampaignState_0059ca50;
Viewport g_stConstellationViewport_005a6b40;
Viewport g_stTrainSimTitleDisplayViewport_005a7b90;
Viewport g_stTrainSimHighScoreBufferViewport_005a7bb0;
TextContext g_stTrainSimTextContext_005a7bd0;
Viewport g_stTrainSimHighScoreDisplayViewport_005a7bf0;
TextContext g_stTrainSimHighScoreTextContext_005a7c10;
HighScoreEntry g_aHighScoreEntries_005a7c30[6];
unsigned char *g_pSceneAnimationPrimaryShape_005a7c58;
SceneAnimationObject *g_pSceneAnimationObjects_005a7c64;
short g_nSceneAnimationWaitFrames_005a7c68;
unsigned char *g_pSceneAnimationDefinitions_005a7c6c;
unsigned char *g_pSceneAnimationSecondaryShape_005a7c70;
signed char g_cCockpitLogicalFile_005a7c74;
short g_nWingmanKillCount_005a7cb8;
unsigned int g_dwOriginalFreeMemory_005a7cd8;
unsigned char *g_pMouseCursorResource_005a7cdc;
DiskFileRecord *g_pDiskFileRecords_005a7cf0;
Viewport g_stDiskPromptBackgroundViewport_005a7d00;
unsigned char g_abDiskPromptDriveState_005a7d20[2];
Viewport g_stDiskPromptViewport_005a7d40;
TextContext g_stDiskPromptTextContext_005a7d60;
unsigned int g_dwDiskPromptTopLeft_005a7d80;
unsigned int g_dwDiskPromptBottomRight_005a7d84;
short g_nMissionMedalScore_005a8116;
short g_nCampaignDataSet_005a8118;
short g_nMenuInputRepeatDelay_005a8208;
short DAT_005a86a6;
CampaignDate *g_pCurrentCampaignDate_005a86a8;
short g_nFuneralCasketX_005a86c8;
short g_nFuneralCasketY_005a86ca;
unsigned char *g_pConversationSpecialShape_005a86ec;
ShortPoint g_aFuneralParticles_005a86f0[7];
short g_nFuneralParticleDistance_005a8710;
short g_nFuneralForegroundX_005a8718;
short g_nFuneralForegroundY_005a871a;
short g_nFuneralMainScale_005a871c;
short g_nFuneralRifleFrame_005a871e;
short g_nFuneralBaseX_005a8720;
short g_nFuneralBaseY_005a8722;
short g_nFuneralParticleScale_005a8728;
short g_nFuneralMainDistance_005a8738;
short g_nFuneralGuardFrame_005a873c;
short g_nTalkingHeadFaceX_005a8754;
short g_nTalkingHeadFaceY_005a8756;
short g_nTalkingHeadMouthY_005a8758;
short g_nTalkingHeadMouthX_005a875a;
unsigned char *g_pIntroFont_005a8960;
int g_nStreamerAudioPlaying_005b2810;
#ifdef SDL_PORT
char g_szStreamerPath_005b2818[4096];
#else
char g_szStreamerPath_005b2818[128];
#endif
char g_szLegacyGameDataPath_005b2898[0x400];
int g_bMonoDebugInstalled_005b30e8;
HANDLE g_hMonoDebugDevice_005b30ec;
short DAT_005b30f0;
unsigned char g_bCurrentManeuverReroll_005b30f4;
int g_nWipeRadialStep_005b30f8;
int g_nWipeRadialThreshold_005b30fc;
int g_nWipeRadialAccumulator_005b3100;
short g_asWipeRadialLeadingEdges_005b3108[200];
short g_asWipeRadialTrailingEdges_005b3298[200];
short g_nWipeShortDimension_005b3428;
signed char g_bWipeWidthAtLeastHeight_005b342c;
signed char g_bWipeWidthLessThanHeight_005b3430;
short g_nWipeProgress_005b3434;
short *g_pWipeLeadingEdges_005b3438;
short *g_pWipeTrailingEdges_005b343c;
short g_nWipeTotalSteps_005b3440;
int g_nWipeType_005b3444;
short g_nWipeWidth_005b3448;
short g_nWipeHeight_005b344c;
Viewport g_stWipeDestinationViewport_005b3450;
short g_nWipeCurrentHorizontalEdge_005b3464;
short g_nWipeCurrentVerticalEdge_005b3468;
short g_nWipePreviousHorizontalEdge_005b346c;
short g_nWipePreviousVerticalEdge_005b3470;
int g_nWipeHorizontalStep_005b3478;
int g_nWipeHorizontalThreshold_005b347c;
int g_nWipeHorizontalAccumulator_005b3480;
Viewport g_stWipeSourceViewport_005b3488;
Viewport g_stWipeWorkingDestinationViewport_005b34a0;
short g_nWipePassFlags_005b34b4;
int g_nWipeVerticalStep_005b34b8;
int g_nWipeVerticalThreshold_005b34bc;
int g_nWipeVerticalAccumulator_005b34c0;
short g_nWipeMaximumStep_005b34c4;
Viewport g_stWipeWorkingSourceViewport_005b34c8;
NavMapLabel g_aNavMapLabels_005b34e0[20];
ShortRect g_aNavMapExclusionRects_005b35a8[20];
/* The button mask KeyboardMousePump last posted. */
int g_nKeyboardMouseButtons_005b3648;
char g_szSfxWavePath_005b3650[0x40];
int g_nInsertKeyState_005b3690;
int g_nClearedKeyboardState_005b3694;
int g_nOemPeriodKeyState_005b3718;
int g_nHomeKeyState_005b371c;
int g_nPageDownKeyState_005b3720;
int g_nOemCommaKeyState_005b3724;
int g_nRightArrowKeyState_005b3728;
int g_nEndKeyState_005b372c;
int g_nClearKeyState_005b3730;
int g_nDeleteKeyState_005b3734;
int g_nUpArrowKeyState_005b3738;
int g_nDownArrowKeyState_005b373c;
#ifdef SDL_PORT
char g_szStreamsPath_005b3740[4096];
#else
char g_szStreamsPath_005b3740[0x100];
#endif
int g_nPageUpKeyState_005b3840;
int g_nLeftArrowKeyState_005b3844;
char g_szReadDataFileError_005b3848[0x40];
char g_szCreateDataFileError_005b3888[0x40];
char g_szWriteDataFileError_005b38c8[0x40];
char g_szSeekDataFileError_005b3908[0x80];
char g_szSpaceFlightMaxFpsMessage_005c3970[0x80];
InputEventState g_stFoundInputEvent_005c3af8;
unsigned char g_abInputCursorBackground_005c3b10[0x400];
int g_bMouseFireButtonLatched_005c4b10;
short g_wSavedInputDeviceButtonState_005c4b14;
extern unsigned int g_dwStreamerState_005c4c38;
InputDeviceSample g_stPreviousFlightInput_005c57d0;
short g_bDiscreteFlightInputQueued_005c57e4;
short g_nResourcePaletteMode_005c57e6;
short g_nMenuTextColour_005c57e8;
char *g_pStartupArguments_005c57f0[30];
int g_bSpaceFlightActive_005c586c;
short g_nCurrentSeries_005c5870;
short g_nSpaceExplosionFlashStep_005c5872;
short g_nMenuHighlightColour_005c5874;
short g_nMenuShadowColour_005c5876;
short g_nCurrentMission_005c5878;
short g_bPolledFlightInputQueued_005c587a;
InputEvent g_aInputEventPool_005c5890[0x100];
int g_bPlayerInputActive_005c8090;
int g_nDrawnInputCursorY_005c80a8;
int g_nDrawnInputCursorX_005c80ac;
unsigned char *DAT_005c80c0;
int g_bInputCursorBackgroundCaptured_005c80c4;
short g_bInputManagerInitialized_005c80ca;
short g_nDetectedGraphicsMode_005c80d2;
short g_wPendingInputButtons_005c80d4;
unsigned char g_nInputRepeatDelay_005c80d6;
short g_nViewCenterX_005c80d8;
short g_nViewCenterY_005c80da;
void *g_pLastPacketAllocation_005c80e0;
short g_bHighMemoryResourcesEnabled_005c80e4;
unsigned char g_bInputCursorEnabled_005c80e6;
unsigned char g_abInputKeyState_005c80f0[0x80];
int g_nCapturedInputCursorY_005c8178;
int g_nCapturedInputCursorX_005c817c;
int g_anObjectDepthPlaced_005c8180[SPACE_OBJECT_COUNT];
short DAT_005c82a4;
signed char g_cScreenViewportMode_005c82a6;
signed char g_bCinematicSpriteFontEnabled_005c82a7;
const ScreenViewportGeometry * volatile
    g_pScreenViewportGeometry_005c82b0;
int g_anSortedObject_005c82c0[SPACE_OBJECT_COUNT];
unsigned char *g_pActiveCutscenePixels_005c83dc;
signed char g_cKeypadMouseSpeed_005c83e6;
short g_nQueuedInputX_005c83f0;
short g_nQueuedInputY_005c83f2;
unsigned char g_bQueuedPrimaryButton_005c83f4;
unsigned char g_bQueuedSecondaryButton_005c83f5;
unsigned short g_wQueuedInputFlags_005c83f7;
unsigned char *g_pInputCursorShape_005c83f9;
short g_nInputCursorFrame_005c83fd;
Viewport *g_pInputViewport_005c8403;
unsigned char g_bInputCursorShapeChanged_005c840b;
void (*g_pfnInputPump_005c840c)(void);
short g_nEmsIsrBusy_005c841c;
signed char DAT_005c8430;
short g_wInputDeviceButtonState_005c8432;
int g_bSuppressNextMouseMove_005c843c;
short g_nInputTickInterval_005c8448;
volatile int g_nFrameTimerPending_005c844c;
int g_bInputCursorDirty_005c8450;
short DAT_005c8458;
int g_nInputCursorDirtyRight_005c845c;
int g_nInputCursorDirtyBottom_005c8460;
InputManagerState *g_pInputManagerState_005c8464;
short g_nPersonnelCursorX_005c8470;
short g_nPersonnelCursorY_005c8472;
short g_nUiCursorFrame_005c8481;
void *g_pApplicationScratchBuffer_005c8483;
InputPumpContext *g_pActiveInputContext_005c8487;
short g_nViewportOriginY_005c849c;
short g_nViewportOriginX_005c849e;
volatile int g_nInputClock_005c84a8;
int g_nInputCursorDirtyLeft_005c85c0;
int g_nInputCursorDirtyTop_005c85c4;
int g_anInputEventSlotUsed_005c87e0[0x100];
void (*g_pfnEventManagerShutdown_005c8be0)(void);
void (*g_pfnDrawCinematicTextString_005c8cf4)(char *text);
short g_nPersonnelMouseX_005c8d00;
short g_nPersonnelMouseY_005c8d02;
unsigned char g_bPersonnelPrimaryButton_005c8d04;
unsigned char g_bPersonnelSecondaryButton_005c8d05;
unsigned short g_wPersonnelInputFlags_005c8d07;
TextContext *g_pCurrentTextContext_005c8d1c;
int g_nInputTickScale_005c8d24;
void (*g_pfnDrawCinematicFontCharacter_005c8d30)(char character);
short g_nUiInputMode_005c8d3c;
short g_nTargetCameraYawIndicatorX_005c8d3e;
short g_nTargetCameraYawIndicatorY_005c8d40;
short g_asTargetCameraYawIndicatorBackground_005c8d48[3];
short g_nGunDisplayEnergyPercent_005c8d4e;
int g_nTargetCameraMode_005c8d50;
FixedVector g_vTargetCameraForce_005c8d60;
short g_nTargetCameraPitchIndicatorX_005c8d6c;
short g_nTargetCameraPitchIndicatorY_005c8d6e;
short g_aasGunCooldown_005c8d70[10][3];
short g_asTargetCameraPitchIndicatorBackground_005c8db0[3];
int g_nTargetCameraOverlayMode_005c8db8;
short g_nGunDisplayCount_005c8dbc;
short g_bMissionWingmanFlag_005c8dbe;
short g_nGunDisplayIndex_005c8dc0;
int g_nAllocateViewportCalls_005c8dc4;
int g_nMemoryConfiguration_005c8dc8;
int g_nMusicMemoryRequirement_005c8dcc;
unsigned int g_dwInitialFreeMemory_005c8dd0;
short g_nSoundBlasterIrqSetting_005c8dd8;
short g_nMemoryAdjustmentKb_005c8dda;
short g_bPumpMessagesWhileLoading_005c8ddc;
short g_nSoundBlasterPortSetting_005c8dde;
int g_nAvailableGameMemory_005c8de0;
int g_nMissionResourceBudget_005c8de4;
short g_bSpeechEnabled_005c8de8;
FireworkState g_aFireworks_005c8df0[30];
unsigned char *g_pTitleFireworkShape_005c8f58;
Viewport *g_pConstellationViewport_005c8f5c;
ConstellationParticle g_aConstellationParticles_005c8f60[18];
short g_nConstellationParticleCount_005c9014;
short g_bSceneBackgroundClicked_005c9018;
short g_nCockpitArchiveVariant_005c901a;
void *g_apPacketHandles_005c9020[0x1000];
int g_aiPacketHandleOffsets_005cd020[0x1000];
int g_nPacketHandleCount_005d1020;
char g_szCockpitResourceFilename_005d1030[16];
unsigned char *g_apCockpitViewShapes_005d1040[4];
Viewport g_stCockpitBackgroundViewport_005d1050;
unsigned char *g_apCockpitShipShapes_005d1070[4];
unsigned char *g_pCockpitViewFrameData_005d1088;
short g_nCarrierLaunchInterval_005d108c;
short g_asCarrierLaunchTimer_005d1090[10];
unsigned int g_dwSystemKey_005d10a4;
JOYINFO g_aJoystickInfo_005d10b0[2];
HostMouseMessage g_stHostMouseMessage_005d10d0;
HWND g_hMainWindow_005d10e0;
HANDLE g_hSingleInstanceSemaphore_005d10e4;
unsigned long g_dwGameClockBase_005d10e8;
GuardedAllocation *g_pGuardedAllocationTail_005d10ec;
char g_szGuardedHeapDebugMessage_005d10f0[0x80];
char g_abMemoryUsageReport_005d1170[0x80];
int g_bAltNumpadAddHotkey_005d1270;
int g_nLastAltCommandScanCode_005d1274;
HDC DAT_005d1278;
int g_bAltFHotkey_005d127c;
int g_bAltDHotkey_005d1280;
int g_bJoystickCalibrationHotkey_005d1284;
HCURSOR DAT_005d1288;
int g_bAltHHotkey_005d128c;
int g_bAltBHotkey_005d1290;
int g_bAltAHotkey_005d1294;
int g_bAltTHotkey_005d1298;
unsigned int g_dwGameExitTime_005d129c;
int g_nSavedWindowCursorX_005d12a0;
int g_nSavedWindowCursorY_005d12a4;
int g_bAltNumpadSubtractHotkey_005d12a8;
int g_bMainWindowRunning_005d12ac;
HINSTANCE DAT_005d12b0;
unsigned int g_dwGameStartTime_005d12b4;
long g_lGameClockStart_005d12b8;
int g_aiSoundEffectSourceActive_005d12c0[SPACE_OBJECT_COUNT];
MusicResource g_aMusicResources_005d13e0[66];
unsigned int g_dwRestoredSceneMusicState_005d14e8;
int g_nPassingShipSoundCooldown_005d156c;
short g_nAlternateSampleToggle_005d1570;
int g_anObjectSoundEffects_005d1580[SPACE_OBJECT_COUNT];
short g_nCurrentStarSystem_005d169c;
short g_nNavMapMaximumX_005d169e;
short g_nNavMapMinimumX_005d16a0;
unsigned short g_awNavObjectiveLabelIndex_005d16b0[8];
short g_nNavMapMaximumY_005d16c0;
short g_nNavMapCentreX_005d16c2;
short g_nNavMapMinimumY_005d16c4;
TextContext g_stNavMapTextContext_005d16d0;
short g_nNavMapCentreY_005d16ec;
TextContext g_stNavLabelTextContext_005d16f0;
/* Where UpdateInflightNavText (0x451561), InflightComputer (0x4521e2)
 * and FormatNavCoordinates (0x45164A) point g_pElapsedCampaignDate when
 * it is still null; the readouts take its first short two bytes at a time,
 * and the inflight nav map keeps the system it is showing in the second. */
CampaignDate g_stElapsedCampaignDate_005d170c;
void *g_pDigiData_005d1710;
unsigned short g_wDigiLeft_005d1714;
void *g_pDmaBuffer_005d1718;
unsigned short g_wDigiPosition_005d171c;
void *g_pDigiSample_005d1720;
unsigned char g_ucDmaPage_005d1724;
short g_nActiveInputDevice_005d1726;
int g_nJoystickVerticalDeadZone_005d1728;
int g_nJoystickHorizontalDeadZone_005d172c;
int g_nJoystickUpScale_005d173c;
int g_nJoystickLeftScale_005d1740;
int g_nJoystickMaximumX_005d1744;
int g_nJoystickMaximumY_005d1748;
int g_nJoystickMinimumX_005d174c;
int g_nJoystickMinimumY_005d1750;
int g_nJoystickVerticalRange_005d1754;
int g_nJoystickHorizontalRange_005d1758;
int g_nJoystickRightScale_005d175c;
int g_nJoystickDownScale_005d1760;
int g_nJoystickCentreY_005d1764;
int g_nJoystickCentreX_005d1768;
int g_nJoystickFailureValue_005d176c;
InputDeviceSample g_aInputDeviceSamples_005d1780[2];
short g_nFlightInputCommand_005d1798;
short g_nEnemyCommCommandBase_005d179c;
short g_nEnemyCommPilotIndex_005d179e;
short g_asCapitalHitDamageStep_005d17a0[10];
short g_nWipeRadialRowCount_005d17b4;
char *g_apCommunicationTextPackets_005d17c0[50];
char g_szOnScreenMessageBuffer_005d1890[52];
char g_szCommMessageText_005d18f0[0x58];
signed char g_abCommMenuChoiceCommand_005d1948[7];
const char *g_pszCommMenuHeading_005d1950;
char g_szCommMenuHeadingBuffer_005d1960[0x40];
const char *g_apszCommMenuChoiceText_005d19a0[7];
int g_nViewportAllocationCount_005d19bc;
unsigned char *g_apViewportAllocations_005d19c0[128];
int g_nFreeViewportCalls_005d1bc0;
short g_nPendingEjectionShip_005d1bc4;
short DAT_005d1bd0[16];
short DAT_005d1bf0[16];
short DAT_005d1c10[10];
ReleaseWeaponPosition g_stReleaseWeaponDisplayPosition_005d1c24;
short g_nReleaseWeaponDisplayType_005d1c28;
short g_nTargetLeadIndicatorY_005d1c2a;
unsigned char *g_pScannerMarkerBackground_005d1c2c;
signed char g_cPilotHandFrame_005d1c30;
short g_nHudMessageTime_005d1c32;
short g_nPendingCockpitDamage_005d1c34;
char g_szTextScratchBuffer_005d1c40[256];
HudMessageSlot g_aHudMessageSlots_005d1d40[2];
short g_asScannerTargetBackground_005d1d80[5];
signed char g_cPreviousTargetObject_005d1d8e;
short g_nCommPortraitAnimationFrame_005d1d94;
char g_szComponentHitMessage_005d1da0[40];
signed char g_abShipArrivalMessageSent_005d1dd0[16];
ShortPoint g_aWeaponDisplayPositions_005d1de0[16];
short g_nTargetLockMarkerY_005d1e20;
short g_nHomeMissionShipIndex_005d1e22;
CockpitReadout g_aCockpitReadouts_005d1e30[6];
short g_nScannerCursorX_005d1e6c;
short g_nScannerCursorY_005d1e6e;
signed char g_abCockpitLightState_005d1e70[7];
short g_nTargetLockMusicCooldown_005d1e78;
short g_asScannerObjectY_005d1e80[11];
volatile ShortPoint g_stCockpitExplosionPosition_005d1e98;
short g_asScannerObjectX_005d1ea0[11];
signed char g_abCockpitLightGoal_005d1eb8[7];
int g_nCriticalDamageWarningSfxHandle_005d1ec0;
short g_asScannerBackgroundColour_005d1ed0[10];
short g_asCockpitDamageState_005d1ee8[4];
unsigned short g_nHudMessageColour_005d1ef0;
char *g_apszPersonnelTransferLabels_005d2100[8];
short g_nSceneHotspotLeft_005d2120;
short g_nSceneHotspotTop_005d2122;
short g_nSceneHotspotRight_005d2124;
short g_nSceneHotspotBottom_005d2126;
void *g_pStartupStarPacket_005d212c;
char g_szLegacySavePath_005d2130[0x30];
Viewport g_stCockpitViewport_005d2160;
Viewport g_stLeftVduViewport_005d2180;
Viewport g_stScreenViewport_005d21a0;
TextContext g_stSpaceTextContext_005d21c0;
Viewport g_stCockpitBarViewport_005d21e0;
unsigned char *g_pCockpitHudShape_005d21f4;
unsigned char *g_apTextFonts_005d2200[4];
FixedVector g_aDirectionViewRightVector_005d2210[DIRECTION_VIEW_COUNT];
FixedVector g_aDirectionViewUpVector_005d2500[DIRECTION_VIEW_COUNT];
FixedVector g_aDirectionViewForwardVector_005d27f0[DIRECTION_VIEW_COUNT];
short g_bHighMemoryBuffersReady_005d2ad8;
TextContext g_stLeftVduTextContext_005d2ae0;
Viewport g_stViewBuffer_005d2b00;
Viewport g_stRightVduViewport_005d2b20;
Viewport g_stPilotHandBackgroundViewport_005d2b40;
unsigned char *g_pCockpitWeaponShape_005d2b54;
Viewport g_stConversationTextViewport_005d2b60;
char g_szDefaultTextBuffer_005d2b80[0xc8];
unsigned char *g_pCockpitIndicatorShape_005d2c48;
unsigned char *g_pConstellationShape_005d2c4c;
Viewport g_stModalSourceViewport_005d2c50;
unsigned char *g_pPilotHandAnimationShape_005d2c64;
Viewport g_stPilotHandViewport_005d2c70;
Viewport g_stSecondaryViewBuffer_005d2c90;
short g_aasCockpitHitPaletteFades_005d2cb0[6][3];
unsigned char *g_pCockpitPanelShape_005d2cd4;
TextContext g_stRightVduTextContext_005d2ce0;
TextContext g_stCockpitTextContext_005d2d00;
TextContext g_stDefaultTextContext_005d2d20;
TextContext g_stConversationTextContext_005d2d40;
short g_asSpacePaletteFade_005d2d60[3];
signed char g_bCutsceneCockpitLoaded_005d2d66;
short g_nSavedCutsceneResourceOwner_005d2d68;
short g_asCutsceneSpeechChannels_005d2d70[8];
Viewport g_stCutsceneTextViewport_005d2d90;
CutsceneResourceTable *g_pActiveCutsceneFileResources_005d2da4;
Viewport g_stCutsceneTextBackingViewport_005d2db0;
unsigned int g_dwSceneMusicState_005d2dc4;
char *g_pszCutsceneFormattedText_005d2dc8;
char *g_pszCutsceneSourceText_005d2dcc;
short g_asCutsceneSpeechSections_005d2dd0[8];
signed char g_bCutsceneFramePresented_005d2de0;
TextContext g_stCutsceneDebugTextContext_005d2df0;
SceneFlicCacheEntry g_aSceneFlicCache_005d2e10[15];
unsigned int g_nCutsceneInitialAvailableMemory_005d2e88;
short g_nCutsceneBranchOffset_005d2e8c;
unsigned char *g_pCutscenePalettePacket_005d2e98;
short g_nCutsceneResourceScratch_005d2e9c;
short g_nCutsceneTextIndex_005d2ea4;
unsigned int g_nCutsceneInitialLargestBlock_005d2ea8;
Viewport g_stSceneFlicScratchViewport_005d2eb0;
short g_nPreviousGlobalCount_005d2ec4;
int g_nCutsceneMemoryDelta_005d2ec8;
char *g_pszCutsceneWorkBuffer_005d2ecc;
short g_bCutsceneTextAdvance_005d2ed0;
char *g_apszCutsceneSpeechFiles_005d2ee0[8];
short *g_pCutsceneLocalsScratch_005d2f00;
InputEventState *g_pCutsceneInputEvent_005d2f08;
short g_nCutsceneFormatValue_005d2f0c;
char *g_pszCutscenePrintBuffer_005d2f10;
TextContext g_stCutsceneTextContext_005d2f40;
TextContext g_stCutsceneDrawingTextContext_005d2f60;
void *g_apCutsceneSpeechPackets_005d2f80[8];
short g_nCutsceneFontOwner_005d2fa0;
short g_bPlayerDestroyed_005d2fa4;
signed char g_cPlayerKillCount_005d2fa8;
short g_nCollisionCountdown_005d2faa;
signed char g_acMissionShipStatusFlags_005d2fb0[8];
short g_bWingmanKilledThisMission_005d2fb8;
signed char g_acInitialShipKillCount_005d2fc0[8];
short g_nLastFoundShip_005d2fc8;
short *g_pPilotStatus_005d2fcc;
int g_nShapePrepareTicks_005d2fd0;
int g_nShapeClipTicks_005d2fd4;
int g_nShapeDrawTicks_005d2fd8;
int g_nSavedWingmanObjectCounter_005d2fdc;
int g_nSavedPlayerObjectCounter_005d2fe0;
signed char g_acJumpDriveUsedBySystem_005d2fe8[5];
int g_nJoystickCalibrationMinimumY_005d2ff0;
int g_nJoystickCalibrationMinimumX_005d2ff4;
int g_nJoystickRawX_005d2ff8;
int g_nJoystickRawY_005d2ffc;
int g_nJoystickCalibrationMaximumX_005d3000;
int g_nJoystickCalibrationMaximumY_005d3004;
unsigned char *DAT_005d300c;
short *g_pPaletteTransitionDirection_005d3010;
short *g_pPaletteTransitionDelta_005d3014;
short g_nPaletteTransitionMaxDelta_005d3018;
short *g_pPaletteTransitionAccumulator_005d301c;
short g_nPaletteTransitionCountdown_005d3020;
int g_anInputButton2PressTime_005d3028[2];
short g_asInputButton1DoubleClick_005d3030[2];
short g_asInputButton1Changed_005d3034[2];
short g_asInputButton2Changed_005d3038[2];
short g_asPreviousInputButtons_005d303c[2];
int g_anInputButton1PressTime_005d3040[2];
short g_asInputButton2DoubleClick_005d3048[2];
signed char g_bInputPollingGuard_005d304c;
volatile int g_nNearHeapSize_005d3050;
volatile int g_nNearHeapBase_005d3054;
int g_nNearHeapFirstDescriptor_005d3058;
void * volatile g_pNearHeapAllocation_005d305c;
unsigned char g_aShipIntelligenceData_005d3060[5][0x51];
short g_nStarFieldJRotation_005d320c;
short g_nExternalViewDistance_005d320e;
short g_nStarFieldIRotation_005d3210;
short g_nExternalViewTimeout_005d3212;
unsigned short g_ausPaletteWords_005d3220[256][3];
int g_nNextAfterburnerSfxFrame_005d3820;
short g_asWingmanDamageCredit_005d3830[10];
short g_nAdaptiveDifficulty_005d3844;
short g_asShipExplosionStageTimer_005d3850[10];
int g_bAfterburnerSfxActive_005d3864;
signed char g_abExplosionObjectVisited_005d3870[
    SPACE_OBJECT_COUNT];
short g_asPlayerDamageCredit_005d38c0[10];
short g_asMissileProximityFuse_005d38e0[16];
short g_asObjectCreationFrame_005d3900[SPACE_OBJECT_COUNT];
Viewport *g_pDibScreenViewport_005d398c;
Viewport *g_pFullScreenViewport_005d3990;
unsigned short g_awDibScreenRowOffsets_005d39a0[202];
int g_nDibScreenAllocationState_005d3b40;
Viewport *g_pDibScreenViewportMirror_005d3b44;
int *g_pDibScreenAllocationState_005d3b4c;
FixedVector g_vNormalizedToTarget_005d3bd0;
unsigned int g_uFrameTimerEventId_005d3bec;
short g_nSelectedStartingCampaign_005d3bf0;
short g_nSelectedCampaignSlot_005d3bf2;
SceneResourceTable g_stSceneHotspotTable_005d3bf8;
SceneResourceTable g_stSceneTextTable_005d3c00;
unsigned char g_abMissionAuxData_005d3c10[0x28];
unsigned char g_abSeriesAuxData_005d3c40[0x28];
MissionObjectiveSource g_aMissionObjectiveSources_005d3c70[8];
MissionHeader g_stMissionHeader_005d3e70;
CampaignDate *g_pElapsedCampaignDate_005d3e8c;
unsigned char g_abMissionBriefingData_005d3e90[0xde];
short g_nCannedSceneWriteIndex_005d3fa8;
short g_nCannedSceneRecordedFrameCount_005d3faa;
int g_nCannedSceneFileOffset_005d3fac;
void *g_pCannedSceneStateBlock_005d3fb0;
DwordPtr g_dwHighMemoryParagraph_005d3fb4;
short g_asCannedSceneFrameOffsets_005d3fc0[70];
short g_nCannedSceneFrameCount_005d404c;
short g_awPaletteEntryAllocation_005d4050[256];
short g_nDamageDisplayTicks_005d4250;
short g_nWeaponDisplayOriginX_005d4254;
short g_nWeaponDisplayOriginY_005d4256;
char g_szDamageStatusText_005d4260[70];
signed char g_cDamageDisplayComponent_005d42a6;
signed char g_cDamagedComponentCount_005d42a7;
int g_nDisplayedComponentDamage_005d42a8;
signed char g_cDamageDisplayFrame_005d42ac;
short g_nDamageDisplayState_005d42ae;
ShortPoint g_stDamageSpritePosition_005d42b0;
short g_nDamageDisplayPhase_005d42b4;
SceneFlicObject *g_pWatchedCutsceneSpriteA_005d2e90;
SceneFlicObject *g_pWatchedCutsceneSpriteB_005d2e94;
