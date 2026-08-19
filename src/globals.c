/*
 *  Definitions for the globals declared in include/globals.h.
 *
 *  Provisional: the original scatters these across the modules that own them,
 *  so this file has no counterpart in the shipped image and contributes no
 *  code.  It exists so that regrouping functions cannot break the link.
 */
#include "wc1.h"

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
const unsigned char g_abIffBmhdChunkId_00406f74_WC1_UNMAPPED /* no-address */[4] = {
    'B', 'M', 'H', 'D'
};
const unsigned char g_abIffCmapChunkId_00406f78_WC1_UNMAPPED /* no-address */[4] = { 'C', 'M', 'A', 'P' };
const unsigned char g_abIffBodyChunkId_00406f7c_WC1_UNMAPPED /* no-address */[4] = { 'B', 'O', 'D', 'Y' };

TitleMenuRegion g_aBarracksMenuRegions_00463008_WC1_UNMAPPED /* no-address */[21] = {
    { 1, 137, 88, 149, 94 },
    { 1, 110, 88, 136, 94 },
    { 1, 172, 88, 184, 94 },
    { 1, 185, 88, 210, 94 },
    { 1, 133, 98, 146, 107 },
    { 1, 100, 98, 132, 107 },
    { 1, 174, 98, 189, 107 },
    { 1, 190, 98, 220, 107 },
    { 1, 124, 114, 142, 128 },
    { 1, 81, 114, 123, 128 },
    { 1, 178, 114, 197, 128 },
    { 1, 198, 114, 238, 128 },
    { 1, 109, 141, 135, 164 },
    { 1, 50, 141, 108, 164 },
    { 1, 185, 141, 213, 164 },
    { 1, 214, 141, 268, 164 },
    { 1, 288, 39, 311, 85 },
    { 1, 9, 33, 39, 95 },
    { 1, 218, 37, 248, 78 },
    { 1, 86, 44, 181, 78 },
    { -1, 0, 0, 0, 0 }
};
const short g_asEjectionPrimaryFrames_00465550_WC1_UNMAPPED /* no-address */[8] = {
    0, 1, 1, 3, 3, 0, 0, 0
};
const short g_asEjectionSecondaryFrames_00465560_WC1_UNMAPPED /* no-address */[8] = {
    -1, -1, 2, -1, 4, 0, 0, 0
};
const short g_asEjectionViewScript_00465570_WC1_UNMAPPED /* no-address */[12] = {
    3, 11, 14, 70, 3, 10, 14, 80, 3, 4, -1, 0
};
const char *g_pStrandedMessage_00465588_WC1_UNMAPPED /* no-address */ =
    g_szStrandedMessage_0046558c_WC1_UNMAPPED;
const char g_szStrandedMessage_0046558c_WC1_UNMAPPED /* no-address */[72] =
    "\nWith your carrier\ndestroyed, you drift\nendlessly through\n"
    "the void...";
const char g_szViewTemplates_004655d4_WC1_UNMAPPED /* no-address */[16] = "View Templates";
const char g_szStrandedTheEnd_004655e4_WC1_UNMAPPED /* no-address */[12] = "THE END";
const short g_asMouthFramesByPhoneme_004655f0_WC1_UNMAPPED /* no-address */[26] = {
    0, 5, 4, 4, 1, 8, 4, 7, 0, 4, 4, 7, 5,
    4, 2, 5, 6, 4, 4, 4, 3, 4, 6, 4, 4, 4
};
const char g_szConversationIntegerFormat_00465628_WC1_UNMAPPED /* no-address */[4] = "%d";
const char g_szConversationIntegerFormatAlt_0046562c_WC1_UNMAPPED /* no-address */[4] = "%d";
const char g_szCampaignDateFormat_00465630_WC1_UNMAPPED /* no-address */[12] = "%03d.%03d";
const char g_szSavedCampaignDateFormat_0046563c_WC1_UNMAPPED /* no-address */[12] = "%03d.%03d";
const char g_szCampaignTimeFormat_00465648_WC1_UNMAPPED /* no-address */[12] = "%02d:%02d";
const char g_szConversationTextFormat_00465654_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
signed char g_cScrambleLeftWalkerFrame_00465768_WC1_UNMAPPED /* no-address */ = 7;
signed char g_cScrambleRightWalkerFrame_0046576c_WC1_UNMAPPED /* no-address */ = 10;
signed char g_acScrambleWalkerOverlayFrames_00465770_WC1_UNMAPPED /* no-address */[8] = {
    3, 4, 5, 3, 4, 6, 6, 3
};
short g_nScrambleLeftWalkerX_00465778_WC1_UNMAPPED /* no-address */ = 70;
short g_nScrambleRightWalkerX_0046577c_WC1_UNMAPPED /* no-address */ = 170;
short g_nScrambleBackgroundX_00465780_WC1_UNMAPPED /* no-address */ = 0;
const unsigned short g_ausScrambleActorAnimationA_00465788_WC1_UNMAPPED /* no-address */[8] = {
    0, 1, 2, 3, 4, 5, 0x80, 0
};
const unsigned short g_ausScrambleActorAnimationB_00465798_WC1_UNMAPPED /* no-address */[11] = {
    0, 0, 1, 2, 2, 2, 2, 2, 1, 1, 0x80
};
ScrambleAnimationActor g_aScrambleAnimationActors_004657b0_WC1_UNMAPPED /* no-address */[5] = {
    { 0, 0, 0, 0,  2,  0, 0,
      g_ausScrambleActorAnimationA_00465788_WC1_UNMAPPED, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 10,  0, 0,
      g_ausScrambleActorAnimationB_00465798_WC1_UNMAPPED, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 13,  0, 0,
      g_ausScrambleActorAnimationB_00465798_WC1_UNMAPPED, 0, 0, 0, 0 },
    { 0, 0, 0, 0,  0, -1, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0,  1, -1, 0, 0, 0, 0, 0, 0 }
};
const ScrambleShipDetail g_aaScrambleShipDetails_00465828_WC1_UNMAPPED /* no-address */[4][32] = {
    {
        {  2,   23,   5 }, {  3, -112,  16 },
        {  6,  -94, -23 }, {  6,  -26,   5 },
        {  7,  -58, -11 }, {  7,  -98,   5 },
        {  7,  -14,  13 }, {  7,   60,   6 },
        {  7,  149,  24 }, {  9, -112, -24 },
        {  9,  -99,  -9 }, {  9, -116,  18 },
        {  9,  -81,  15 }, {  9,  -41,   9 },
        {  9,  -54,   1 }, {  9,  -12, -14 },
        {  9,   45,  18 }, {  9,   75,  11 },
        {  9,  130,  14 }, {  9,  166,  24 },
        {  0,  -68, -35 }, {  0,  -19,  -9 },
        {  0,   68,  13 }, {  8,  -76, -18 },
        {  8,  -33,  -1 }, {  8,   97,  31 },
        { 10,  -86,  10 }, { 10, -117,  47 },
        { 10,    9,   0 }, {  1, -111,  52 },
        {  1,  -50,  15 }, {  1,   75,   5 }
    },
    {
        {  3,  -88,  20 }, {  6,  -57,  29 },
        {  7, -139,  63 }, {  7,  -72,   5 },
        {  7,  -14,  28 }, {  9,  -91, -34 },
        {  9, -113, -14 }, {  9, -132,  33 },
        {  9,  -95,  34 }, {  9,  -60,  12 },
        {  9, -155,  52 }, {  9,  -66,  51 },
        {  9,  -56,  44 }, {  9,   44,  35 },
        {  9,   57,  25 }, {  9,   59,  -1 },
        {  9,  123,  41 }, {  9,  148,  30 },
        {  0, -135,  -2 }, {  0, -100,  59 },
        {  8,  -40,  11 }, { 10, -157, -19 },
        { 10,  -77,  35 }, {  6,  248,  17 },
        {  7,  300,  41 }, {  9,  172,  27 },
        {  9,  194,  12 }, {  9,  291,  17 },
        {  9,  290,  35 }, {  0,  233,  20 },
        {  8,  206,  17 }, { 10,  187,  42 }
    },
    {
        {  3,  -97,  -6 }, {  3,   89,   9 },
        {  6,  151,  -4 }, {  7,  -91, -39 },
        {  7,  -78, -16 }, {  7,  -56,  41 },
        {  7,  124,   1 }, {  7, -118,   0 },
        {  9, -107, -37 }, {  9, -127,  38 },
        {  9, -102,  21 }, {  9,  -67,  35 },
        {  9,  -54, -14 }, {  9,   12,  34 },
        {  9,   35,  39 }, {  9,   31,  21 },
        {  9,  130,  35 }, {  9,  185,  10 },
        {  0,  -42,  -3 }, {  0,   75,  35 },
        {  0,  177,  42 }, {  8, -126, -39 },
        { 10,  -16,  37 }, { 10,  148,   5 },
        {  1,   -3,  45 }, {  6,  160,  38 },
        {  7,  154,  16 }, {  9,  188,   3 },
        {  9,  150,  37 }, {  9,  214,  31 },
        {  9,  302,  16 }, {  8,  200,  15 }
    },
    {
        {  6,  103,  18 }, {  7, -114,   4 },
        {  7, -142,  39 }, {  7,   48,  23 },
        {  7,   88,  44 }, {  9,  -67, -30 },
        {  9, -134, -24 }, {  9,  -82,  -1 },
        {  9,  -45,  19 }, {  9, -153,  46 },
        {  9,   -8,  48 }, {  9,   11,  24 },
        {  9,   64,  27 }, {  9,  141,  38 },
        {  9,  154,  12 }, {  0, -105, -20 },
        {  0,   18,  14 }, {  0,  134,  20 },
        {  8, -153, -11 }, { 10,  -76,  64 },
        { 10,   33,  18 }, {  1,  -85, -28 },
        {  1, -154,  14 }, {  1,  148,  21 },
        {  3,  198,  23 }, {  6,  260,  14 },
        {  7,  238,  19 }, {  9,  165,  14 },
        {  9,  221,  22 }, {  0,  246,  16 },
        {  8,  221,  33 }, {  1,  282,  22 }
    }
};
const int g_anLandingDamageDetailCounts_00465aa8_WC1_UNMAPPED /* no-address */[4] = {
    0, 8, 16, 24
};
const char *g_apszLandingDamageComments_00465ab8_WC1_UNMAPPED /* no-address */[4] = {
    g_szLandingCleanComment_00465b58_WC1_UNMAPPED,
    g_szLandingLightDamageComment_00465b78_WC1_UNMAPPED,
    g_szLandingHeavyDamageComment_00465ba8_WC1_UNMAPPED,
    g_szLandingSurvivalComment_00465bcc_WC1_UNMAPPED
};
const signed char g_acLandingCanopyFramesHeavy_00465ac8_WC1_UNMAPPED /* no-address */[24] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10,
    10, 10, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0x40
};
const signed char g_acLandingCanopyFramesLight_00465ae0_WC1_UNMAPPED /* no-address */[24] = {
    0, 1, 2, 3, 11, 12, 13, 14, 14, 14, 14, 14,
    14, 14, 13, 12, 11, 3, 2, 1, 0, 0x40, 0, 0
};
const signed char g_acLandingCanopyFramesModerate_00465af8_WC1_UNMAPPED /* no-address */[16] = {
    0, 15, 16, 17, 18, 19, 19, 18,
    17, 16, 15, 0, 0x40, 0, 0, 0
};
const signed char *g_apLandingCanopyFrames_00465b08_WC1_UNMAPPED /* no-address */[4] = {
    g_acLandingCanopyFramesLight_00465ae0_WC1_UNMAPPED,
    g_acLandingCanopyFramesModerate_00465af8_WC1_UNMAPPED,
    g_acLandingCanopyFramesHeavy_00465ac8_WC1_UNMAPPED,
    g_acLandingCanopyFramesHeavy_00465ac8_WC1_UNMAPPED
};
const ShortPoint g_aFuneralParticleOrigins_00465b18_WC1_UNMAPPED /* no-address */[7] = {
    { 234, 83 }, { 248, 85 }, { 260, 80 }, { 273, 78 },
    { 286, 75 }, { 299, 76 }, { 310, 74 }
};
const short g_asFuneralSceneBySeries_00465b36_WC1_UNMAPPED /* no-address */[15] = {
    0, 0, 1, 1, 1, 1, 1, 2, 3, 2, 3, 3, 2, 3, 0
};
int g_bFuneralShowTheEnd_00465b54_WC1_UNMAPPED /* no-address */;
const char g_szLandingCleanComment_00465b58_WC1_UNMAPPED /* no-address */[32] =
    "You got away pretty clean, sir!";
const char g_szLandingLightDamageComment_00465b78_WC1_UNMAPPED /* no-address */[48] =
    "Looks like it got a little hot out there, sir!";
const char g_szLandingHeavyDamageComment_00465ba8_WC1_UNMAPPED /* no-address */[36] =
    "You sure got yourself shot up, sir!";
const char g_szLandingSurvivalComment_00465bcc_WC1_UNMAPPED /* no-address */[44] =
    "Glad to see you made it back alive, sir.";
const char g_szLandingCommentFormat_00465bf8_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
const char g_szTheEnd_00465c04_WC1_UNMAPPED /* no-address */[8] = "THE END";
const char g_szFuneralTextFormat_00465c0c_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
const char g_szFuneralCompanyCommand_00465c18_WC1_UNMAPPED /* no-address */[20] =
    "%X%Y%FCompany...%P";
const char g_szFuneralAttentionCommand_00465c2c_WC1_UNMAPPED /* no-address */[20] =
    "%X%YAtten-SHUN!%P";
const char g_szFuneralPrepareArmsCommand_00465c40_WC1_UNMAPPED /* no-address */[20] =
    "%X%YPrepare arms!%P";
const char g_szFuneralFireCommand_00465c54_WC1_UNMAPPED /* no-address */[16] =
    "%X%Y%FFire!%P";
short g_nDebriefingPersonality_00465c80_WC1_UNMAPPED /* no-address */;
void *g_aapPacketReferences_00465c88_WC1_UNMAPPED /* no-address */[4][0x25];
const ShortVector g_aaFormationPositions_00465ed8_WC1_UNMAPPED /* no-address */[5][8] = {
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
const char g_szErrorLoadingPilotSpeech_00466010_WC1_UNMAPPED /* no-address */[32] =
    "Error loading pilot speech";


/* Child/hardpoint offsets at 0x004682F0. */
const ShortVector g_aChildOffsets_004682f0_WC1_UNMAPPED /* no-address */[56] = {
    { 120, 10, 20 }, { -120, 10, 20 }, { 75, 45, -30 },
    { -75, 45, -30 }, { 0, 50, 10 }, { -100, 10, -40 },
    { -90, 0, 30 }, { -30, -40, -30 }, { 0, 0, 0 },
    { 30, -40, -30 }, { 100, 10, -40 }, { 90, 0, 30 },
    { -140, 10, 30 }, { -100, 10, 0 }, { -75, 0, -40 },
    { -30, 10, -20 }, { 0, 10, 10 }, { 30, 10, -20 },
    { 75, 0, -40 }, { 100, 10, 0 }, { 140, 10, 30 },
    { -120, -10, 0 }, { -100, 10, -20 }, { -90, 0, 40 },
    { -30, 20, -20 }, { 0, 10, -80 }, { 0, 10, 10 },
    { 30, 20, -20 }, { 90, 0, 40 }, { 100, 10, -20 },
    { 120, -10, 0 }, { 0, 10, 10 }, { 0, 0, -60 },
    { 0, 0, 500 }, { -200, 0, 250 }, { 200, 0, 250 },
    { -300, 0, 0 }, { 300, 0, 0 }, { -200, 0, -250 },
    { 200, 0, -250 }, { 0, 0, -500 }, { -130, 40, 20 },
    { -110, 20, 20 }, { -90, 0, 20 }, { 130, 40, 20 },
    { 110, 20, 20 }, { 90, 0, 20 }, { 0, 0, 400 },
    { -50, -20, 350 }, { 50, 20, 350 }, { -150, 20, 0 },
    { 150, -20, 0 }, { -75, -20, -350 }, { 75, 20, -350 },
    { 0, 100, 350 }, { 0, 0, -300 }
};
const char *g_apszSaveCampaignMenuLabels_004693e8_WC1_UNMAPPED /* no-address */[2] = {
    g_szSaveCampaignMenuLabel_00469468_WC1_UNMAPPED,
    g_szSaveCampaignMenuLabelAlt_00469480_WC1_UNMAPPED
};
char *g_apszBarracksMenuLabels_004693f0_WC1_UNMAPPED /* no-address */[20] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    (char *)g_szMissionHangarMenuLabel_00469498_WC1_UNMAPPED,
    (char *)g_szReturnToBarMenuLabel_004694a8_WC1_UNMAPPED,
    (char *)g_szQuitGameMenuLabel_004694bc_WC1_UNMAPPED,
    (char *)g_szViewMedalsMenuLabel_004694d0_WC1_UNMAPPED
};
const short g_asCampaignPilotFiles_00469450_WC1_UNMAPPED /* no-address */[3] = {58, 61, 74};
const short g_asCampaignBriefingFiles_00469458_WC1_UNMAPPED /* no-address */[3] = {10, 62, 73};
const short g_asMissionDataFiles_00469460_WC1_UNMAPPED /* no-address */[3] = {15, 52, 72};
const char g_szSaveCampaignMenuLabel_00469468_WC1_UNMAPPED /* no-address */[24] =
    "Save this campaign  ";
const char g_szSaveCampaignMenuLabelAlt_00469480_WC1_UNMAPPED /* no-address */[24] =
    "Save this campaign  ";
const char g_szMissionHangarMenuLabel_00469498_WC1_UNMAPPED /* no-address */[16] = "Mission Hangar";
const char g_szReturnToBarMenuLabel_004694a8_WC1_UNMAPPED /* no-address */[20] = "Return to the Bar";
const char g_szQuitGameMenuLabel_004694bc_WC1_UNMAPPED /* no-address */[20] = "Quit Wing Commander";
const char g_szViewMedalsMenuLabel_004694d0_WC1_UNMAPPED /* no-address */[20] = "View your medals";
char g_szDiskMarkerFile_00469688_WC1_UNMAPPED /* no-address */[9] = "DISK.000";
short g_nDiskPromptBorderColour_00469694_WC1_UNMAPPED /* no-address */ = 0x50;
short g_nKeyboardPointerStep_004696a4_WC1_UNMAPPED /* no-address */ = 4;
short g_bSlowSceneAnimation_00469998_WC1_UNMAPPED /* no-address */;
short g_asConversationTextColours_004699f0_WC1_UNMAPPED /* no-address */[24] = {
    0x25, 0xb6, 0x9a, 0x50, 0x94, 0x85, 0x27, 0xa6,
    0xfd, 0x47, 0xaa, 0x0b, 0x09, 0x0d, 0x03, 0x04,
    0x0b, 0x0c, 0x01, 0x0a, 0x06, 0x0e, 0x02, 0x07
};
const int g_anBaseMemoryReservationByVideoMode_00469a90_WC1_UNMAPPED /* no-address */[4] = {
    280000, 260000, 280000, 0
};
const int g_anFullMusicMemoryReservationByVideoMode_00469aa0_WC1_UNMAPPED /* no-address */[4] = {
    306000, 286000, 306000, 0
};
const int g_anExpandedMemoryReservationByVideoMode_00469ab0_WC1_UNMAPPED /* no-address */[3] = {
    258000, 238000, 258000
};
PacketResourceDescriptor g_aCockpitResourceDescriptors_00469c48_WC1_UNMAPPED /* no-address */[19] = {
    { &g_pCockpitHudShape_005d21f4, 8, 0 },
    { &g_pPilotHandAnimationShape_005d2c64, 2, 3 },
    { &g_pConfedCommBackground_00469278_WC1_UNMAPPED, 11, 0 },
    { &g_pCommStaticShape_0046927c_WC1_UNMAPPED, 11, 11 },
    { &g_pKilrathiCommBackground_00469280, 11, 9 },
    { (unsigned char **)&g_pCockpitExplosionShape_00469064_WC1_UNMAPPED, 8, 5 },
    { &g_pCockpitIndicatorShape_005d2c48, 8, 4 },
    { &g_pCinematicViewBackdrop_005a7c90, 8, 6 },
    { &g_pRearViewBackdrop_005a7c94, 8, 7 },
    { &g_apCommPortraitShapes_0059e180[0], 11, 1 },
    { &g_apCommPortraitShapes_0059e180[1], 11, 2 },
    { &g_apCommPortraitShapes_0059e180[2], 11, 3 },
    { &g_apCommPortraitShapes_0059e180[3], 11, 4 },
    { &g_apCommPortraitShapes_0059e180[4], 11, 5 },
    { &g_apCommPortraitShapes_0059e180[5], 11, 6 },
    { &g_apCommPortraitShapes_0059e180[6], 11, 7 },
    { &g_apCommPortraitShapes_0059e180[7], 11, 8 },
    { &g_apCommPortraitShapes_0059e180[8], 11, 10 },
    { 0, 0, 0 }
};
unsigned int g_bIntroSceneResourcesActive_00469d48_WC1_UNMAPPED /* no-address */ = 1;
const short g_asSceneAnimationLogicalFiles_00469d60_WC1_UNMAPPED /* no-address */[8] = {
    63, 64, 65, 66, 67, 68, 69, 70
};
short g_bSceneAnimationWaitCommand_00469d70_WC1_UNMAPPED /* no-address */;
const char g_szSceneAnimationTextFormat_00469d74_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
const char g_szMeanwhile_00469d80_WC1_UNMAPPED /* no-address */[16] = "Meanwhile...";
const char g_szEmptyAnswerInput_00469d90_WC1_UNMAPPED /* no-address */[4] = "";
const char g_szAnswerLabel_00469d94_WC1_UNMAPPED /* no-address */[9] = "Answer: ";
Viewport g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED /* no-address */;
const ShortRect g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED /* no-address */ = {
    0x30, 0x1d, 0x110, 0x6d
};
const char g_szTrainSimTitle_00469dc8_WC1_UNMAPPED /* no-address */[24] = "SQUADRON: TRAINSIM";
unsigned char *g_apszBuiltInHighScoreNames_00469de0_WC1_UNMAPPED /* no-address */[6] = {
    &g_aszBuiltInHighScores_00469e38_WC1_UNMAPPED[0],
    &g_aszBuiltInHighScores_00469e38_WC1_UNMAPPED[8],
    &g_aszBuiltInHighScores_00469e38_WC1_UNMAPPED[16],
    &g_aszBuiltInHighScores_00469e38_WC1_UNMAPPED[24],
    &g_aszBuiltInHighScores_00469e38_WC1_UNMAPPED[32],
    &g_aszBuiltInHighScores_00469e38_WC1_UNMAPPED[40]
};
TitleMenuRegion g_aTrainSimMissionRegions_00469df8_WC1_UNMAPPED /* no-address */[5] = {
    { 1, 47, 29, 67, 49 },
    { 1, 47, 89, 67, 109 },
    { 1, 251, 29, 271, 49 },
    { 1, 251, 89, 271, 109 },
    { -1, 0, 0, 0, 0 }
};
const short g_asCarrierLaunchApproachDeltaX_0046a550_WC1_UNMAPPED /* no-address */[24] = {
    -1, -1, -1, -2, -2, -2, -2, -3,
    -3, -3, -3, -3, -2, -2, -2, -2,
    -1, -1, -1, -1, -1, 0, 0, 0
};
const signed char g_acCarrierLaunchApproachFrames_0046a580_WC1_UNMAPPED /* no-address */[24] = {
    36, 32, 32, 32, 32, 32, 32, 32,
    25, 25, 25, 25, 25, 25, 25, 25,
    18, 18, 18, 18, 18, 18, 18, 18
};
const ShortPoint g_aCarrierLaunchFighterPath_0046a598_WC1_UNMAPPED /* no-address */[9] = {
    { -2, 1 }, { -1, 1 }, { -1, 1 },
    { -1, 0 }, { -1, 1 }, { -1, 0 },
    { -1, 1 }, { -1, 0 }, { 0, 0 }
};
const short g_asCarrierLaunchFighterDeltaY_0046a5bc_WC1_UNMAPPED /* no-address */[16] = {
    0, 0, 3, 3, 3, 2, 2, 2,
    2, 2, 1, 1, 1, 1, 1, 1
};
const short g_asCarrierLaunchViewData_0046a5dc_WC1_UNMAPPED /* no-address */[16] = {
    0, 0, 12, 11,
    1, 90, 90, 0, 9, -90, 0, 0, 20, 3, 15, -1
};
const char g_szPressAnyKeyWhenReady_0046a5fc_WC1_UNMAPPED /* no-address */[28] =
    "Press any key when ready";
short g_nScriptedViewObject_0046a8d0_WC1_UNMAPPED /* no-address */ = -1;
int g_bScriptedView_0046a8d4_WC1_UNMAPPED /* no-address */;
const char g_szWingmanTargetNameFormat_0046a998_WC1_UNMAPPED /* no-address */[4] = " %s";
const char g_szAceTargetNameFormat_0046a99c_WC1_UNMAPPED /* no-address */[4] = " %s";
int g_bMusicCommandSuppressed_0046a9fc_WC1_UNMAPPED /* no-address */;
short g_nFuneralSequenceActive_0046aa10_WC1_UNMAPPED /* no-address */;
int g_nMusicStreamSet_0046aa18_WC1_UNMAPPED /* no-address */ = -1;
const unsigned char g_abFireworkSoundDescriptor_0046ab70_WC1_UNMAPPED /* no-address */[7] = {
    0x00, 0x80, 0x40, 0x40, 0x3c, 0x00, 0x00
};
const char *g_apszCampaignVictoryText_0046ad90_WC1_UNMAPPED /* no-address */[4] = {
    g_szCampaignVictoryOpening_0046add0_WC1_UNMAPPED,
    g_szCampaignVictoryAttack_0046ae14_WC1_UNMAPPED,
    g_szCampaignVictoryConclusion_0046ae40_WC1_UNMAPPED,
    0
};
const char *g_pszTigerClawEscapeOpening_0046ada0_WC1_UNMAPPED /* no-address */ =
    g_szTigerClawEscapeOpening_0046ae74_WC1_UNMAPPED;
const char *g_pszTigerClawEscapeJump_0046ada4_WC1_UNMAPPED /* no-address */ =
    g_szTigerClawEscapeJump_0046aeb4_WC1_UNMAPPED;
const char *g_pszTigerClawEscapeClosing_0046ada8_WC1_UNMAPPED /* no-address */ =
    g_szTigerClawEscapeClosing_0046aee4_WC1_UNMAPPED;
const ShortVector g_aCampaignVictoryProjectileOrigins_0046adb0_WC1_UNMAPPED /* no-address */[4] = {
    { -55, 42, 0 }, { -68, 46, 0 },
    { 60, 42, 16 }, { 73, 46, 16 }
};
const char *g_pszTheEnd_0046adc8_WC1_UNMAPPED /* no-address */ = g_szTheEnd_0046af10_WC1_UNMAPPED;
const char *g_pszForNow_0046adcc_WC1_UNMAPPED /* no-address */ = g_szForNow_0046af18_WC1_UNMAPPED;
const char g_szCampaignVictoryOpening_0046add0_WC1_UNMAPPED /* no-address */[68] =
    "Destroying the remains of the Kilrathi naval power in the sector...";
const char g_szCampaignVictoryAttack_0046ae14_WC1_UNMAPPED /* no-address */[44] =
    "The Tiger's Claw closes in for the kill...";
const char g_szCampaignVictoryConclusion_0046ae40_WC1_UNMAPPED /* no-address */[52] =
    "And the last Kilrathi planet in the sector falls!";
const char g_szTigerClawEscapeOpening_0046ae74_WC1_UNMAPPED /* no-address */[64] =
    "Fleeing from the overwelming Kilrathi forces in the sector...";
const char g_szTigerClawEscapeJump_0046aeb4_WC1_UNMAPPED /* no-address */[48] =
    "The Tiger's Claw manages to jump out. Barely.";
const char g_szTigerClawEscapeClosing_0046aee4_WC1_UNMAPPED /* no-address */[44] =
    "There'll be other sectors, other battles...";
const char g_szTheEnd_0046af10_WC1_UNMAPPED /* no-address */[8] = "The End";
const char g_szForNow_0046af18_WC1_UNMAPPED /* no-address */[12] = "For Now...";
const char g_szCampaignVictoryTextFormat_0046af24_WC1_UNMAPPED /* no-address */[12] = "%X%Y%s%P";
const char g_szTigerClawEscapeOpeningFormat_0046af30_WC1_UNMAPPED /* no-address */[12] = "%X%Y%s%P";
const char g_szTigerClawEscapeJumpFormat_0046af3c_WC1_UNMAPPED /* no-address */[12] = "%X%Y%s%P";
const char g_szTigerClawEscapeClosingFormat_0046af48_WC1_UNMAPPED /* no-address */[12] = "%X%Y%s%P";

const char *g_apszKilrathiAceNames_0046af80_WC1_UNMAPPED /* no-address */[4] = {
    &g_aszKilrathiAceNames_0046afd4_WC1_UNMAPPED[0x00],
    &g_aszKilrathiAceNames_0046afd4_WC1_UNMAPPED[0x08],
    &g_aszKilrathiAceNames_0046afd4_WC1_UNMAPPED[0x10],
    &g_aszKilrathiAceNames_0046afd4_WC1_UNMAPPED[0x18]
};

const char g_aszKilrathiAceNames_0046afd4_WC1_UNMAPPED /* no-address */[32] =
    "Bhurak\0\0"
    "Dakhath\0"
    "Khajja\0\0"
    "Bakhtosh";
short g_nEjectedPilotObject_0046c044_WC1_UNMAPPED /* no-address */;
const short g_asIntroCameraSequence_0046c090_WC1_UNMAPPED /* no-address */[20] = {
    0, -1000, 0, -4263, 2, 15, 1, 0, 30, 0,
    3, 15, 4, 30, 1, 13, 14, 400, -1, 0
};
const short g_asCampaignVictoryViewScript_0046c160_WC1_UNMAPPED /* no-address */[24] = {
    16, 38, 0, 1200, 0, 1600, 1, 180,
    0, 0, 15, 3, 4, 14, 100, 2,
    15, 3, 15, 14, 120, -1, 0, 0
};
const short g_asTigerClawEscapeViewScript_0046c238_WC1_UNMAPPED /* no-address */[12] = {
    0, 0, 0, 0, 2, 15, 3, 15, 14, 400, -1, 0
};
ManeuverChoice g_aRatedManeuverChoices_0046d3e8_WC1_UNMAPPED /* no-address */[13][9][3] = {
    {
        { { 70, 40, 30 }, { 80, 40, 30 }, { 100, 21, -1 } },
        { { 80, 29, 37 }, { 80, 29, 37 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 21, -1 } },
        { { 50, 7, 8 }, { 75, 14, 17 }, { 100, 21, -1 } },
        { { 80, 41, 29 }, { 80, 41, 24 }, { 100, 21, -1 } },
        { { 60, 26, 29 }, { 70, 26, 29 }, { 90, 26, 29 } },
        { { 50, 16, 12 }, { 50, 13, 20 }, { 100, 21, -1 } },
        { { 100, 19, -1 }, { 50, 9, 20 }, { 100, 21, -1 } },
        { { 100, 2, -1 }, { 100, 2, -1 }, { 100, 2, -1 } }
    },
    {
        { { 40, 40, 30 }, { 80, 40, 30 }, { 100, 21, -1 } },
        { { 80, 29, 37 }, { 50, 31, 40 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 21, -1 } },
        { { 55, 12, 19 }, { 70, 11, 12 }, { 100, 21, -1 } },
        { { 60, 29, 41 }, { 80, 29, 41 }, { 100, 33, -1 } },
        { { 80, 26, 29 }, { 50, 26, 29 }, { 100, 29, -1 } },
        { { 50, 15, 16 }, { 50, 16, 32 }, { 100, 21, -1 } },
        { { 50, 7, 10 }, { 50, 14, 9 }, { 100, 21, -1 } },
        { { 50, 25, 2 }, { 50, 25, 2 }, { 100, 2, -1 } }
    },
    {
        { { 40, 40, 30 }, { 60, 40, 30 }, { 100, 21, -1 } },
        { { 80, 29, 37 }, { 50, 29, 40 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 21, -1 } },
        { { 60, 15, 17 }, { 50, 12, 17 }, { 100, 21, -1 } },
        { { 20, 41, 30 }, { 30, 41, 17 }, { 40, 41, 21 } },
        { { 100, 26, -1 }, { 100, 26, -1 }, { 100, 26, -1 } },
        { { 40, 32, 17 }, { 20, 32, 16 }, { 100, 21, -1 } },
        { { 50, 17, 20 }, { 50, 23, 36 }, { 100, 20, -1 } },
        { { 100, 2, -1 }, { 100, 2, -1 }, { 100, 21, -1 } }
    },
    {
        { { 50, 40, 30 }, { 90, 40, 30 }, { 100, 21, -1 } },
        { { 80, 29, 37 }, { 80, 30, 37 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 44, -1 }, { 100, 44, -1 } },
        { { 70, 7, 16 }, { 100, 44, -1 }, { 100, 44, -1 } },
        { { 20, 31, 41 }, { 100, 44, -1 }, { 100, 44, -1 } },
        { { 80, 26, 31 }, { 100, 44, -1 }, { 100, 44, -1 } },
        { { 75, 16, 15 }, { 75, 16, 15 }, { 100, 44, -1 } },
        { { 50, 12, 16 }, { 100, 44, -1 }, { 100, 44, -1 } },
        { { 100, 2, -1 }, { 100, 2, -1 }, { 100, 2, -1 } }
    },
    {
        { { 60, 40, 30 }, { 100, 40, -1 }, { 100, 21, -1 } },
        { { 80, 29, 37 }, { 50, 29, 40 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 39, -1 } },
        { { 50, 15, 16 }, { 60, 15, 7 }, { 70, 20, 16 } },
        { { 70, 41, 17 }, { 80, 41, 20 }, { 100, 20, -1 } },
        { { 100, 26, -1 }, { 100, 26, -1 }, { 100, 26, -1 } },
        { { 50, 16, 13 }, { 50, 16, 20 }, { 100, 21, -1 } },
        { { 50, 20, 36 }, { 50, 11, 10 }, { 100, 20, -1 } },
        { { 25, 25, 2 }, { 100, 2, -1 }, { 100, 2, -1 } }
    },
    {
        { { 40, 40, 30 }, { 80, 40, 30 }, { 100, 21, -1 } },
        { { 80, 29, 37 }, { 20, 29, 40 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 39, -1 } },
        { { 50, 7, 8 }, { 50, 9, 24 }, { 100, 12, -1 } },
        { { 80, 41, 29 }, { 80, 41, 20 }, { 100, 41, -1 } },
        { { 100, 26, -1 }, { 100, 26, -1 }, { 100, 26, -1 } },
        { { 20, 32, 16 }, { 50, 16, 20 }, { 100, 21, -1 } },
        { { 50, 19, 14 }, { 50, 14, 12 }, { 100, 8, -1 } },
        { { 50, 17, 23 }, { 50, 17, 23 }, { 30, 25, 2 } }
    },
    {
        { { 50, 30, 31 }, { 60, 40, 31 }, { 100, 39, -1 } },
        { { 80, 29, 37 }, { 80, 29, 37 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 39, -1 } },
        { { 50, 9, 10 }, { 60, 7, 11 }, { 100, 20, -1 } },
        { { 50, 31, 41 }, { 80, 31, 23 }, { 100, 33, -1 } },
        { { 80, 26, 29 }, { 70, 26, 29 }, { 100, 26, -1 } },
        { { 50, 32, 16 }, { 50, 20, 16 }, { 100, 21, -1 } },
        { { 50, 36, 12 }, { 50, 20, 8 }, { 50, 19, 20 } },
        { { 80, 25, 2 }, { 50, 25, 17 }, { 100, 2, -1 } }
    },
    {
        { { 50, 40, 30 }, { 80, 40, 30 }, { 100, 21, -1 } },
        { { 80, 29, 37 }, { 50, 29, 40 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 21, -1 } },
        { { 60, 15, 8 }, { 70, 19, 14 }, { 100, 21, -1 } },
        { { 70, 29, 20 }, { 50, 29, 20 }, { 100, 41, -1 } },
        { { 100, 26, -1 }, { 100, 26, -1 }, { 100, 26, -1 } },
        { { 50, 16, 19 }, { 50, 16, 8 }, { 100, 21, -1 } },
        { { 50, 36, 12 }, { 50, 19, 20 }, { 100, 21, -1 } },
        { { 100, 2, -1 }, { 100, 2, -1 }, { 100, 2, -1 } }
    },
    {
        { { 100, -1, -1 }, { 100, -1, -1 }, { 100, -1, -1 } },
        { { 100, -1, -1 }, { 100, -1, -1 }, { 100, -1, -1 } },
        { { 100, -1, -1 }, { 100, -1, -1 }, { 100, -1, -1 } },
        { { 100, -1, -1 }, { 100, -1, -1 }, { 100, -1, -1 } },
        { { 100, -1, -1 }, { 100, -1, -1 }, { 100, -1, -1 } },
        { { 100, -1, -1 }, { 100, -1, -1 }, { 100, -1, -1 } },
        { { 100, -1, -1 }, { 100, -1, -1 }, { 100, -1, -1 } },
        { { 0, -1, -1 }, { 0, -1, -1 }, { 0, -1, -1 } },
        { { 0, -1, -1 }, { 0, -1, -1 }, { 0, -1, -1 } }
    },
    {
        { { 60, 40, 29 }, { 60, 40, 29 }, { 100, 21, -1 } },
        { { 80, 29, 37 }, { 50, 29, 40 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 21, -1 } },
        { { 50, 16, 38 }, { 75, 7, 20 }, { 100, 21, -1 } },
        { { 60, 41, 31 }, { 60, 41, 31 }, { 100, 21, -1 } },
        { { 100, 26, -1 }, { 100, 26, -1 }, { 100, 26, -1 } },
        { { 50, 32, 20 }, { 50, 32, 20 }, { 100, 21, -1 } },
        { { 50, 19, 20 }, { 50, 9, 20 }, { 100, 21, -1 } },
        { { 100, 25, -1 }, { 100, 2, -1 }, { 100, 2, -1 } }
    },
    {
        { { 50, 40, 31 }, { 70, 40, 31 }, { 100, 21, -1 } },
        { { 80, 29, 37 }, { 50, 29, 40 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 21, -1 } },
        { { 97, 12, 22 }, { 95, 20, 22 }, { 100, 21, -1 } },
        { { 40, 41, 31 }, { 20, 41, 31 }, { 100, 21, -1 } },
        { { 50, 26, 31 }, { 100, 31, -1 }, { 100, 26, -1 } },
        { { 50, 16, 20 }, { 50, 19, 20 }, { 100, 21, -1 } },
        { { 100, 20, 36 }, { 50, 12, 20 }, { 100, 21, -1 } },
        { { 100, 25, -1 }, { 100, 25, -1 }, { 100, 2, -1 } }
    },
    {
        { { 70, 40, 30 }, { 70, 16, 30 }, { 100, 21, -1 } },
        { { 100, 18, -1 }, { 100, 18, -1 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 21, -1 } },
        { { 50, 23, 20 }, { 75, 12, 17 }, { 100, 21, -1 } },
        { { 30, 41, 29 }, { 50, 41, 20 }, { 100, 21, -1 } },
        { { 100, 26, -1 }, { 100, 26, -1 }, { 100, 26, -1 } },
        { { 20, 32, 20 }, { 50, 16, 20 }, { 100, 21, -1 } },
        { { 50, 16, 19 }, { 50, 12, 20 }, { 100, 21, -1 } },
        { { 100, 25, -1 }, { 100, 2, -1 }, { 100, 2, -1 } }
    },
    {
        { { 80, 40, 31 }, { 90, 40, 31 }, { 100, 21, -1 } },
        { { 100, 18, -1 }, { 100, 18, -1 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 21, -1 } },
        { { 50, 15, 20 }, { 75, 16, 19 }, { 100, 21, -1 } },
        { { 50, 41, 31 }, { 50, 41, 31 }, { 100, 21, -1 } },
        { { 80, 26, 31 }, { 100, 26, -1 }, { 100, 26, -1 } },
        { { 50, 16, 20 }, { 50, 32, 20 }, { 100, 21, -1 } },
        { { 70, 19, 17 }, { 50, 11, 20 }, { 100, 21, -1 } },
        { { 100, 25, -1 }, { 100, 2, -1 }, { 100, 2, -1 } }
    }
};
ManeuverChoice g_aKilrathiManeuverChoices_0046d808_WC1_UNMAPPED /* no-address */[5][9][3] = {
    {
        { { 70, 40, 45 }, { 80, 40, 45 }, { 100, 21, -1 } },
        { { 80, 29, 37 }, { 50, 29, 40 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 21, -1 } },
        { { 90, 17, 46 }, { 90, 17, 46 }, { 100, 21, -1 } },
        { { 50, 46, 45 }, { 80, 46, 45 }, { 100, 21, -1 } },
        { { 100, 45, -1 }, { 100, 45, -1 }, { 100, 45, -1 } },
        { { 100, 46, -1 }, { 20, 17, 46 }, { 100, 21, -1 } },
        { { 100, 46, -1 }, { 25, 17, 46 }, { 100, 21, -1 } },
        { { 100, 2, -1 }, { 100, 2, -1 }, { 100, 2, -1 } }
    },
    {
        { { 40, 40, 45 }, { 70, 40, 45 }, { 100, 21, -1 } },
        { { 80, 29, 37 }, { 50, 29, 30 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 21, -1 } },
        { { 50, 8, 46 }, { 50, 14, 46 }, { 100, 21, -1 } },
        { { 70, 45, 46 }, { 60, 45, 46 }, { 100, 21, -1 } },
        { { 100, 45, -1 }, { 100, 45, -1 }, { 100, 45, -1 } },
        { { 100, 46, -1 }, { 100, 46, -1 }, { 100, 21, -1 } },
        { { 50, 38, 46 }, { 100, 46, -1 }, { 100, 21, -1 } },
        { { 100, 2, -1 }, { 100, 2, -1 }, { 100, 2, -1 } }
    },
    {
        { { 80, 40, 29 }, { 80, 40, 29 }, { 100, 21, -1 } },
        { { 80, 29, 37 }, { 50, 29, 40 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 21, -1 } },
        { { 60, 46, 45 }, { 60, 46, 45 }, { 100, 21, -1 } },
        { { 30, 41, 45 }, { 30, 41, 45 }, { 100, 21, -1 } },
        { { 100, 29, -1 }, { 100, 45, -1 }, { 100, 45, -1 } },
        { { 100, 46, -1 }, { 100, 46, -1 }, { 100, 46, -1 } },
        { { 50, 29, 46 }, { 100, 46, -1 }, { 100, 21, -1 } },
        { { 100, 2, -1 }, { 100, 2, -1 }, { 100, 2, -1 } }
    },
    {
        { { 50, 40, 29 }, { 50, 40, 29 }, { 100, 21, -1 } },
        { { 100, 18, -1 }, { 100, 18, -1 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 39, -1 } },
        { { 30, 12, 29 }, { 98, 46, 22 }, { 100, 21, -1 } },
        { { 90, 29, 31 }, { 100, 29, -1 }, { 100, 21, -1 } },
        { { 100, 45, -1 }, { 100, 45, -1 }, { 100, 45, -1 } },
        { { 100, 46, -1 }, { 100, 46, -1 }, { 100, 46, -1 } },
        { { 50, 36, 46 }, { 100, 46, -1 }, { 100, 21, -1 } },
        { { 100, 2, -1 }, { 100, 2, -1 }, { 100, 2, -1 } }
    },
    {
        { { 40, 40, 29 }, { 40, 40, 29 }, { 100, 21, -1 } },
        { { 100, 18, -1 }, { 100, 18, -1 }, { 100, 21, -1 } },
        { { 100, 39, -1 }, { 100, 39, -1 }, { 100, 39, -1 } },
        { { 90, 46, 36 }, { 90, 46, 12 }, { 100, 21, -1 } },
        { { 100, 29, -1 }, { 100, 29, -1 }, { 100, 21, -1 } },
        { { 100, 45, -1 }, { 100, 45, -1 }, { 100, 45, -1 } },
        { { 100, 46, -1 }, { 100, 46, -1 }, { 100, 21, -1 } },
        { { 10, 11, 46 }, { 100, 46, -1 }, { 100, 46, -1 } },
        { { 100, 2, -1 }, { 100, 2, -1 }, { 100, 2, -1 } }
    }
};
CampaignDate g_stSavedCampaignDate_0046e188_WC1_UNMAPPED /* no-address */ = {20, 340};
const TalkingHeadOrigin g_aTalkingHeadOrigins_0046e190_WC1_UNMAPPED /* no-address */[11] = {
    {161, 60, 161, 90},
    {161, 60, 161, 87},
    {160, 60, 159, 90},
    {161, 60, 161, 90},
    {161, 60, 161, 90},
    {161, 60, 161, 90},
    {161, 60, 161, 90},
    {161, 60, 161, 90},
    {161, 60, 161, 90},
    {161, 60, 161, 90},
    {160, 53, 160, 88},
};
const signed char g_abBriefingSmallCharacterAnimation_0046e1e8_WC1_UNMAPPED /* no-address */[24] = {
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0
};
const signed char g_abBriefingLargeCharacterAnimation_0046e200_WC1_UNMAPPED /* no-address */[24] = {
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0
};
BriefingCharacterLayout g_aBriefingCharacters_0046e218_WC1_UNMAPPED /* no-address */[8] = {
    { 60, 123,  10, 95, 176, 1, 0, 2,
      g_abBriefingSmallCharacterAnimation_0046e1e8_WC1_UNMAPPED, 0},
    {316, 123, 264, 94, 176, 1, 2, 1,
      g_abBriefingSmallCharacterAnimation_0046e1e8_WC1_UNMAPPED, 0},
    {193, 123, 141, 95, 176, 1, 3, 1,
      g_abBriefingSmallCharacterAnimation_0046e1e8_WC1_UNMAPPED, 0},
    {250, 124, 199, 93, 176, 1, 4, 1,
      g_abBriefingSmallCharacterAnimation_0046e1e8_WC1_UNMAPPED, 0},
    {124, 123,  71, 94, 176, 1, 5, 1,
      g_abBriefingSmallCharacterAnimation_0046e1e8_WC1_UNMAPPED, 0},
    {103, 122,  29, 76, 256, 1, 6, 2,
      g_abBriefingLargeCharacterAnimation_0046e200_WC1_UNMAPPED, 0},
    {191, 122, 118, 76, 256, 1, 8, 1,
      g_abBriefingLargeCharacterAnimation_0046e200_WC1_UNMAPPED, 0},
    {287, 122, 212, 76, 256, 1, 9, 1,
      g_abBriefingLargeCharacterAnimation_0046e200_WC1_UNMAPPED, 0}
};
const short g_asMedalDisplayX_0046e2d0_WC1_UNMAPPED /* no-address */[5] = {191, 199, 207, 216, 228};
const char *g_apszMedalNames_0046e2e0_WC1_UNMAPPED /* no-address */[5] = {
    g_szBronzeStar_0046e594_WC1_UNMAPPED,
    g_szSilverStar_0046e5a0_WC1_UNMAPPED,
    g_szGoldStar_0046e5ac_WC1_UNMAPPED,
    g_szGoldenSun_0046e5b8_WC1_UNMAPPED,
    g_szTerranMedalOfValor_0046e5c4_WC1_UNMAPPED
};
unsigned char *g_pMedalSceneShape_0046e2f4_WC1_UNMAPPED /* no-address */;
const char *g_pszMedalsPilotSummary_0046e2f8_WC1_UNMAPPED /* no-address */ =
    g_szMedalsPilotSummary_0046e5dc_WC1_UNMAPPED;
const int g_aiBriefingLeftPanelVelocity_0046e480_WC1_UNMAPPED /* no-address */[12] = {
    1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4
};
const int g_aiBriefingPodiumVelocity_0046e4b0_WC1_UNMAPPED /* no-address */[12] = {
    1, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5, 6
};
const int g_aiBriefingRightPanelVelocity_0046e4e0_WC1_UNMAPPED /* no-address */[12] = {
    2, 2, 3, 4, 4, 4, 5, 5, 5, 6, 7, 8
};
const signed char g_abBriefingPodiumFrames_0046e510_WC1_UNMAPPED /* no-address */[40] = {
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
    10, 11, 12, 13, 14, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 14, 13, 12, 11, 10,  9,
     8,  7,  6,  5,  4,  0,  0,  0,  0,  0
};
const signed char g_abDebriefingEstablishDeltas_0046e538_WC1_UNMAPPED /* no-address */[48] = {
    -2, -2, -1, -1,  0,  0,  1,  0,
     1,  0,  1,  0,  1,  0,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  0,  1,  0,  1,
     0,  1,  0,  1,  0,  0, -1, -1
};
short g_nDebriefingLeftX_0046e56c_WC1_UNMAPPED /* no-address */;
short g_nDebriefingPilotX_0046e570_WC1_UNMAPPED /* no-address */ = 80;
short g_nDebriefingRightX_0046e574_WC1_UNMAPPED /* no-address */ = 278;
short g_nDebriefingOfficerX_0046e578_WC1_UNMAPPED /* no-address */ = 200;
short g_nDebriefingPodiumX_0046e57c_WC1_UNMAPPED /* no-address */ = 344;
short g_nConversationCharacter_0046e580_WC1_UNMAPPED /* no-address */ = -1;
short g_nTalkingHeadFace_0046e584_WC1_UNMAPPED /* no-address */ = -1;
short g_nConversationBackdropFrame_0046e588_WC1_UNMAPPED /* no-address */ = -1;
int g_bConversationConstellation_0046e58c_WC1_UNMAPPED /* no-address */;
int g_bConversationOverlay_0046e590_WC1_UNMAPPED /* no-address */;
const char g_szBronzeStar_0046e594_WC1_UNMAPPED /* no-address */[12] = "Bronze Star";
const char g_szSilverStar_0046e5a0_WC1_UNMAPPED /* no-address */[12] = "Silver Star";
const char g_szGoldStar_0046e5ac_WC1_UNMAPPED /* no-address */[12] = "Gold Star";
const char g_szGoldenSun_0046e5b8_WC1_UNMAPPED /* no-address */[12] = "Golden Sun";
const char g_szTerranMedalOfValor_0046e5c4_WC1_UNMAPPED /* no-address */[24] =
    "Terran Medal of Valor";
const char g_szMedalsPilotSummary_0046e5dc_WC1_UNMAPPED /* no-address */[40] =
    "$R $N, aka $C.\n$S system, dateline $D.";
const char g_szViewMedalsTextFormat_0046e604_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
const char g_szMedalChestTextFormat_0046e610_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
const char g_szMedalLongShotTextFormat_0046e61c_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
const char g_szMedalEstablishTextFormat_0046e628_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
const char g_szPinMedalTextFormat_0046e634_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
const char g_szEstablishingShotTextFormat_0046e640_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
const char g_szBriefingReturnTextFormat_0046e64c_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
const char g_szDismissedTextFormat_0046e658_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
const char g_szDebriefEstablishTextFormat_0046e664_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
const char g_szFuneralLongShotTextFormat_0046e670_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
const char *g_pszBlankRoomMenuLabel_00470090_WC1_UNMAPPED /* no-address */ =
    g_szBlankRoomMenuLabel_0047052c_WC1_UNMAPPED;
CampaignState g_stInitialCampaignState_004700b0_WC1_UNMAPPED /* no-address */ = {
    0,
    OBJECT_TYPE_HORNET,
    {0, 0, 0, 0, 0},
    {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    0,
    1,
    0,
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1},
    {110, 2654},
    {6, 0},
    0,
    0,
    0,
    0
};
PilotRecord g_aInitialPilotRecords_00470108_WC1_UNMAPPED /* no-address */[9] = {
    {"TANAKA",   "SPIRIT",  3, 1, 11, 14, 1},
    {"ST.JOHN",  "HUNTER",  4, 2, 25, 32, 4},
    {"CHEN",     "BOSSMAN", 1, 3, 35, 37, 2},
    {"CASEY",    "ICEMAN",  0, 3, 28, 43, 1},
    {"DEVEREAUX", "ANGEL",   0, 2, 22, 20, 1},
    {"TAGGART",  "PALADIN", 2, 3, 42, 34, 2},
    {"MARSHALL", "MANIAC",  4, 0, 5, 6, 1},
    {"KHUMALO",  "KNIGHT",  3, 2, 18, 23, 3},
    {"PELLEY",   "GOBLIN",  0, 0, 0, 0, 0}
};
signed char g_abSpiritRecRoomAnimation_00470260_WC1_UNMAPPED /* no-address */[48] = {
    0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 3, 4, 3, 3, 4,
    3, 4, 3, 4, 5, 3, 4, 5, 3, 4, 3, 4, 3, 4, 5, 3,
    2, 2, 2, 3, 2, 2, 2, 1, 1, 1, -1, 0, 0, 0, 0, 0
};
signed char g_abHunterRecRoomAnimation_00470290_WC1_UNMAPPED /* no-address */[40] = {
    0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 2, 2,
    3, 3, 4, 4, 5, 3, 3, 4, 4, 5, 5, 5, 2, 2, 1, 1,
    0, 0, -1, 0, 0, 0, 0, 0
};
signed char g_abAngelRecRoomAnimation_004702b8_WC1_UNMAPPED /* no-address */[48] = {
    3, 4, 3, 3, 4, 3, 4, 4, 3, 4, 3, 4, 0, 0, 0, 2,
    0, 2, 0, 1, 0, 2, 0, 0, 2, 0, 1, 0, 2, 3, 3, 4,
    3, 4, 4, 5, 5, 5, 5, 5, -1, 0, 0, 0, 0, 0, 0, 0
};
signed char g_abKnightRecRoomAnimation_004702e8_WC1_UNMAPPED /* no-address */[40] = {
    0, 0, 0, 0, 0, 1, 0, 1, 0, 5, 0, 0, 1, 2, 3, 4,
    4, 3, 2, 2, 2, 3, 4, 4, 5, 5, 0, 0, 1, 0, 1, 2,
    1, 0, 1, -1, 0, 0, 0, 0
};
signed char g_abIcemanRecRoomAnimation_00470310_WC1_UNMAPPED /* no-address */[40] = {
    0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 3, 3, 3, 3,
    4, 4, 4, 5, 4, 4, 5, 4, 4, 5, 4, 4, 5, 4, 4, 3,
    3, 2, 2, 1, 0, 0, -1, 0
};
signed char g_abManiacRecRoomAnimation_00470338_WC1_UNMAPPED /* no-address */[24] = {
    0, 1, 2, 0, 1, 2, 0, 3, 2, 0, 1, 1,
    0, 4, 2, 0, 1, 2, 0, 5, 5, 5, 5, -1
};
signed char g_abPaladinRecRoomAnimation_00470350_WC1_UNMAPPED /* no-address */[56] = {
    0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 2, 2, 2, 3, 2,
    2, 3, 2, 3, 2, 3, 2, 4, 4, 2, 3, 2, 3, 2, 3, 2,
    3, 2, 3, 4, 4, 5, 5, 4, 4, 4, 4, 5, 5, 4, 2, 2,
    2, -1, 0, 0, 0, 0, 0, 0
};
signed char g_abBossmanRecRoomAnimation_00470388_WC1_UNMAPPED /* no-address */[48] = {
    3, 3, 3, 2, 3, 2, 3, 2, 3, 3, 2, 3, 1, 3, 1, 3,
    0, 3, 0, 3, 0, 2, 1, 2, 1, 3, 3, 4, 5, 5, 4, 5,
    4, 5, 1, 3, 1, 3, 1, 3, -1, 0, 0, 0, 0, 0, 0, 0
};
signed char g_abShotglassIdleAnimation_004703b8_WC1_UNMAPPED /* no-address */[24] = {
    0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 2, 0,
    0, 3, 3, 3, -1, 0, 0, 0, 0, 0, 0, 0
};
signed char g_abShotglassGlassAnimation_004703d0_WC1_UNMAPPED /* no-address */[32] = {
    3, 6, 7, 8, 8, 9, 9, 10, 10, 10, 12, 11, 10, 12, 11, 10,
    12, 11, 10, 12, 11, 10, 13, 13, 13, 10, 9, 8, 8, 3, 3, -1
};
signed char g_abShotglassPourAnimation_004703f0_WC1_UNMAPPED /* no-address */[80] = {
    3, 3, 6, 6, 7, 8, 8, 14, 15, 15, 16, 17, 18, 18, 18, 18,
    17, 19, 19, 20, 20, 21, 20, 21, 20, 21, 20, 21, 20, 21, 20, 21,
    22, 22, 23, 24, 24, 25, 25, 26, 26, 27, 28, 28, 28, 29, 30, 31,
    31, 31, 31, 31, 32, 32, 31, 31, 31, 33, 33, 32, 34, 34, 35, 35,
    36, 36, 37, 37, 37, 38, 38, 39, 40, 3, 3, 3, -1, 0, 0, 0
};
signed char g_abShotglassWipeAnimation_00470440_WC1_UNMAPPED /* no-address */[24] = {
    3, 4, 3, 4, 5, 3, 4, 3, 5, 6, 3, 3,
    3, 3, 4, 3, 4, 3, -1, 0, 0, 0, 0, 0
};
signed char *g_apRecRoomAnimations_00470458_WC1_UNMAPPED /* no-address */[14] = {
    g_abSpiritRecRoomAnimation_00470260_WC1_UNMAPPED,
    g_abHunterRecRoomAnimation_00470290_WC1_UNMAPPED,
    g_abBossmanRecRoomAnimation_00470388_WC1_UNMAPPED,
    g_abIcemanRecRoomAnimation_00470310_WC1_UNMAPPED,
    g_abAngelRecRoomAnimation_004702b8_WC1_UNMAPPED,
    g_abPaladinRecRoomAnimation_00470350_WC1_UNMAPPED,
    g_abManiacRecRoomAnimation_00470338_WC1_UNMAPPED,
    g_abKnightRecRoomAnimation_004702e8_WC1_UNMAPPED,
    0,
    g_abShotglassIdleAnimation_004703b8_WC1_UNMAPPED,
    g_abShotglassGlassAnimation_004703d0_WC1_UNMAPPED,
    g_abShotglassPourAnimation_004703f0_WC1_UNMAPPED,
    g_abShotglassWipeAnimation_00470440_WC1_UNMAPPED,
    0
};
const ShortPoint g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED /* no-address */[3] = {
    {94, 59}, {161, 79}, {202, 79}
};
TitleMenuRegion g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED /* no-address */[7] = {
    { 1, 94, 59, 130, 95 },
    { 1, 161, 79, 180, 95 },
    { 1, 210, 79, 240, 95 },
    { 1, 180, 50, 250, 75 },
    { 1, 275, 50, 319, 135 },
    { 1, 0, 100, 120, 190 },
    { -1, 0, 0, 0, 0 }
};
char *g_apszRecRoomBaseLabels_004704e8_WC1_UNMAPPED /* no-address */[3] = {
    g_szTalkToShotglass_0047055c_WC1_UNMAPPED,
    g_szTalkToFirstPilot_00470570_WC1_UNMAPPED,
    g_szTalkToSecondPilot_00470588_WC1_UNMAPPED
};
char *g_apszRecRoomMenuLabels_004704f8_WC1_UNMAPPED /* no-address */[6] = {
    0, 0, 0,
    g_szCheckPilotScores_004705a0_WC1_UNMAPPED,
    g_szEnterBarracks_004705b4_WC1_UNMAPPED,
    g_szFlyTrainingMission_004705c4_WC1_UNMAPPED
};
int DAT_00470510_WC1_UNMAPPED /* no-address */;
CampaignDate g_stChalkBoardDate_00470514_WC1_UNMAPPED /* no-address */ = {-1, -1};
short g_asChalkBoardPilotOrder_00470518_WC1_UNMAPPED /* no-address */[9] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8
};
const char g_szBlankRoomMenuLabel_0047052c_WC1_UNMAPPED /* no-address */[4] = " ";
char g_szTalkToShotglass_0047055c_WC1_UNMAPPED /* no-address */[20] = "Talk to SHOTGLASS.";
char g_szTalkToFirstPilot_00470570_WC1_UNMAPPED /* no-address */[24] = "Talk to ??????????????";
char g_szTalkToSecondPilot_00470588_WC1_UNMAPPED /* no-address */[24] = "Talk to ??????????????";
char g_szCheckPilotScores_004705a0_WC1_UNMAPPED /* no-address */[20] = "Check pilot scores";
char g_szEnterBarracks_004705b4_WC1_UNMAPPED /* no-address */[16] = "Enter barracks";
char g_szFlyTrainingMission_004705c4_WC1_UNMAPPED /* no-address */[24] = "Fly training mission";
const char g_szRoomMenuLabelFormat_004705dc_WC1_UNMAPPED /* no-address */[12] = "%X%Y%s%P";
const char g_szEmptyRoomMenuLabel_004705e8_WC1_UNMAPPED /* no-address */[4] = "";
const char g_szTalkToPilotFormat1_004705ec_WC1_UNMAPPED /* no-address */[16] = "Talk to %s.";
const char g_szTalkToPilotFormat2_004705fc_WC1_UNMAPPED /* no-address */[16] = "Talk to %s.";
const char g_szChalkBoardHeading_0047060c_WC1_UNMAPPED /* no-address */[32] =
    "%X%YCARRIER - TIGER'S CLAW%P";
const char g_szChalkBoardPilotHeading_0047062c_WC1_UNMAPPED /* no-address */[12] = "%X%YPILOT";
const char g_szChalkBoardScoreHeading_00470638_WC1_UNMAPPED /* no-address */[20] =
    "%X%YSORTIES KILLS";
const char g_szChalkBoardPilotFormat_0047064c_WC1_UNMAPPED /* no-address */[12] = "%X%Y%s %s";
const char g_szChalkBoardScoreFormat_00470658_WC1_UNMAPPED /* no-address */[12] = "%X%Y%d%X%d";
const char g_szKilledInAction_00470664_WC1_UNMAPPED /* no-address */[4] = "KIA";
const char g_szChalkBoardKiaFormat_00470668_WC1_UNMAPPED /* no-address */[16] = "%X%Y     %s";
const int g_nJoystickUnavailableSample_0048e054 = -1;
const unsigned short g_wSpeechCacheCodeBytes_0048e0e0 = 0xfffa;
const unsigned short g_wSpeechCacheDataWords_0048e0e4 = 0x0fa0;
unsigned char *g_pDamageDisplayBackground_00490060;
const ShortPoint g_aDamageDisplayPositions_00490068[9] = {
    {36, 37}, {36, 28}, {36, 30}, {36, 23}, {36, 19},
    {36, 15}, {36, 24}, {36, 16}, {36, 22}
};
const char *g_apszComponentNames_00490090[9] = {
    g_szIonDrive_004900dc,
    g_szPowerPlant_004900e8,
    g_szShieldGenerator_004900f4,
    g_szComputerSystem_00490104,
    g_szIntercomUnit_00490114,
    g_szTargetTracking_00490124,
    g_szAccelerationAbsorbers_00490134,
    g_szEjectorSystem_00490144,
    g_szRepairSystems_00490154
};
const unsigned char g_abDamageDisplayFrames_004900b8[9] = {
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28
};
const char *g_apszDamageSeverityNames_004900c8[5] = {
    g_szDamageOk_00490164,
    g_szDamageLight_00490168,
    g_szDamageModerate_00490170,
    g_szDamageHeavy_0049017c,
    g_szDamageDestroyed_00490184
};
const char g_szIonDrive_004900dc[12] = "Ion drive";
const char g_szPowerPlant_004900e8[12] = "Power plant";
const char g_szShieldGenerator_004900f4[16] = "Shield gen'r";
const char g_szComputerSystem_00490104[16] = "Computer sys";
const char g_szIntercomUnit_00490114[16] = "InterCom unit";
const char g_szTargetTracking_00490124[16] = "Target track";
const char g_szAccelerationAbsorbers_00490134[16] = "Acc. absorbers";
const char g_szEjectorSystem_00490144[16] = "Ejector system";
const char g_szRepairSystems_00490154[16] = "Repair systems";
const char g_szDamageOk_00490164[4] = "Ok";
const char g_szDamageLight_00490168[8] = "Light";
const char g_szDamageModerate_00490170[12] = "Moderate";
const char g_szDamageHeavy_0049017c[8] = "Heavy";
const char g_szDamageDestroyed_00490184[12] = "Destroyed";
const char g_szDamageReport_00490190[16] = "DAMAGE REPORT";
const char g_szNoInternalDamage_004901a0[20] =
    "NO INTERNAL\n\nDAMAGE";
const char g_szDamagedUnitCountFormat_004901b4[20] =
    "%d Unit%c Damaged";
const char g_szDamageStatusFormat_004901c8[16] = "%s\nDamage: %s";
const char g_szDamageStatusFormatHighRes_004901d8[16] =
    "%s\nDamage: %s";
short g_nSpacePaletteFadeMode_004901e8 = -1;
void *g_pHighMemoryBlockA_004901f8;
unsigned short g_wHighMemoryBlockBytes_004901fc = 0xd4e4;
void *g_pHighMemoryBlockB_00490200;
const char g_szCannedSceneTapeFile_00490208[9] = "tape.tmp";
short g_nCannedSceneBufferNearCapacityFlag_00490214;
short g_nCannedSceneSegmentEndFrame_00490218 = -1;
short g_nCannedSceneMode_0049021c;
short g_nCannedSceneStateRecordMarker_00490220;
const char g_abCannedSceneBufferOverflowCode_00490224[4] = "028";
const char g_szCannedSceneCreateError_00490270[4] = "029";
const char g_szCannedSceneWriteOpenError_00490274[4] = "030";
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

MissionNavPoint g_aMissionNavPoints_00491e98[
    WC2_MISSION_NAV_POINT_COUNT] = { { 0 } };

MissionShipRecord g_aMissionShips_00492290[WC2_MISSION_SHIP_COUNT];
void *g_pActiveScenePacket_00492654;
char *g_pszPersonnelFooter_00492658;
Wc2PilotProfile g_stDefaultPilotProfile_00492660;
short g_bNewPilotCampaignInitialized_004926c0;
short g_bDeveloperCampaignReady_004926c4 = 1;
int g_anCloakVisibilityPattern_00492710[20] = {
    1, 0, 0, 0, 1, 0, 0, 1, 1, 0,
    0, 0, 1, 1, 1, 0, 0, 1, 1, 0
};
CutsceneObjectResourceList *g_pCutsceneSpriteResources_0049288c;
CutsceneObjectResourceList *g_pCutscenePlaneResources_00492890;
CutsceneObjectResourceList *g_pCutsceneSequenceResources_00492894;
CutsceneObjectResourceList *g_pCutsceneSceneResources_00492898;
CutsceneTextResource *g_pCutsceneTextResources_0049289c;
CutsceneResourceTable *g_pCutsceneFontResources_004928a0;
CutsceneResourceTable *g_pCutsceneSpeechResources_004928a4;
CutsceneResourceTable *g_pCutsceneShapeResources_004928a8;
CutsceneResourceTable *g_pCutsceneMouseResources_004928ac;
CutsceneResourceTable *g_pCutscenePaletteResources_004928b0;
CutsceneResourceTable *g_pCutsceneFilmResources_004928b4;
CutsceneResourceTable *g_pCutsceneFxResources_004928b8;
CutsceneResourceTable *g_pCutsceneMusicResources_004928bc;
const char *g_pszPacketReadOperation_00492b0c = "reading from disk";
const char *g_pszPacketAllocateOperation_00492b10 = "allocating memory";
const char *g_pszPacketLoadErrorFormat_00492b14 =
    "Sorry, an error has occurred while %s.\n"
    "Please note the following information:\n"
    "%Fs #%d (ERR %d  PS%ld  LB%ld  FL%d) at %s\n"
    "Check your configuration.  If this problem persists, please\n"
    "call Origin Systems' service line.  We are sorry for the inconvenience.";
short g_bAiMissileFiringEnabled_00492d58 = 1;
short g_bFriendlyFireWarningIssued_00492d5c;
short g_nLastAdaptiveDifficultyChangeFrame_00492d60;
short g_nObjectType62Index_00492d64 = -1;
short g_nObjectType63Index_00492d68 = -1;
short g_nLastFriendlyFireObjectFrame_00492d6c = 0x0ee4;
const short g_asPlayerDamageSystemTable_00492d70[50] = {
    0, 8, 6, 5, 0, 3, 5, 5, 7, 6,
    0, 8, 6, 5, 4, 3, 4, 0, 4, 4,
    1, 2, 5, 2, 7, 3, 4, 7, 5, 1,
    1, 4, 1, 5, 2, 3, 4, 7, 2, 1,
    4, 4, 4, 4, 0, 8, 6, 5, 4, 0
};
const short g_aeShipHitDebrisTypes_00492e10[4] = {
    WC2_OBJECT_TYPE_GIRDER_CHUNK,
    WC2_OBJECT_TYPE_SHIP_TUBING,
    WC2_OBJECT_TYPE_O_RING
};
short g_bExplosionTraversalIdle_00492e18 = 1;
const signed char g_acGunRefireDelay_00492e1c[4] = {5, 9, 3, 4};
const char g_szWeaponDestroyed_00492e20[20] = "Weapon destroyed";
const char g_szFuelTanksHit_00492e34[16] = "Fuel tanks hit";
const char g_szComponentFixedFormat_00492e44[8] = "%s FIXD";
const char g_szNeedLock_00492e4c[12] = "Need Lock";

short g_nHazardReferenceSpeed_00492e58;
short g_nActiveHazards_00492e5c;
signed char g_bLandingCommRequestPending_00492fa0;
int g_nCurrentView_00492fa8 = -1;
short g_nSpaceExplosionFlashActive_00492fb4;
short g_bApplyingCollisionDamage_00492fb8;
short g_asIntelligenceEvent_00492fc0[12];
signed char g_aDefenseManeuversNovice_00492fd8[8] = {
    24, 24, 13, 14, -1, 0, 0, 0
};
signed char g_aDefenseManeuversVeteran_00492fe0[8] = {
    8, 13, 15, 14, 19, 24, -1, 0
};
signed char g_aDefenseManeuversElite_00492fe8[16] = {
    8, 15, 17, 23, 19, 9, 20, 24, 14, -1, 0, 0, 0, 0, 0, 0
};
signed char g_aDefenseManeuversAce_00492ff8[16] = {
    17, 23, 15, 19, 9, 14, 20, 12, -1, 0, 0, 0, 0, 0, 0, 0
};
signed char g_aDefenseManeuversBoss_00493008[16] = {
    15, 19, 12, 11, 17, 23, 7, 9, -1, 0, 0, 0, 0, 0, 0, 0
};
signed char *g_apDefenseManeuvers_00493018[5] = {
    g_aDefenseManeuversNovice_00492fd8,
    g_aDefenseManeuversVeteran_00492fe0,
    g_aDefenseManeuversElite_00492fe8,
    g_aDefenseManeuversAce_00492ff8,
    g_aDefenseManeuversBoss_00493008
};
signed char g_acPilotAggression_00493030[8] = {
    3, 3, 3, 2, 2, 3, 0, 0
};
signed char g_acPilotRecovery_00493038[8] = {
    6, 7, 8, 8, 9, 8, 0, 0
};
short g_bWingmanTurnRateUnlocked_00493040;
short g_nGraphicsInitializationFlags_00493048 = 0x80;
short g_nInputDoubleClickInterval_00493050 = 1;
short g_nMenuPointerSpeed_00493054 = 2;
short g_bEjectionSequencePending_00493058;
short g_nPaletteTransitionInitialise_0049305c = 1;
short g_nAutopilotFormationShipCount_00493060;
short g_bAutopilotSequenceActive_00493064;
signed char g_acExpectedGraphicsModes_00493078[5] = {
    0x13, 0x0d, 4, 9, 7
};
/* Only the first row is this table's own storage: difflevl.000 is loaded
 * straight into g_asDifficultyLevels_004930a8, which is the next eight bytes
 * on, so GetAdaptiveTurnRate indexes on into the loaded packet exactly as the
 * original does. */
short g_aasSeriesMissionTurnRate_004930a0[1][4] = { { 108, 0, 0, 0 } };
short g_asDifficultyLevels_004930a8[52] = {
    50, 200, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127
};
signed char g_cCurrentKey_00493128 = (signed char)0x80;
signed char g_cPreviousKey_0049312c = (signed char)0x80;
unsigned int g_dwCannedSceneSnapshotStart_00493130;
short g_nSpaceFrame_00493134;
short g_nRenderedSpaceFrame_00493138;
signed char g_cViewObject_0049313c = -1;
short g_nEyePitchGoal_00493140;
short g_nEyeYawGoal_00493144;
short g_nEyeRollGoal_00493148;
short g_nEyePitchRate_0049314c = 1;
short g_nEyeYawRate_00493150 = 1;
short g_nEyeRollRate_00493154 = 1;
FixedVector g_vStarFieldMotion_00493158;
FixedVector g_vPreviousStarFieldMotion_00493168;
FixedVector g_vCollisionDelta_00493178;
FixedVector g_vToTarget_00493188;
short g_nFacingToTarget_00493194;
short g_nTargetFacing_00493198;
short g_nTargetRange_0049319c;
short g_nTargetShip_004931a0;
short g_bExpandedShipGraphicsEnabled_004931a4;
short g_nPitchInput_004931a8;
short g_nYawInput_004931aa;
short g_nRollInput_004931ac;
short g_nPreviousPitchInput_004931b0;
short g_nPreviousYawInput_004931b2;
short g_nPreviousRollInput_004931b4;
short g_nNavPointerObject_004931b8 = -1;
short g_nCurrentNavPoint_004931bc;
short g_nCurrentWave_004931c0 = -1;
short g_nLoadedCommPortraitPilot_004931c4 = -1;
int g_nCurrentObjectiveRange_004931c8;
signed char g_cCurrentObjective_004931cc = -1;
short g_nHazardFieldCount_004931d0;
HazardField g_aHazardFields_004931d8[7] = { { -1 } };
HazardField *g_pActiveHazardField_00493278;
short g_bUseEyePositionForHazards_0049327c;
signed char g_abHazardObjects_00493280[0x14] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
signed char g_cMissionObjectiveCount_00493294 = -1;
signed char g_cCurrentNavPointIndex_00493298 = -1;
signed char g_abFlightPath_004932a0[WC2_MISSION_OBJECTIVE_COUNT] = { -1 };
MissionObjective g_aMissionObjectives_004932a8[WC2_MISSION_OBJECTIVE_COUNT] = {
    { 0, 0, 0xff, 7, -1, 0xff, -1 }
};
ObjectResourceSlot g_aObjectResourceSlots_00493398[5] = { { -1 } };
Wc2PilotProfile g_stCurrentPilotProfile_00493408;
short g_nMissionScore_00493462;
short g_nPlayerShipType_00493464;
short g_nExternalViewShip_00493468 = -1;
short g_nYourWingman_0049346c = -1;
signed char g_acPlayerComponentDamage_00493470[9] = { -1 };
short g_nPlayerCollisionObject_00493480 = -1;
FixedVector g_vPlayerAcceleration_00493488;
int g_aaiVduModeStack_00493498[2][4];
signed char g_acVduModeStackDepth_004934c8[2] = { -1, 0 };
int g_anLastDrawnVduMode_004934d0[2];
short g_nSelectedGunType_004934dc = -1;
short g_nSelectedReleaseWeaponIndex_004934e0 = -1;
short DAT_004934e4;
short g_bTargetLockReadoutDirty_004934e8;
short g_nTargetLockCountdown_004934ec;
short g_nTargetLockMarkerAngle_004934f0;
signed char g_cTargetDisplayObject_004934f4 = -1;
short g_bTargetLockAcquired_004934fc;
short g_bTargetLockMode_00493500;
short g_bMissileCameraEnabled_00493504;
short g_asObjectViewFrame_00493508[WC2_SPACE_OBJECT_COUNT];
short g_asObjectScreenX_00493598[WC2_SPACE_OBJECT_COUNT];
short g_asObjectScreenY_00493628[WC2_SPACE_OBJECT_COUNT];
short g_asObjectScreenAngle_004936b8[WC2_SPACE_OBJECT_COUNT];
short g_asObjectDrawX_00493748[WC2_SPACE_OBJECT_COUNT];
short g_asObjectDrawY_004937d8[WC2_SPACE_OBJECT_COUNT];
unsigned char *g_apObjectShape_00493868[WC2_SPACE_OBJECT_COUNT];
signed char g_acObjectType_00493980[WC2_SPACE_OBJECT_COUNT];
short g_asObjectFlip_004939c8[WC2_SPACE_OBJECT_COUNT];
short g_asObjectScreenScale_00493a58[WC2_SPACE_OBJECT_COUNT];
short g_asObjectDistance_00493ae8[WC2_SPACE_OBJECT_COUNT];
FixedVector g_aShipRightVector_00493b78[WC2_SPACE_OBJECT_COUNT];
FixedVector g_aShipUpVector_00493ec0[WC2_SPACE_OBJECT_COUNT];
FixedVector g_aShipForwardVector_00494208[WC2_SPACE_OBJECT_COUNT];
FixedVector g_aShipPosition_00494550[WC2_SPACE_OBJECT_COUNT];
FixedVector g_aShipVelocity_00494898[WC2_SPACE_OBJECT_COUNT];
short g_asObjectCounter_00494be0[WC2_SPACE_OBJECT_COUNT];
short g_asObjectAnimationIndex_00494c70[WC2_SPACE_OBJECT_COUNT];
signed char g_acObjectAnimationDelay_00494d00[WC2_SPACE_OBJECT_COUNT];
signed char g_acObjectCollisionGraceTicks_00494d48[
    WC2_SPACE_OBJECT_COUNT];
unsigned short g_asObjectScale_00494d90[WC2_SPACE_OBJECT_COUNT];
int g_anShipSpeed_00494e20[WC2_SPACE_OBJECT_COUNT];
short g_anObjectPitchRotation_00494f38[WC2_SPACE_OBJECT_COUNT];
short g_anObjectYawRotation_00494fc8[WC2_SPACE_OBJECT_COUNT];
short g_anObjectRollRotation_00495058[WC2_SPACE_OBJECT_COUNT];
short g_asObjectCollisionRadius_004950e8[70];
short g_asObjectDamage_00495178[WC2_SPACE_OBJECT_COUNT];
signed char g_acObjectOwner_00495208[WC2_SPACE_OBJECT_COUNT] = { -1 };
signed char g_acLastCollisionObject_00495250[WC2_SPACE_OBJECT_COUNT] = { -1 };
short g_asObjectType_00495298[WC2_SPACE_OBJECT_COUNT];
short g_aeObjectClass_00495328[WC2_SPACE_OBJECT_COUNT];
unsigned char *g_apObjectExhaustShape_004953b8[10];
signed char g_acShipNavPoint_004953e0[12] = { -1 };
FixedVector g_aShipDestination_004953f0[10];
ShortVector g_aShipFormationOffset_00495468[10];
short g_anPitchGoal_004954a8[12];
short g_anYawGoal_004954c0[12];
short g_anRollGoal_004954d8[12];
short g_aasShipMaximumShield_004954f0[10][2];
short g_aasShipShield_00495518[10][2];
short g_aasShipArmor_00495540[10][4];
short g_asShipWeaponEnergy_00495590[10];
short g_asShipAfterburnerTimer_004955a8[10];
signed char g_acShipLastAttacker_004955c0[16] = { -1 };
short g_asShipSide_004955d0[12] = { 2 };
short g_asActionCount_004955e8[12];
short g_aeSpecialManeuver_00495600[12];
signed char g_acTurnRegulator_00495618[16] = { -1 };
signed char g_acTurnInterval_00495628[16] = { -1 };
int g_anShipFuel_00495638[10];
signed char g_acShipExhaustHeat_00495660[16] = { -1 };
signed char g_acShipCommunicator_00495670[16] = { -1 };
signed char g_acShipDestroyedWeaponCount_00495680[16] = { -1 };
signed char g_acShipDamage_00495690[16] = { -1 };
signed char g_acShipIonDriveDamage_004956a0[16] = { -1 };
unsigned char g_aShipWeapons_004956b0[10][0xA1] = {
    { 0xff, 0, 0, 0, 0, 0xff, 0xff }
};
short g_asShipMissionIndex_00495d00[12];
short g_asLoadedShipViewFrame_00495d18[12];
short g_asShipIntelSlot_00495d30[10];
short g_asShipIntelResourceKey_00495d48[10];
short g_asPilotLevel_00495d60[12];
signed char DAT_00495d78[16] = { -1 };
signed char g_acShipPortrait_00495d88[12] = { -1 };
signed char g_acShipPendingMessage_00495d98[16] = { -1 };
MissionShipRecord *g_apShipMissionRecord_00495da8[10];
short g_asShipWingLeader_00495dd0[12];
short g_asShipMissionType_00495de8[12];
short g_asShipMissionParameter_00495e00[12];
FixedVector g_aShipMissionSpot_00495e18[10];
short g_aeShipObjective_00495f08[12];
signed char g_acShipTarget_00495f20[16] = { -1 };
short g_asShipTactic_00495f30[12];
short g_asShipManeuver_00495f48[12] = { 29 };
signed char g_abShipNavPointIndex_00495f60[16] = { -1 };
short g_asShipMaximumVelocity_00495f70[12];
unsigned int g_anShipAlertFlags_00495f88[10];
signed char g_acShipAlertSource_00495fb0[12] = { -1 };
short g_asShipAlertCountdown_00495fc0[12];
signed char g_abShipTurn_00495fd8[16] = { -1 };
signed char g_acShipSequence_00495fe8[14] = { -1 };
short g_asShipCount_00495ff8[12];
signed char g_acShipCollisionCooldown_00496010[16];
int g_anShipCloakState_00496020[10] = { -1 };
short g_asShipCloakCooldown_00496048[10];
short g_asShipCloakElapsedFrames_00496060[10];
short g_asShipIdentified_00496078[12] = { 1 };
short g_asShipFriendlyFireCooldown_00496090[10];
signed char g_abProjectileCollisionBonus_004960a8[
    WC2_SPACE_OBJECT_COUNT];
short DAT_004960f0[8];
signed char g_acShipStress_00496100[16];
signed char g_abPilotEjectionAttempted_00496110[16];
signed char g_abShipEjectionSequenceEnabled_00496120[12];
int g_bEngageAllowed_0049612c;
short g_nAutoEngageTimer_00496130 = -1;
short g_nWingmanRoutDecisionMode_00496138;
short g_nWingmanFormationDisobeyMode_0049613a;
short g_nWingmanTargetingMode_0049613c;
short g_nWingmanEngagementMode_0049613e;
int g_nMissionMusicTrackOverride_00496144;
signed char g_acShipList_00496148[16] = { -1 };
short g_asShipListRange_00496158[16];
signed char g_cViableTargetCount_00496178;
signed char g_acViableTarget_00496180[16] = { -1 };
short g_asViableTargetDistance_00496190[10];
ActiveSoundEntry *g_pActiveSoundHead_004961a8;
ActiveSoundEntry *g_pActiveSoundTail_004961ac;
int g_bAudioSystemInitialized_004961b0;
WaveTableEntry *g_pWaveTableHead_004961b4;
WaveTableEntry *g_pWaveTableTail_004961b8;
const char g_szPlayWaveOpenError_004961ec[36] =
    "playWAVE Unable to open file '%s'";
short g_nEscapedEnemyCount_004962e8;
short g_bStarSystemJumpTransition_004962ec;
short g_bJumpSequenceActive_004962f0;
short g_nStarSystemJumpDelay_004962f4;
unsigned char g_abPaletteTranslation_00496338[256] = {
      0,   1,   2,   3,   4,   5,   6,   7,
      8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,
     24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,
     40,  41,  42,  43,  44,  45,  46,  47,
     48,  49,  50,  51,  52,  53,  54,  55,
     56,  57,  58,  59,  60,  61,  62,  63,
     64,  65,  66,  67,  68,  69,  70,  71,
     72,  73,  74,  75,  76,  77,  78,  79,
     80,  81,  82,  83,  84,  85,  86,  87,
     88,  89,  90,  91,  92,  93,  94,  95,
     96,  97,  98,  99, 100, 101, 102, 103,
    104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151,
    152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167,
    168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215,
    216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247,
    248, 249, 250, 251, 252, 253, 254, 255
};
unsigned short g_awAbsoluteCosine_00496438[360] = {
    255, 255, 255, 255, 255, 255, 255, 254, 254, 253, 252, 251,
    250, 250, 248, 247, 246, 245, 243, 242, 241, 239, 237, 236,
    234, 232, 230, 228, 226, 224, 221, 219, 217, 215, 212, 210,
    207, 204, 202, 198, 196, 193, 190, 187, 184, 181, 178, 175,
    171, 168, 165, 161, 158, 154, 150, 147, 143, 139, 136, 131,
    128, 124, 120, 116, 112, 108, 104, 100,  96,  92,  88,  83,
     79,  74,  71,  66,  62,  58,  53,  49,  45,  40,  36,  31,
     27,  22,  18,  13,   9,   4,   0,   4,   9,  13,  18,  22,
     27,  31,  36,  40,  45,  49,  53,  58,  62,  66,  71,  75,
     79,  83,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124,
    128, 131, 136, 139, 143, 147, 150, 154, 158, 161, 165, 168,
    171, 175, 178, 181, 184, 187, 190, 193, 196, 198, 202, 204,
    207, 210, 212, 215, 217, 219, 221, 224, 226, 228, 230, 232,
    234, 236, 237, 239, 241, 242, 243, 245, 246, 247, 248, 250,
    250, 251, 252, 253, 254, 254, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 254, 254, 253, 252, 251,
    250, 250, 248, 247, 246, 245, 243, 242, 241, 239, 237, 236,
    234, 232, 230, 228, 226, 224, 221, 219, 217, 215, 212, 210,
    207, 204, 202, 198, 196, 193, 190, 187, 184, 181, 178, 175,
    171, 168, 165, 161, 158, 154, 150, 147, 143, 139, 136, 131,
    128, 124, 120, 116, 112, 108, 104, 100,  96,  92,  88,  83,
     79,  74,  71,  66,  62,  58,  53,  49,  45,  40,  36,  31,
     27,  22,  18,  13,   9,   4,   0,   4,   9,  13,  18,  22,
     27,  31,  36,  40,  45,  49,  53,  58,  62,  66,  71,  75,
     79,  83,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124,
    128, 131, 136, 139, 143, 147, 150, 154, 158, 161, 165, 168,
    171, 175, 178, 181, 184, 187, 190, 193, 196, 198, 202, 204,
    207, 210, 212, 215, 217, 219, 221, 224, 226, 228, 230, 232,
    234, 236, 237, 239, 241, 242, 243, 245, 246, 247, 248, 250,
    250, 251, 252, 253, 254, 254, 255, 255, 255, 255, 255, 255
};
unsigned short g_awAbsoluteSine_00496708[360] = {
      0,   4,   9,  13,  18,  22,  27,  31,  36,  40,  45,  49,
     53,  58,  62,  66,  71,  75,  79,  83,  88,  92,  96, 100,
    104, 108, 112, 116, 120, 124, 128, 131, 136, 139, 143, 147,
    150, 154, 158, 161, 165, 168, 171, 175, 178, 181, 184, 187,
    190, 193, 196, 198, 202, 204, 207, 210, 212, 215, 217, 219,
    221, 224, 226, 228, 230, 232, 234, 236, 237, 239, 241, 242,
    243, 245, 246, 247, 248, 250, 250, 251, 252, 253, 254, 254,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 254, 254, 253, 252, 251, 250, 250, 248, 247, 246, 245,
    243, 242, 241, 239, 237, 236, 234, 232, 230, 228, 226, 224,
    221, 219, 217, 215, 212, 210, 207, 204, 202, 198, 196, 193,
    190, 187, 184, 181, 178, 175, 171, 168, 165, 161, 158, 154,
    150, 147, 143, 139, 136, 131, 128, 124, 120, 116, 112, 108,
    104, 100,  96,  92,  88,  83,  79,  74,  71,  66,  62,  58,
     53,  49,  45,  40,  36,  31,  27,  22,  18,  13,   9,   4,
      0,   4,   9,  13,  18,  22,  27,  31,  36,  40,  45,  49,
     53,  58,  62,  66,  71,  75,  79,  83,  88,  92,  96, 100,
    104, 108, 112, 116, 120, 124, 128, 131, 136, 139, 143, 147,
    150, 154, 158, 161, 165, 168, 171, 175, 178, 181, 184, 187,
    190, 193, 196, 198, 202, 204, 207, 210, 212, 215, 217, 219,
    221, 224, 226, 228, 230, 232, 234, 236, 237, 239, 241, 242,
    243, 245, 246, 247, 248, 250, 250, 251, 252, 253, 254, 254,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 254, 254, 253, 252, 251, 250, 250, 248, 247, 246, 245,
    243, 242, 241, 239, 237, 236, 234, 232, 230, 228, 226, 224,
    221, 219, 217, 215, 212, 210, 207, 204, 202, 198, 196, 193,
    190, 187, 184, 181, 178, 175, 171, 168, 165, 161, 158, 154,
    150, 147, 143, 139, 136, 131, 128, 124, 120, 116, 112, 108,
    104, 100,  96,  92,  88,  83,  79,  74,  71,  66,  62,  58,
     53,  49,  45,  40,  36,  31,  27,  22,  18,  13,   9,   4
};
const char g_szInvalidShapeAllocation_004969ec[12] = "not jefftep";
const char g_szBadViewport_004969f8[12] = "bad vport";
const char g_szShapeRLEVersion_00496a04[8] = "1.00";
const char g_szShapeRLEOverflow_00496a0c[20] = "qq copy overflow";
const char g_szBadShapeFlip_00496a20[12] = "bad flip";
const char g_szCopyLine_00496a2c[12] = "__copyline";
const char g_szWipeLine_00496a38[12] = "__wipeline";
const char g_szTriangle_00496a54[12] = "triangle";
const char g_szShadedTriangle_00496a60[16] = "shaded_triangle";
const char g_szShadowDraw_00496a70[12] = "shadow_draw";
const char g_szFizzleFade_00496a7c[12] = "fizzle_fade";
const char g_szSnowViewport_00496a88[16] = "snow_viewport";



/* Frame lists the object-type records point at; 0xA000 and 0x9000 end a
 * sequence.  They sit immediately below the table in the retail image. */
static const short g_asExplosionSmallAnimation_00496af0[8] = {
    0, 1, 2, 3, 4, 5, 6, (short)0xa000
};
static const short g_asExplosionMediumAnimation_00496b00[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, (short)0xa000, 0, 0
};
static const short g_asExplosionLargeAnimation_00496b20[20] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    (short)0xa000, 0, 0
};
static const short g_asLaserSparkAnimation_00496b48[8] = {
    0, 1, 2, 3, 4, 5, (short)0xa000, 0
};
static const short g_asBlueSparkAnimation_00496b58[8] = {
    0, 1, 2, 3, (short)0xa000, 0, 0, 0
};
static const short g_asRedSparkAnimation_00496b68[8] = {
    0, 1, 2, 3, (short)0xa000, 0, 0, 0
};
static const short g_asSparkTrailAnimation_00496b78[8] = {
    0, 1, 2, 3, (short)0xa000, 0, 0, 0
};
static const short g_asGirderChunkAnimation_00496b88[8] = {
    0, 1, 2, 3, 4, 5, (short)0x9000, 0
};
static const short g_asShipTubingAnimation_00496b98[8] = {
    6, 7, 8, 9, 10, 11, (short)0x9000, 0
};
static const short g_asBurningDebrisAnimation_00496ba8[20] = {
    12, 13, 14, 15, 16, 17, 18, 19, 146, 145, 144, 143, 142, 141, 140,
    20, (short)0x9000, 0, 0, 0
};
static const short g_asORingDebrisAnimation_00496bd0[8] = {
    21, 22, 23, 24, 25, 26, (short)0x9000, 0
};
static const short g_asPipeDebrisAnimation_00496be0[8] = {
    27, 28, 29, 30, 31, 32, (short)0x9000, 0
};
static const short g_asMetalSheetAnimation_00496bf0[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, (short)0x9000, 0, 0
};
static const short g_asDataCapsuleAnimation_00496c10[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, (short)0x9000
};
static const short g_asMineAnimation_00496c28[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, (short)0x9000
};
static const short g_asChaffPodAnimation_00496c40[20] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, (short)0x9000, 0, 1, 2, 3, 4, 5,
    (short)0x9000, 0
};
static const short g_asAsteroidSpinAnimation_00496c68[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, (short)0x9000, 0, 0
};
static const short g_asAsteroidRollAnimation_00496c88[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, (short)0x9000, 0, 0
};
static const short g_asAsteroidSpinReverseAnimation_00496ca8[16] = {
    12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, (short)0x9000, 0, 0
};
static const short g_asAsteroidRollReverseAnimation_00496cc8[16] = {
    12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, (short)0x9000, 0, 0
};
static const short g_asEjectedPilotAnimation_00496ce8[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, (short)0x9000, 0
};
static const short g_asShipDebrisFlashAnimation_00496d00[24] = {
    0, 1026, 1028, 1028, 1032, 1032, 1032, 1040, 1040, 1040, 1040, 2080,
    2080, 2064, 2056, 2052, 1, 2052, 2052, 2052, 2052, (short)0xa000, 0,
    0
};

/* WC2 keeps its own 49-record table: slots 0-6 are the ship types the game
 * loads at runtime and stay zero in the image, and 7-48 are the static
 * projectile, missile, asteroid, debris and explosion records read straight
 * out of the retail .data. */
ObjectTypeData g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE_COUNT] = {
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    {
        "Laser cannon", OBJECT_CLASS_PROJECTILE, 0, 7, 10, 0, 512, 7,
        30, 0, 25, 0, 160
    },
    {
        "Neutron gun", OBJECT_CLASS_PROJECTILE, 0, 8, 10, 1, 832, 14,
        20, 0, 40, 0, 140
    },
    {
        "Mass driver", OBJECT_CLASS_PROJECTILE, 0, 9, 10, 0, 512, 9, 25,
        0, 30, 0, 120
    },
    {
        "Particle cannon", OBJECT_CLASS_PROJECTILE, 0, 10, 10, 0, 512,
        9, 30, 0, 53, 0, 220
    },
    { "", OBJECT_CLASS_PROJECTILE, 0, 11, 10, 0, 512, 7, 30, 0, 25, 0, 160 },
    {
        "", OBJECT_CLASS_PROJECTILE, 0, 12, 15, 0, 832, 15, 40, 0, 50,
        4000, 180
    },
    {
        "", OBJECT_CLASS_PROJECTILE, 0, 13, 1, 0, 832, 15, 100, 0, 300,
        0, 250
    },
    {
        "", OBJECT_CLASS_PROJECTILE, 0, 14, 1, 0, 832, 15, 80, 0, 1200,
        0, 150
    },
    {
        "Dart DF", OBJECT_CLASS_MISSILE, 0, 15, 20, 5, 768, 500, 120, 0,
        4, 14500, 150, 0, 0, 1433, 15, 15, 15, 100
    },
    {
        "Javelin HS", OBJECT_CLASS_MISSILE, 0, 16, 20, 5, 768, 400, 140,
        0, 4, 13500, 150, 0, 0, 1689, 11, 11, 11, 100
    },
    {
        "Pilum FF", OBJECT_CLASS_MISSILE, 0, 17, 20, 5, 768, 400, 160,
        0, 4, 10500, 150, 0, 0, 1689, 11, 11, 11, 100
    },
    {
        "Spiculum IR", OBJECT_CLASS_MISSILE, 0, 18, 20, 5, 768, 400,
        110, 0, 4, 11500, 150, 0, 0, 1689, 11, 11, 11, 100
    },
    {
        "Torpedo", OBJECT_CLASS_MISSILE, 0, 19, 55, 10, 1280, 400, 8200,
        0, 20, 30000, 50, 0, 0, 512, 4, 4, 4, 500
    },
    {
        "Chaff Pod", OBJECT_CLASS_MISSILE, 0, 20, 50, 1, 768, 400, 160,
        0, 4, 1000, 90, 0, (unsigned char
        *)g_asChaffPodAnimation_00496c40, 1689, 11, 11, 11, 100
    },
    {
        "Porcupine", OBJECT_CLASS_MINE, 0, 21, 20, 5, 768, 110, 120, 0,
        4, 20000, 20, 20, (unsigned char *)g_asMineAnimation_00496c28,
        0, 0, 2, 2
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 22, 100, 300, 1024, 0, 0, 0, -1,
        0, 0, 0, (unsigned char *)g_asAsteroidSpinAnimation_00496c68, 0,
        0, 1, 13
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 23, 100, 300, 1024, 0, 0, 0, -1,
        0, 0, 0, (unsigned char *)g_asAsteroidRollAnimation_00496c88, 0,
        0, 1, 12
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 24, 100, 300, 1024, 0, 0, 0, -1,
        0, 0, 0, (unsigned char
        *)g_asAsteroidSpinReverseAnimation_00496ca8, 0, 0, 1, 13
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 25, 100, 300, 1024, 0, 0, 0, -1,
        0, 0, 0, (unsigned char
        *)g_asAsteroidRollReverseAnimation_00496cc8, 0, 0, 1, 12
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 26, 100, 300, 1024, 0, 0, 0, -1,
        0, 0, 0, (unsigned char *)g_asAsteroidSpinAnimation_00496c68, 0,
        0, 2, 13
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 27, 100, 300, 1024, 0, 0, 0, -1,
        0, 0, 0, (unsigned char *)g_asAsteroidRollAnimation_00496c88, 0,
        0, 2, 12
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 28, 10, 4, 192, 0, 0, 0, -1, 0, 0,
        0, (unsigned char *)g_asAsteroidSpinAnimation_00496c68, 0, 0, 2,
        13
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 29, 10, 1, 2048, 0, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asGirderChunkAnimation_00496b88, 0, 0, 2,
        5
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 30, 10, 1, 2048, 0, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asShipTubingAnimation_00496b98, 0, 0, 2, 5
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 31, 20, 2, 1280, 0, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asMetalSheetAnimation_00496bf0, 0, 0, 1,
        11
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 32, 20, 2, 1280, 0, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asDataCapsuleAnimation_00496c10, 0, 0, 1,
        2
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 33, 20, 2, 768, 0, 0, 0, 0, 0, 0, 0,
        (unsigned char *)g_asBurningDebrisAnimation_00496ba8, 0, 0, 1,
        15
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 34, 2, 1, 1792, 0, 0, 0, 0, 0, 0, 0,
        (unsigned char *)g_asORingDebrisAnimation_00496bd0, 0, 0, 1, 5
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 35, 6, 1, 1536, 0, 0, 0, 0, 0, 0, 0,
        (unsigned char *)g_asPipeDebrisAnimation_00496be0, 0, 0, 1, 5
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 36, 0, 0, 768, 0, 0, 0, -1, 6000,
        0, 0, (unsigned char *)g_asExplosionSmallAnimation_00496af0, 0,
        0, 1
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 37, 0, 0, 1024, 0, 0, 0, -1,
        6000, 0, 0, (unsigned char
        *)g_asExplosionMediumAnimation_00496b00, 0, 0, 1
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 38, 0, 0, 336, 0, 0, 0, -1, 6000,
        0, 0, (unsigned char *)g_asExplosionLargeAnimation_00496b20, 0,
        0, 1
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 39, 0, 0, 256, 0, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asLaserSparkAnimation_00496b48, 0, 0, 1
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 40, 0, 0, 256, 0, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asRedSparkAnimation_00496b68, 0, 0, 2, 3
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 41, 0, 0, 256, 0, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asBlueSparkAnimation_00496b58, 0, 0, 2, 3
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 42, 1, 1, 256, 0, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asSparkTrailAnimation_00496b78, 0, 0, 2, 3
    },
    { "", OBJECT_CLASS_FIXED_OBJECT, 0, 43, 0, 0, 256 },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 44, 6, 10, 512, 0, 0, 0, 0, 0, 0, 0,
        (unsigned char *)g_asEjectedPilotAnimation_00496ce8, 0, 0, 1, 12
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 45, 0, 0, 1024, 0, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asShipDebrisFlashAnimation_00496d00, 0, 0,
        1
    },
    {
        "data capsule", OBJECT_CLASS_DEBRIS, 0, 61, 1, 10, 1024, 0, 0,
        0, 0, 0, 0, 0, (unsigned char
        *)g_asDataCapsuleAnimation_00496c10, 0, 0, 1
    },
    {
        "", OBJECT_CLASS_FIXED_OBJECT, 0, 62, 0, 0, 192, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1
    },
    {
        "", OBJECT_CLASS_FIXED_OBJECT, 0, 63, 0, 0, 192, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1
    }
};
int g_bSceneDisplayUpdateActive_00499bb8;
short g_bKilrathiAceGreetingSent_00499bf4;
unsigned char g_bMemoryLogToFile_00499bf8;
unsigned char g_bAutopilotDebugEnabled_00499bfc;
unsigned char g_bRoomTransitionAnimationEnabled_00499c00 = 1;
void *g_pCutsceneCockpitPacket_00499c04;
void *g_pSceneMusicPacket_00499c08;
void *g_pCutsceneCockpitPalette_00499c0c;
short g_nCutsceneSpritesDrawn_00499c10;
const char *g_apszCutsceneObjectTypeNames_00499c18[4] = {
    g_szCutsceneSpriteType_00499f44,
    g_szCutscenePlaneType_00499f4c,
    g_szCutsceneSequenceType_00499f54,
    g_szCutsceneScriptType_00499f60
};
CutsceneSoundEffect *g_pCutsceneSoundEffects_00499c28;
signed char g_cCutsceneSoundVolume_00499c2c = 0x40;
void *g_pCutsceneFxPacket_00499c30;
CutsceneMusicNode *g_pCutsceneMusicPackets_00499c34;
SceneFlicObject **g_apSceneObjects_00499c38;
CutscenePlane **g_apCutscenePlanes_00499c3c;
CutsceneSequence **g_apCutsceneSequences_00499c40;
CutsceneScene **g_apCutsceneScenes_00499c44;
signed char g_cCutsceneVideoMode_00499c48 = 0x13;
signed char g_bCutsceneViewportPreallocated_00499c4c;
short g_nSceneFlicContext_00499c50;
signed char g_bCutsceneSkipFrame_00499c54;
signed char g_bCutsceneSkipAll_00499c58;
signed char g_bCutsceneDrawingEnabled_00499c60;
SceneFlicObject *g_pLinkedCutsceneSprite_00499c64;
CutscenePlane *g_pLinkedCutscenePlane_00499c68;
CutsceneSequence *g_pLinkedCutsceneSequence_00499c6c;
SceneFlicObject *g_pCurrentCutsceneSprite_00499c78;
CutscenePlane *g_pCurrentCutscenePlane_00499c7c;
CutsceneSequence *g_pCurrentCutsceneSequence_00499c80;
CutsceneScene *g_pCurrentCutsceneScene_00499c84;
short g_nCutsceneFrameTick_00499c88;
unsigned short g_nCutsceneFrameDelay_00499c8c;
unsigned int g_nNextCutsceneFrameClock_00499c90;
Wc2CampaignGlobals *g_pCampaignGlobals_00499c94;
unsigned char g_abCutscenePaletteTranslation_00499c98[256] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    7, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 6, 6, 6, 6, 6,
    11, 11, 11, 9, 9, 9, 9, 9, 9, 9, 9, 9, 1, 1, 1, 1,
    15, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    14, 14, 14, 14, 14, 14, 14, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    12, 12, 12, 12, 12, 12, 12, 12, 14, 10, 10, 10, 10, 10, 2, 2,
    14, 14, 14, 14, 14, 14, 14, 14, 6, 6, 6, 6, 6, 6, 6, 6,
    12, 12, 12, 12, 12, 12, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    10, 10, 10, 10, 10, 10, 10, 10, 4, 4, 4, 4, 4, 4, 4, 4,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 9, 9, 9, 9, 1,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8,
    12, 12, 12, 12, 12, 12, 12, 12, 4, 4, 4, 4, 4, 4, 4, 4,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 2, 2, 2, 2, 2, 2,
    7, 7, 7, 7, 8, 8, 8, 8, 12, 12, 12, 12, 4, 4, 4, 4,
    10, 10, 10, 10, 2, 2, 2, 2, 13, 13, 13, 13, 13, 13, 13, 13
};
short g_nCutsceneResourceLevel_00499d98;
short g_nActiveCutsceneResourceLevel_00499d9c;
signed char g_bCutsceneTextRestorePending_00499da0;
char *g_pszCurrentCutsceneText_00499da4;
FILE *g_pMemoryLogFile_00499da8;
signed char g_acCutsceneMouthFrames_00499db0[128] = {
    10, 13, 14, 12, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    3, 5, 5, 1, 5, 9, 5, 5, 1, 2, 5, -1, -1, -1, -1, -1,
    -1, 1, 0, 5, 5, 1, 9, 5, 3, 2, 5, 5, 8, 0, 5, 3,
    0, 6, 5, 5, 5, 4, 9, 6, 5, 5, 5, 1, -1, -1, -1, -1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
signed char g_acCutsceneMouthDurations_00499e30[128] = {
    40, 3, 3, 50, 60, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,
    1, 1, 1, 1, 2, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
char *g_pszCutsceneSpeechCursor_00499eb0;
signed char g_cCutsceneSpeechSpeed_00499eb4 = 1;
signed char g_bCutsceneSpeechActive_00499eb8;
signed char g_bCutsceneTextBoundsOverride_00499edc;
short g_nCutsceneTextLeft_00499ee0;
short g_nCutsceneTextTop_00499ee2;
short g_nCutsceneTextRight_00499ee4 = 319;
short g_nCutsceneTextBottom_00499ee6 = 199;
signed char g_bCutsceneDebugOverlayEnabled_00499ee8;
unsigned short g_wCutsceneDebugFrameNumber_00499eec;
CutscenePlane *g_pCutsceneSpriteParentPlane_00499ef0;
char g_szPilotCallsign_00499ef8[24] = "CALLSIGN";
char g_szPilotLastName_00499f10[24] = "PCNAME";
char g_szPilotFirstName_00499f28[24] = "FIRSTNAME";
signed char g_cCutsceneTextStyle_00499f40;
const char g_szCutsceneSpriteType_00499f44[8] = "sprite";
const char g_szCutscenePlaneType_00499f4c[8] = "plane";
const char g_szCutsceneSequenceType_00499f54[12] = "sequence";
const char g_szCutsceneScriptType_00499f60[8] = "script";
unsigned char *g_pCockpitBackgroundPacket_0049a5f0;
unsigned char *g_apCockpitVduOverlayShapes_0049a5f8[2];
int g_nLegacySaveSource_0049a600;
const char *g_apszPilotRankNames_0049a608[5] = {
    g_szSecondLieutenantRank_0049a6d8,
    g_szFirstLieutenantRank_0049a6e0,
    g_szCaptainRank_0049a6e8,
    g_szMajorRank_0049a6f0,
    g_szLieutenantColonelRank_0049a6f8
};
const char *g_apszShortPilotRankNames_0049a620[5] = {
    g_szShortSecondLieutenantRank_0049a704,
    g_szShortFirstLieutenantRank_0049a70c,
    g_szShortCaptainRank_0049a714,
    g_szShortMajorRank_0049a71c,
    g_szShortLieutenantColonelRank_0049a724
};
short g_nPersonnelMenuHighlight_0049a6a0 = -1;
unsigned char *g_pPersonnelMenuBackdrop_0049a6b8;
short g_sPersonnelPreviousInput_0049a6bc = -128;
short g_bPersonnelMenuDrawing_0049a6c0 = 1;
const char *g_apszWc1SaveGameFileNames_0049a6c8[3] = {
    "savegame.wld", "savegame.wld", "crusade.wld"
};
signed char g_bPersonnelPrimaryInputDown_0049a6d4;
const char g_szSecondLieutenantRank_0049a6d8[8] = "2ND LT.";
const char g_szFirstLieutenantRank_0049a6e0[8] = "1ST LT.";
const char g_szCaptainRank_0049a6e8[8] = "CAPTAIN";
const char g_szMajorRank_0049a6f0[8] = "MAJOR";
const char g_szLieutenantColonelRank_0049a6f8[12] = "LT. COL.";
const char g_szShortSecondLieutenantRank_0049a704[8] = "2ND LT";
const char g_szShortFirstLieutenantRank_0049a70c[8] = "1ST LT";
const char g_szShortCaptainRank_0049a714[8] = "CAPT.";
const char g_szShortMajorRank_0049a71c[8] = "MAJOR";
const char g_szShortLieutenantColonelRank_0049a724[8] = "LT COL";
const char g_szDefaultPilotName_0049ab4c[8] = "Blair";
const char g_szPilotNameLabel_00469ec8_WC1_UNMAPPED /* no-address */[12] = "LAST NAME: ";
const char g_szDefaultCallsign_0049ab54[12] = "Maverick";
const char g_szCallsignLabel_00469ee0_WC1_UNMAPPED /* no-address */[12] = "CALLSIGN : ";
const char g_szCheaterCallsign_00469eec_WC1_UNMAPPED /* no-address */[8] = "CHEATER";
const char g_szHighScoreCongratulations_00469ef4_WC1_UNMAPPED /* no-address */[68] =
    "*******\n"
    "CONGRATULATIONS!\n"
    "YOU HAVE SCORE NUMBER\n"
    ">>>> %d <<<<\n"
    "*******";
const char g_szLowScoreMessage_00469f38_WC1_UNMAPPED /* no-address */[56] =
    "> SORRY <\n\n"
    "YOUR SCORE IS ONLY\n"
    "%ld0\n\n"
    "PLEASE PLAY AGAIN!";
const char g_szHighScoresHeading_00469f70_WC1_UNMAPPED /* no-address */[16] = "%JHIGH SCORES%P";
const char g_szHighScoreNumberFormat_00469f80_WC1_UNMAPPED /* no-address */[8] = "%0ld";
const char g_szHighScoreRowFormat_00469f88_WC1_UNMAPPED /* no-address */[16] =
    "%X%Y%d. %s%X%s0";
const char g_szSelectEnemy_00469f98_WC1_UNMAPPED /* no-address */[16] = "SELECT\nENEMY%P";
int g_bPauseInputActive_0049ac9c;
const char g_szAllocateBufferTag_0049ae20[4] = "AB";
short g_bTargetLockActive_0049ae80;
short g_nCommDeathSequenceFrame_0049ae84;
short g_nWeaponDisplayOffsetX_0049ae8c;
ShortPoint g_aCockpitDamagePositions_0049ae98[4];
short g_asCockpitLightX_0049aea8[7];
short g_asCockpitLightY_0049aeb8[7];
signed char g_acCockpitLightOffFrame_0049aec8[7];
signed char g_acCockpitLightOnFrame_0049aed0[7];
CockpitBarDefinition g_aCockpitBarDefinitions_0049aed8[8];
ShortPoint g_aCockpitReadoutOrigins_0049af58[4];
ShortRect g_stLeftVduBounds_0049af68;
ShortRect g_stRightVduBounds_0049af70;
CockpitScannerGeometry g_stCockpitScannerGeometry_0049af78;
ShortRect g_stPilotHandBounds_0049af88;
ShortPoint g_stPilotHandOrigin_0049af90;
signed char g_cHazardBaseTravelTime_0049af94;
short DAT_0049af98;
ShortPoint g_aDefaultWeaponDisplayPositions_0049afa0[16];
signed char g_bTargetLockDisplayEnabled_0049afe0;
short g_nTargetLeadIndicatorX_0049afe8 = (short)0x8001;
unsigned char *g_pPendingCockpitDamageShape_0049b03c;
unsigned char *g_pCockpitExplosionBackground_0049b040;
unsigned char *g_pCockpitHudBackground_0049b044;
unsigned char * volatile g_pCockpitExplosionShape_0049b048;
volatile short g_nCockpitExplosionFrame_0049b04c = 8;
char *g_pszAutopilotWaitReason_0049b050 =
    g_szAutopilotWaitReason_0049b2f0;
short g_bFuelGaugeDamaged_0049b054;
short g_bForceCriticalDamageWarning_0049b058;
unsigned char *g_pReleaseWeaponDisplayBackground_0049b05c;
signed char g_cReleaseWeaponDisplayFrame_0049b060 = -1;
signed char g_cReleaseWeaponDisplayTicks_0049b064;
signed char g_cReleaseWeaponDisplayState_0049b068;
unsigned char g_bTargetBracketVisible_0049b284 = 1;
ShortRect g_stTargetBracketBounds_0049b2a0 = {-0x7fff, 0, 0, 0};
ShortRect g_stCommTargetBracketBounds_0049b2a8 = {-0x7fff, 0, 0, 0};
short g_bMissileLockAcquired_0049b2b0;
short g_bCockpitDamageFrame0Shown_0049b2b4;
short g_bCockpitDamageFrame2Shown_0049b2b8;
Viewport g_stTrainSimRightVduSource_0049b2c0;
Viewport g_stTrainSimLeftVduSource_0049b2d8;
char g_szAutopilotWaitReason_0049b2f0[28] =
    "Wait for ??????????????????";
char g_szTargetOutOfRange_0049b438[13] = "OUT OF RANGE";
char g_szIttsEngaged_0049b448[13] = "ITTS ENGAGED";
unsigned short g_wPacketCompressionFormatFlags_0049b46c;
const ShortRect g_aTargetArmorClipRects_0049b470[4] = {
    { 12, -20, 29, 20 },
    { -11, 1, 11, 20 },
    { -11, -20, 11, 0 },
    { -29, -20, -12, 20 }
};
char g_szEmptyTargetDisplayText_0049b490[4] = "";
const char g_szLockedTarget_0049b494[20] = "   LOCKED TARGET";
const char g_szTextColourStringColourFormat_0049b4a8[8] = "%F%s%F";
const char g_szAutoTargetting_0049b4b0[20] = "  AUTO TARGETTING";
const char g_szTextColourStringFormat_0049b4c4[8] = "%F%s";
const char g_szTargetLabel_0049b4cc[12] = "\nTarget:";
const char g_szNoTarget_0049b4d8[8] = " None";
const char g_szShipTargetNameFormat_0049b4ec[4] = " %s";
const char g_szRangeLabel_0049b4f0[12] = "\nRange : ";
const char g_szTargetTooFar_0049b4fc[8] = "TOO FAR";
const char g_szRangeMetresSuffix_0049b504[4] = " m";
const char g_szTargetOffscreenRange_0049b508[8] = "----- m";
unsigned char g_abManeuverRerollChance_0049b538[50] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 3, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0,
    0, 0, 0
};
/* perform_maneuver (0x4426AF) dispatches through this table; its guard admits
 * maneuvers 0 through 0x31, and "Maneuver error %d" follows at 0x49B638. */
void (*g_apManeuverHandlers_0049b570[50])(short, short) = {
    /* 00 */ (void (*)(short, short))Mnone,
    /* 01 */ (void (*)(short, short))Mnone,
    /* 02 */ (void (*)(short, short))Mveer_away,
    /* 03 */ (void (*)(short, short))ShipAiState3,
    /* 04 */ (void (*)(short, short))Mfull_ahead,
    /* 05 */ (void (*)(short, short))Mthink,
    /* 06 */ (void (*)(short, short))Mram_missile,
    /* 07 */ (void (*)(short, short))Mturn_n_kick,
    /* 08 */ (void (*)(short, short))Mtight_loop,
    /* 09 */ (void (*)(short, short))Mhard_break,
    /* 10 */ (void (*)(short, short))Msit_n_spin,
    /* 11 */ (void (*)(short, short))Mturn_n_spin,
    /* 12 */ (void (*)(short, short))Mburnout,
    /* 13 */ (void (*)(short, short))Mwabble,
    /* 14 */ (void (*)(short, short))Mroll_over,
    /* 15 */ (void (*)(short, short))Mhard_turn,
    /* 16 */ (void (*)(short, short))Mfish_hook,
    /* 17 */ (void (*)(short, short))Msplit_left,
    /* 18 */ (void (*)(short, short))Msit_n_fire,
    /* 19 */ (void (*)(short, short))Mkickit,
    /* 20 */ (void (*)(short, short))Mturn_n_kick,
    /* 21 */ (void (*)(short, short))Mrout_me,
    /* 22 */ (void (*)(short, short))Mdrop_a_mine,
    /* 23 */ (void (*)(short, short))Msplit_right,
    /* 24 */ (void (*)(short, short))Mzig_zag,
    /* 25 */ (void (*)(short, short))Mgloat,
    /* 26 */ (void (*)(short, short))Mtail_fire,
    /* 27 */ (void (*)(short, short))Mtarget_laser,
    /* 28 */ (void (*)(short, short))ShipAiState28,
    /* 29 */ (void (*)(short, short))Mstrafe_enemy,
    /* 30 */ (void (*)(short, short))Mbest_strafe,
    /* 31 */ (void (*)(short, short))Mstrafe_n_roll,
    /* 32 */ (void (*)(short, short))Mkill_missile,
    /* 33 */ (void (*)(short, short))Msuicide_run,
    /* 34 */ (void (*)(short, short))Mzig_zag_pitch,
    /* 35 */ (void (*)(short, short))ShipAiState35,
    /* 36 */ (void (*)(short, short))Mtarget_missile,
    /* 37 */ (void (*)(short, short))Mget_distance,
    /* 38 */ (void (*)(short, short))Mcorkscrew,
    /* 39 */ (void (*)(short, short))Mtry2tail,
    /* 40 */ (void (*)(short, short))Mzip_past,
    /* 41 */ (void (*)(short, short))Mbuzz_debris,
    /* 42 */ (void (*)(short, short))Mline_up_drop,
    /* 43 */ (void (*)(short, short))Mchill,
    /* 44 */ (void (*)(short, short))ShipAiState44,
    /* 45 */ (void (*)(short, short))ShipAiState45,
    /* 46 */ (void (*)(short, short))ShipAiState46,
    /* 47 */ (void (*)(short, short))ShipAiState47,
    /* 48 */ (void (*)(short, short))Mbest_strafe,
    /* 49 */ (void (*)(short, short))Mreset
};
short g_nEnemySighting_0049b670 = 0x7fff;
short g_asShipIntelState_0049b678[10];
short g_bMissionDeathSequencePending_0049b720;
short g_bMissionEjectionPodSpawned_0049b724;
char g_szEndInflightReplay_0049b738[20] = "END INFLIGHT REPLAY";
const char g_szCannedSceneReadOpenError_0049b750[4] = "026";
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
const char g_szCommSpeakerTextFormat_0049b888[8] = "%s: %Fs";
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
const short g_anPilotTurnInterval_0049b9c0[8] = {
    5, 5, 4, 4, 3, 1, 1, 0
};
unsigned short g_wSpeechCacheBlockBytes_0049bb58 = 0x220;
unsigned short g_wSpeechCacheSizeCode_0049bb5c = 7;
unsigned short g_wSpeechCacheState_0049bb60;
unsigned short g_wSpeechCacheSegment_0049bb64;
unsigned short g_wSpeechCacheEndSegment_0049bb68;
unsigned char *g_pNavLocationText_0049bc54;
const char g_szBriefingMapTextFormat_0049bd30[12] = "%X%Y%F%s";
const char g_szCloseLookTextFormat_0046566c_WC1_UNMAPPED /* no-address */[12] = "%X%Y%F%s%P";
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
const char g_szSfxWaveFormat_0049c22c[12] = "sfx%02i.wav";
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
unsigned int g_dwModalBoundsTopLeft_0049ca48 = 0x00280018;
unsigned int g_dwModalBoundsBottomRight_0049ca4c = 0x003c0128;
ModalTextPanel *g_pModalTextPanel_0049ca50;
unsigned char *g_pCampaignChalkboardShape_0049ca54;
short g_bDisableChalkboardReplay_0049ca58;
short g_nPacketError_0049ca90;
const short g_asConstellationVelocity_0049cae0[16] = {
    8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1
};
const short g_asConstellationFrame_0049cb00[16] = {
    0, 16, 16, 0, 4, 4, 20, 20, 24, 8, 8, 24, 28, 12, 12, 28
};
short g_nConstellationDirection_0049cb20 = -1;
unsigned char *g_pPacketDecompressionWorkspace_0046a91c_WC1_UNMAPPED /* no-address */;
unsigned short g_wPacketDecompressionInputSizeOverride_0046a920_WC1_UNMAPPED /* no-address */;
int g_nDebugOverlayConsoleCount_0049cb24;
DWORD g_dwDebugOverlayKey_0049cb28;
DWORD g_dwDebugOverlayKeyLatch_0049cb2c;
HHOOK g_hDebugKeyboardHook_0049cb30;
const char g_szDebugOverlayFontName_0049cb34[8] = "Courier";
const char g_szDebugOverlaySpinner_0049cb3c[5] = "-\\|/";
const char g_szDebugOverlayNewline_0049cb44[4] = "\n";
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
int g_nMinimumMemoryNoMusic_0049cc20 = 0x43238;
int g_nMinimumMemoryFullMusic_0049cc24 = 0x49bb0;
int g_nMinimumMemoryAlternate_0049cc28 = 0x43238;
short g_bSpeechResourcesEnabled_0049cc2c;
unsigned char g_abMusicResourcePresent_0049cc30[66] = {
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
const ScreenViewportGeometry g_aScreenViewportGeometry_0049d4e8[6] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 320, 128, 0, 24, 0, 24, -24576, -1 },
    { 320, 200, 0, 0, 0, 0, -1536, -1 }
};
const signed char g_acDirectionShapeFrame_0049d558[
    WC1_DIRECTION_VIEW_COUNT * WC1_DIRECTION_SHAPE_TABLE_COUNT] = {
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
const signed char g_acDirectionShapeFlip_0049d618[
    WC1_DIRECTION_VIEW_COUNT * WC1_DIRECTION_SHAPE_TABLE_COUNT] = {
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
short g_nTrainSimMission_00469e30_WC1_UNMAPPED /* no-address */;
short g_nArcadeWave_00469e34_WC1_UNMAPPED /* no-address */;
unsigned char g_aszBuiltInHighScores_00469e38_WC1_UNMAPPED /* no-address */[48] =
    "BISHOP\0\0"
    "GOBLIN\0\0"
    "JEFFTEP\0"
    "MANGLER\0"
    "THE MAN\0"
    "MONGO\0\0\0";
unsigned int g_dwTrainSimStringPadding_00469e68_WC1_UNMAPPED /* no-address */;
const char g_szTextFlushToken_00469e6c_WC1_UNMAPPED /* no-address */[4] = "%P";
const char g_szNewPilotPrompt_00469e70_WC1_UNMAPPED /* no-address */[80] =
    "CONGRATULATIONS!\n"
    "YOU HAVE A TOP SCORE!\n"
    "PLEASE ENTER YOUR\n"
    "NAME AND CALLSIGN:\n";
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
short g_nArcadeBonusCountdown_0046a014_WC1_UNMAPPED /* no-address */;
int g_bMouseAfterburnerControl_0046a02c_WC1_UNMAPPED /* no-address */;
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
const short g_asMouseYawThresholds_0049d7d8[6] = {
    10, 37, 52, 57, 62, 1070
};
const short g_asMousePitchThresholds_0049d7e8[6] = {
    5, 18, 27, 35, 38, 1040
};
signed char g_bSecondaryMouseButtonHeld_0049d7f4;
signed char g_bMouseMoveEventQueued_0049d7fc;
short g_nMouseYawInput_0049d800;
short g_nMousePitchInput_0049d804;
const short g_aSpaceExplosionFlashPalette_0049d808[13][3] = {
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
const char g_szSpaceBuffer_0049d978[16] = "SPACE BUFFER";
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
int DAT_00598888;
int DAT_0059888c;
int DAT_00598890;
Viewport g_stRoomScreenViewport_005988a0;
short g_nSavedRoomControllerX_005988b4;
unsigned char *g_pRecRoomRoster_005988b8;
unsigned char *g_pMissionCampaignData_005988bc;
unsigned char *g_apRecRoomCharacterShapes_005988c0[3];
PilotRecord g_aPilotRecords_005988d0[9];
ConstellationObjectDefinition *g_pConstellationDefinitions_00598a28;
PilotRecord *g_apWingmanPilots_00598a30[8];
unsigned char *g_pRecRoomBackgroundShape_00598a50;
Viewport g_stRoomDisplayViewport_00598a60;
Viewport g_stRoomMouseViewport_00598a80;
unsigned char *g_apRecRoomTextData_00598aa0[3];
short g_nRoomMenuCursorFrame_00598ab0;
TitleMenuRegion *g_pRoomMenuRegions_00598ab2;
char **g_ppszRoomMenuLabels_00598ab6;
const char *g_pszCurrentRoomMenuLabel_00598aba;
TextContext g_stRoomMenuTextContext_00598abe;
unsigned char *g_apRecRoomSceneData_00598ae0[3];
unsigned char *g_pBriefingPacket_00598aec;
unsigned char *g_pBriefingTextData_00598af0;
short *g_pMouthAnimationCommands_00598af4;
unsigned char *g_pMedalTextData_00598af8;
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
unsigned char *g_pMedalSceneData_00598c20;
unsigned char *g_pBriefingPortraitShape_00598c24;
unsigned char *g_pDebriefingTextData_00598c28;
unsigned char *g_pBriefingCloseupShape_00598c2c;
unsigned char *g_pConversationOverlayShape_00598c30;
unsigned char g_bFilteredKeyWaitStarted_00598c34;
int g_nMouseCursorSavedY_0059a840;
int g_nMouseCursorSavedX_0059a844;
unsigned char g_bInputMode_0059a848;
unsigned char g_bFilteredKeyWaitActive_0059a849;
int DAT_0059a84c;
short DAT_0059a856;
short g_nPacketDecompressSourceFile_0059a858;
int DAT_0059a8e0;
int DAT_0059a8e4;
short DAT_0059a9f0;
short g_nPacketDecompressInputPosition_0059ab00;
void *g_pPacketDecompressInput_0059ab04;
unsigned short g_wCurrentInputModifiers_0059ab08;
MouseCursorState g_stMouseCursorState_0059ab10;
void (*DAT_0059ab2c)(void);
int g_nPacketDecompressResult_0059ab30;
short DAT_0059ab34;
short g_nPacketDecompressPending_0059ab36;
unsigned short g_wPacketDecompressInputSize_0059ab38;
short g_nPacketDecompressWorkspaceSegment_0059ab3a;
int DAT_0059ab40;
int DAT_0059ab44;
int DAT_0059ab48;
int DAT_0059ab4c;
unsigned int DAT_0059ab54;
int DAT_0059ab5c;
int DAT_0059ab60;
short DAT_0059ab64;
MouseCursorState g_stHostMouseState_0059af70;
FixedVector g_aObjectViewPosition_0059afa0[WC2_SPACE_OBJECT_COUNT];
signed char DAT_0059c910[16];
CampaignState g_stCampaignState_0059ca50;
signed char g_acShipRating_0059cd80[16];
unsigned int DAT_0059ce18[2];
unsigned char DAT_0059cf20[0x10];
short g_asPreviousObjectDistance_0059d080[WC2_SPACE_OBJECT_COUNT];
short g_asCollisionCountdown_0059d2d0[16];
short g_asCannedCommand_0059d4e0[16];
signed char g_acShipPointingMode_0059d790[16];
signed char g_cCockpitView_0059dab0;
const short *g_apCannedSequence_0059dce0[12];
signed char g_acShipSpawnNavPoint_0059ded0[16];
short g_asShipAccumulatedDamage_0059dee0[WC2_SPACE_OBJECT_COUNT];
unsigned short g_awPaletteEntryAllocation_0059df80[256];
unsigned char *g_apCommPortraitShapes_0059e180[16];
char g_szHudMessageBuffer_0059e1c0[0x60];
char *g_aapszPilotSpeech_0059e220[14][11];
unsigned char *g_pFireworkShape_005a6a68;
ConstellationStar g_aConstellationStars_005a6a70[10];
short g_nConstellationStarCount_005a6ab0;
Viewport g_stConstellationViewport_005a6b40;
const short *g_pViewScript_005a6b58;
Viewport DAT_005a6b80;
ScreenViewportPacket *g_pScreenViewportPacket_005a6b94;
Viewport DAT_005a6be0;
FontWorkspace **g_apFontWorkspaces_005a6c10[4];
TextContext DAT_005a74f0;
Viewport DAT_005a7530;
unsigned char *g_pCommMenuCursorShape_005a7660;
Viewport DAT_005a7690;
unsigned char *g_pCockpitDamageShape_005a76f4;
TextContext DAT_005a7700;
TextContext DAT_005a7720;
unsigned char *g_pLaunchDoorShape_005a77e8;
unsigned char g_abPaletteTriplets_005a77f0[256][3];
short g_bMouseButtonEventQueued_005a7afc;
short g_bKeyboardEventQueued_005a7afe;
short g_bJoystickEventQueued_005a7b88;
short g_nArcadeStartupParameter_005a7b8a;
Viewport g_stTrainSimTitleDisplayViewport_005a7b90;
Viewport g_stTrainSimHighScoreBufferViewport_005a7bb0;
int g_nArcadeScore_005a7bc4;
TextContext g_stTrainSimTextContext_005a7bd0;
Viewport g_stTrainSimHighScoreDisplayViewport_005a7bf0;
TextContext g_stTrainSimHighScoreTextContext_005a7c10;
short g_nArcadeTimeRemaining_005a7c2c;
HighScoreEntry g_aHighScoreEntries_005a7c30[6];
int g_nArcadeWaveBonus_005a7c50;
unsigned char *g_pSceneAnimationSceneData_005a7c54;
unsigned char *g_pSceneAnimationPrimaryShape_005a7c58;
unsigned char *g_pSceneAnimationTextData_005a7c5c;
unsigned char *g_pSceneAnimationPacket_005a7c60;
SceneAnimationObject *g_pSceneAnimationObjects_005a7c64;
short g_nSceneAnimationWaitFrames_005a7c68;
unsigned char *g_pSceneAnimationDefinitions_005a7c6c;
unsigned char *g_pSceneAnimationSecondaryShape_005a7c70;
signed char g_cCockpitLogicalFile_005a7c74;
unsigned char *g_apCockpitShapes_005a7c80[4];
unsigned char *g_pCinematicViewBackdrop_005a7c90;
unsigned char *g_pRearViewBackdrop_005a7c94;
short g_asCollisionTime_005a7ca0[10];
short g_nWingmanKillCount_005a7cb8;
short g_asCollisionPartner_005a7cc0[10];
unsigned int g_dwOriginalFreeMemory_005a7cd8;
unsigned char *g_pMouseCursorResource_005a7cdc;
int g_nSceneResourceBudget_005a7ce4;
int g_nAfterburnerSoundDeadline_005a7ce8;
DiskFileRecord *g_pDiskFileRecords_005a7cf0;
Viewport g_stDiskPromptBackgroundViewport_005a7d00;
unsigned char g_abDiskPromptDriveState_005a7d20[2];
Viewport g_stDiskPromptViewport_005a7d40;
TextContext g_stDiskPromptTextContext_005a7d60;
unsigned int g_dwDiskPromptTopLeft_005a7d80;
unsigned int g_dwDiskPromptBottomRight_005a7d84;
int DAT_005a7d9c;
signed char g_cCapitalShipLogicalFile_005a7da0;
enum ObjectType g_eReleaseWeaponDisplayType_005a7dc0;
int g_nSavedMouseCursorY_005a7df4;
int g_nSavedMouseCursorX_005a7df8;
unsigned char *g_pTitleShape_005a7f08;
short DAT_005a8114;
short g_nMissionMedalScore_005a8116;
short g_nCampaignDataSet_005a8118;
short g_nMenuInputRepeatDelay_005a8208;
short DAT_005a86a6;
CampaignDate *g_pCurrentCampaignDate_005a86a8;
signed char g_cObjectResourceLogicalFile_005a86b0;
Viewport *g_pScrambleViewport_005a86b4;
short g_nScrambleCanopyFrame_005a86b8;
unsigned char *g_pScrambleDetailShape_005a86bc;
short g_nScrambleCockpitScale_005a86c0;
short g_nScrambleCockpitDetailX_005a86c4;
short g_nScrambleCockpitDetailY_005a86c6;
short g_nFuneralCasketX_005a86c8;
short g_nFuneralCasketY_005a86ca;
short g_nScrambleShipDetailCount_005a86cc;
signed char g_acScrambleShipDetailIndices_005a86d0[24];
signed char g_cScrambleWalkTicks_005a86e8;
unsigned char *g_pConversationSpecialShape_005a86ec;
ShortPoint g_aFuneralParticles_005a86f0[7];
unsigned char *g_pScrambleBackgroundShape_005a870c;
short g_nFuneralParticleDistance_005a8710;
short g_nScrambleBackgroundY_005a8712;
short g_nScrambleBackgroundRightX_005a8714;
short g_nFuneralForegroundX_005a8718;
short g_nFuneralForegroundY_005a871a;
short g_nFuneralMainScale_005a871c;
short g_nFuneralRifleFrame_005a871e;
short g_nFuneralBaseX_005a8720;
short g_nFuneralBaseY_005a8722;
short g_nScrambleShipX_005a8724;
short g_nScrambleShipY_005a8726;
short g_nFuneralParticleScale_005a8728;
unsigned char *g_pScrambleHangarShape_005a872c;
unsigned char *g_pScrambleCockpitShape_005a8730;
short g_nScrambleWalkerY_005a8734;
short g_nScrambleCanopyOffset_005a8736;
short g_nFuneralMainDistance_005a8738;
signed char g_bScrambleCanopyClosed_005a873a;
short g_nFuneralGuardFrame_005a873c;
short g_nScrambleOverlayX_005a8740;
short g_nScrambleOverlayY_005a8742;
unsigned char *g_pScrambleOverlayShape_005a8744;
signed char g_cScrambleWalkerPair_005a8748;
unsigned char *g_pScrambleCanopyShape_005a874c;
unsigned char *g_pScrambleShipShape_005a8750;
short g_nTalkingHeadFaceX_005a8754;
short g_nTalkingHeadFaceY_005a8756;
short g_nTalkingHeadMouthY_005a8758;
short g_nTalkingHeadMouthX_005a875a;
char DAT_005a8760[512];
unsigned char *g_pIntroFont_005a8960;
HCURSOR DAT_005a8a34;
unsigned int DAT_005a8a44;
unsigned short DAT_005a8a50[0x300];
int g_nStreamerAudioPlaying_005b2810;
char g_szStreamerPath_005b2818[128];
char g_szWingCommanderOneGameDataPath_005b2898[0x400];
int g_bMonoDebugInstalled_005b30e8;
HANDLE g_hMonoDebugDevice_005b30ec;
short DAT_005b30f0;
unsigned char g_bCurrentManeuverReroll_005b30f4;
NavMapLabel g_aNavMapLabels_00475e80_WC1_UNMAPPED /* no-address */[20];
unsigned char g_abMouseCursorBackground_00475ff0_WC1_UNMAPPED /* no-address */[0x400];
int g_nFlightFrameTotalTicks_004764f8_WC1_UNMAPPED /* no-address */;
int g_nFlightCockpitTicks_004764fc_WC1_UNMAPPED /* no-address */;
LARGE_INTEGER g_liFlightAfterInput_00476500_WC1_UNMAPPED /* no-address */;
LARGE_INTEGER g_liFlightFrameEnd_00476508_WC1_UNMAPPED /* no-address */;
int g_nFlightPresentTicks_00476510_WC1_UNMAPPED /* no-address */;
LARGE_INTEGER g_liFlightFrameStart_00476518_WC1_UNMAPPED /* no-address */;
LARGE_INTEGER g_liFlightAfterSimulation_00476520_WC1_UNMAPPED /* no-address */;
int g_nFlightSimulationTicks_00476528_WC1_UNMAPPED /* no-address */;
LARGE_INTEGER g_liFlightAfterCockpit_00476530_WC1_UNMAPPED /* no-address */;
int g_nFlightInputTicks_00476538_WC1_UNMAPPED /* no-address */;
LARGE_INTEGER g_liFlightAfterRender_00476540_WC1_UNMAPPED /* no-address */;
int g_nFlightRenderTicks_00476548_WC1_UNMAPPED /* no-address */;
unsigned char DAT_00476620_WC1_UNMAPPED /* no-address */[32];
float DAT_00486510_WC1_UNMAPPED /* no-address */;
unsigned char DAT_004865a8_WC1_UNMAPPED /* no-address */[0x1000];
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
ShortRect g_aNavMapExclusionRects_005b35a8[20];
/* The button mask KeyboardMousePump last posted. */
int g_nKeyboardMouseButtons_005b3648;
char g_szSfxWavePath_005b3650[0x40];
int g_nInsertKeyState_005b3690;
int g_nClearedKeyboardState_005b3694;
char g_szStreamsPath_00475c18_WC1_UNMAPPED /* no-address */[0x100];
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
int g_anObjectDepthPlaced_005c8180[WC2_SPACE_OBJECT_COUNT];
signed char g_cScreenViewportMode_005c82a6;
signed char g_bCinematicSpriteFontEnabled_005c82a7;
const ScreenViewportGeometry * volatile
    g_pScreenViewportGeometry_005c82b0;
int g_anSortedObject_005c82c0[WC2_SPACE_OBJECT_COUNT];
unsigned char *g_pActiveCutscenePixels_005c83dc;
unsigned char g_nInputDeviceMode_005c83e6;
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
signed char DAT_005c8430;
short g_wInputDeviceButtonState_005c8432;
int g_bSuppressNextMouseMove_005c843c;
short g_nInputTickInterval_005c8448;
volatile int g_nFrameTimerPending_005c844c;
int g_bInputCursorDirty_005c8450;
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
short g_asGunCooldown_005c8d70[30];
short g_asTargetCameraPitchIndicatorBackground_005c8db0[3];
int g_nTargetCameraOverlayMode_005c8db8;
short g_nGunDisplayCount_005c8dbc;
short g_bMissionWingmanFlag_005c8dbe;
short g_nGunDisplayIndex_005c8dc0;
int g_nAllocateViewportCalls_005c8dc4;
int g_nMemoryConfiguration_005c8dc8;
int g_nMusicMemoryRequirement_005c8dcc;
unsigned int g_dwInitialFreeMemory_005c8dd0;
short g_nSpeechCacheSizeCode_005c8dd8;
short g_nMemoryAdjustmentKb_005c8dda;
short g_bPumpMessagesWhileLoading_005c8ddc;
short g_nSpeechCacheUnitCount_005c8dde;
int g_nAvailableGameMemory_005c8de0;
int g_nMissionResourceBudget_005c8de4;
short g_bSpeechCacheEnabled_005c8de8;
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
unsigned long g_dwGameClockStart_005d12b8;
int g_aiSoundEffectSourceActive_005d12c0[WC2_SPACE_OBJECT_COUNT];
MusicResource g_aMusicResources_005d13e0[66];
unsigned int g_dwRestoredSceneMusicState_005d14e8;
int g_nPassingShipSoundCooldown_005d156c;
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
void *g_pSpeechCacheDataAllocation_005d1718;
void *g_pSpeechCacheCode_005d1720;
unsigned char g_ucSpeechCachePage_005d1724;
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
short g_nReleaseWeaponDisplayX_005d1c24;
short g_nReleaseWeaponDisplayY_005d1c26;
short g_nReleaseWeaponDisplayType_005d1c28;
short g_nTargetLeadIndicatorY_005d1c2a;
unsigned char *g_pScannerMarkerBackground_005d1c2c;
unsigned char g_cPilotHandFrame_005d1c30;
short g_nHudMessageTime_005d1c32;
short g_nPendingCockpitDamage_005d1c34;
char g_szTextScratchBuffer_005d1c40[256];
HudMessageSlot g_aHudMessageSlots_005d1d40[2];
short g_asScannerTargetBackground_005d1d80[5];
signed char g_cPreviousTargetObject_005d1d8e;
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
short g_nWeaponDisplayOffsetY_0049ae8e_WC1_UNMAPPED /* no-address */ = 0x10;
Viewport g_stScreenViewport_005d21a0;
TextContext g_stSpaceTextContext_005d21c0;
Viewport g_stCockpitBarViewport_005d21e0;
unsigned char *g_pCockpitHudShape_005d21f4;
unsigned char *g_apTextFonts_005d2200[4];
FixedVector g_aDirectionViewRightVector_005d2210[WC1_DIRECTION_VIEW_COUNT];
FixedVector g_aDirectionViewUpVector_005d2500[WC1_DIRECTION_VIEW_COUNT];
FixedVector g_aDirectionViewForwardVector_005d27f0[WC1_DIRECTION_VIEW_COUNT];
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
void *g_pCutscenePalettePacket_005d2e98;
short g_nCutsceneResourceScratch_005d2e9c;
short g_nCutsceneTextIndex_005d2ea4;
unsigned int g_nCutsceneInitialLargestBlock_005d2ea8;
Viewport g_stSceneFlicScratchViewport_005d2eb0;
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
unsigned char g_abJumpDriveUsedBySystem_005d2fe8[5];
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
int DAT_005d3864;
signed char g_abExplosionObjectVisited_005d3870[
    WC2_SPACE_OBJECT_COUNT];
short g_asPlayerDamageCredit_005d38c0[10];
short g_asMissileProximityFuse_005d38e0[16];
short g_asObjectCreationFrame_005d3900[WC2_SPACE_OBJECT_COUNT];
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
unsigned int g_dwHighMemoryParagraph_005d3fb4;
short g_asCannedSceneFrameOffsets_005d3fc0[70];
short g_nCannedSceneFrameCount_005d404c;
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
