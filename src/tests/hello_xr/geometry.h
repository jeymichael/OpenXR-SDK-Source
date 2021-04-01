// Copyright (c) 2017-2021, The Khronos Group Inc
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace Geometry {

struct Vertex {
    XrVector3f Position;
    XrVector3f Color;
};

constexpr XrVector3f Red{1, 0, 0};
constexpr XrVector3f DarkRed{0.25f, 0, 0};
constexpr XrVector3f Green{0, 1, 0};
constexpr XrVector3f DarkGreen{0, 0.25f, 0};
constexpr XrVector3f Blue{0, 0, 1};
constexpr XrVector3f DarkBlue{0, 0, 0.25f};

// Vertices for a 1x1x1 meter cube. (Left/Right, Top/Bottom, Front/Back)
constexpr XrVector3f LBB{-0.5f, -0.5f, -0.5f};
constexpr XrVector3f LBF{-0.5f, -0.5f, 0.5f};
constexpr XrVector3f LTB{-0.5f, 0.5f, -0.5f};
constexpr XrVector3f LTF{-0.5f, 0.5f, 0.5f};
constexpr XrVector3f RBB{0.5f, -0.5f, -0.5f};
constexpr XrVector3f RBF{0.5f, -0.5f, 0.5f};
constexpr XrVector3f RTB{0.5f, 0.5f, -0.5f};
constexpr XrVector3f RTF{0.5f, 0.5f, 0.5f};
#if 0
#define CUBE_SIDE(V1, V2, V3, V4, V5, V6, COLOR) {V1, COLOR}, {V2, COLOR}, {V3, COLOR}, {V4, COLOR}, {V5, COLOR}, {V6, COLOR},

constexpr Vertex c_cubeVertices[] = {
    CUBE_SIDE(LTB, LBF, LBB, LTB, LTF, LBF, DarkRed)    // -X
    CUBE_SIDE(RTB, RBB, RBF, RTB, RBF, RTF, Red)        // +X
    CUBE_SIDE(LBB, LBF, RBF, LBB, RBF, RBB, DarkGreen)  // -Y
    CUBE_SIDE(LTB, RTB, RTF, LTB, RTF, LTF, Green)      // +Y
    CUBE_SIDE(LBB, RBB, RTB, LBB, RTB, LTB, DarkBlue)   // -Z
    CUBE_SIDE(LBF, LTF, RTF, LBF, RTF, RBF, Blue)       // +Z
};

// Winding order is clockwise. Each side uses a different color.
constexpr unsigned short c_cubeIndices[] = {
    0,  1,  2,  3,  4,  5,   // -X
    6,  7,  8,  9,  10, 11,  // +X
    12, 13, 14, 15, 16, 17,  // -Y
    18, 19, 20, 21, 22, 23,  // +Y
    24, 25, 26, 27, 28, 29,  // -Z
    30, 31, 32, 33, 34, 35,  // +Z
};
#endif

constexpr XrVector3f Grey{0.5f, 0.5f, 0.5f};
constexpr XrVector3f Yellow{1, 1, 0};
constexpr XrVector3f White{1, 1, 1};
constexpr XrVector3f Orange{1, 0.64f, 0};

#define W_MAJOR 1.0f
#define W_MINOR (W_MAJOR)/3.0f
#define OFF_MINOR (W_MINOR)/2.0f
#define SCALE_FACTOR 0.95f

#define CUBE_SIDE_L(x1, x2, y1, y2, z1, z2, COLOR)          \
    {{x1, y2, z1}, COLOR}, {{x1, y1, z2}, COLOR}, {{x1, y1, z1}, COLOR}, {{x1, y2, z1}, COLOR}, {{x1, y2, z2}, COLOR}, {{x1, y1, z2}, COLOR},
#define CUBE_SIDE_R(x1, x2, y1, y2, z1, z2, COLOR)          \
    {{x2, y2, z1}, COLOR}, {{x2, y1, z1}, COLOR}, {{x2, y1, z2}, COLOR}, {{x2, y2, z1}, COLOR}, {{x2, y1, z2}, COLOR}, {{x2, y2, z2}, COLOR},
#define CUBE_SIDE_B(x1, x2, y1, y2, z1, z2, COLOR)          \
    {{x1, y1, z1}, COLOR}, {{x1, y1, z2}, COLOR}, {{x2, y1, z2}, COLOR}, {{x1, y1, z1}, COLOR}, {{x2, y1, z2}, COLOR}, {{x2, y1, z1}, COLOR},
#define CUBE_SIDE_T(x1, x2, y1, y2, z1, z2, COLOR)          \
    {{x1, y2, z1}, COLOR}, {{x2, y2, z1}, COLOR}, {{x2, y2, z2}, COLOR}, {{x1, y2, z1}, COLOR}, {{x2, y2, z2}, COLOR}, {{x1, y2, z2}, COLOR},
#define CUBE_SIDE_K(x1, x2, y1, y2, z1, z2, COLOR)          \
    {{x1, y1, z1}, COLOR}, {{x2, y1, z1}, COLOR}, {{x2, y2, z1}, COLOR}, {{x1, y1, z1}, COLOR}, {{x2, y2, z1}, COLOR}, {{x1, y2, z1}, COLOR},
#define CUBE_SIDE_F(x1, x2, y1, y2, z1, z2, COLOR)          \
    {{x1, y1, z2}, COLOR}, {{x1, y2, z2}, COLOR}, {{x2, y2, z2}, COLOR}, {{x1, y1, z2}, COLOR}, {{x2, y2, z2}, COLOR}, {{x2, y1, z2}, COLOR},

#define SUB_CUBE(x1, x2, y1, y2, z1, z2, COLOR_L, COLOR_R, COLOR_B, COLOR_T, COLOR_K, COLOR_F) \
    CUBE_SIDE_L(x1, x2, y1, y2, z1, z2, COLOR_L) \
    CUBE_SIDE_R(x1, x2, y1, y2, z1, z2, COLOR_R) \
    CUBE_SIDE_B(x1, x2, y1, y2, z1, z2, COLOR_B) \
    CUBE_SIDE_T(x1, x2, y1, y2, z1, z2, COLOR_T) \
    CUBE_SIDE_K(x1, x2, y1, y2, z1, z2, COLOR_K) \
    CUBE_SIDE_F(x1, x2, y1, y2, z1, z2, COLOR_F)

constexpr Vertex c_cubeVertices[] = {
        // 6 AXIS Sub-Cubes Left(White), Right(Yellow), Bottom(Green), Top(Blue), Back(Orange), Front(Red)
        // For White sub-cube at (-X, 0, 0)
        SUB_CUBE(-2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR, White, Grey, Grey, Grey, Grey, Grey)
        // For Yellow sub-cube at (+X, 0, 0)
        SUB_CUBE( 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR, Grey, Yellow, Grey, Grey, Grey, Grey)
        // For Green sub-cube at (0, -Y, 0)
        SUB_CUBE( -OFF_MINOR*SCALE_FACTOR,  +OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR, Grey, Grey, Green, Grey, Grey, Grey)
        // For Blue sub-cube at (0, +Y, 0)
        SUB_CUBE( -OFF_MINOR*SCALE_FACTOR,  +OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR, Grey, Grey, Grey, Blue, Grey, Grey)
        // For Orange sub-cube at (0, 0, -Z)
        SUB_CUBE( -OFF_MINOR*SCALE_FACTOR,  +OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, Grey, Grey, Grey, Grey, Orange, Grey)
        // For Red sub-cube at (0, 0, +Z)
        SUB_CUBE( -OFF_MINOR*SCALE_FACTOR,  +OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, Grey, Grey, Grey, Grey, Grey, Red)

        // 12 EDGE Sub-Cubes WG, WB, WO, WR, YG, YB, YO, YR
        // For White Green sub-cube at (-X, -Y, 0)
        SUB_CUBE(-2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR, White, Grey, Green, Grey, Grey, Grey)
        // For White Blue sub-cube at (-X, +Y, 0)
        SUB_CUBE(-2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR, White, Grey, Grey, Blue, Grey, Grey)
        // For White Orange sub-cube at (-X, 0, -Z)
        SUB_CUBE( -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, White, Grey, Grey, Grey, Orange, Grey)
        // For White Red sub-cube at (-X, 0, +Z)
        SUB_CUBE( -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, White, Grey, Grey, Grey, Grey, Red)
        // For Yellow Green sub-cube at (+X, -Y, 0)
        SUB_CUBE(2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR, Grey, Yellow, Green, Grey, Grey, Grey)
        // For Yellow Blue sub-cube at (+X, +Y, 0)
        SUB_CUBE(2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR, Grey, Yellow, Grey, Blue, Grey, Grey)
        // For Yellow Orange sub-cube at (+X, 0, -Z)
        SUB_CUBE( 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, Grey, Yellow, Grey, Grey, Orange, Grey)
        // For Yellow Red sub-cube at (+X, 0, +Z)
        SUB_CUBE( 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -OFF_MINOR*SCALE_FACTOR, +OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, Grey, Yellow, Grey, Grey, Grey, Red)
        // For Green Orange sub-cube at (0, -Y, -Z)
        SUB_CUBE( -OFF_MINOR*SCALE_FACTOR,  +OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, Grey, Grey, Green, Grey, Orange, Grey)
        // For Green Red sub-cube at (0, -Y, +Z)
        SUB_CUBE( -OFF_MINOR*SCALE_FACTOR,  +OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, Grey, Grey, Green, Grey, Grey, Red)
        // For Blue Orange sub-cube at (0, +Y, -Z)
        SUB_CUBE( -OFF_MINOR*SCALE_FACTOR,  +OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, Grey, Grey, Grey, Blue, Orange, Grey)
        // For Blue Red sub-cube at (0, +Y, +Z)
        SUB_CUBE( -OFF_MINOR*SCALE_FACTOR,  +OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, Grey, Grey, Grey, Blue, Grey, Red)

        // TODO: 8 Corner Sub-Cubes WGO, WGR, WBO, WBR, YGO, YGR, YBO, YBR
        // For White Green Orange sub-cube at (-X, -Y, -Z)
        SUB_CUBE( -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, White, Grey, Green, Grey, Orange, Grey)
        // For White Green Red sub-cube at (-X, -Y, +Z)
        SUB_CUBE(-2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, White, Grey, Green, Grey, Grey, Red)
        // For White Blue Orange sub-cube at (-X, +Y, -Z)
        SUB_CUBE( -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, White, Grey, Grey, Blue, Orange, Grey)
        // For White Blue Red sub-cube at (-X, +Y, +Z)
        SUB_CUBE(-2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, White, Grey, Grey, Blue, Grey, Red)
        // For Yellow Green Orange sub-cube at (+X, -Y, -Z)
        SUB_CUBE( 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, Grey, Yellow, Green, Grey, Orange, Grey)
        // For Yellow Green Red sub-cube at (+X, -Y, +Z)
        SUB_CUBE(2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, Grey, Yellow, Green, Grey, Grey, Red)
        // For Yellow Blue Orange sub-cube at (+X, +Y, -Z)
        SUB_CUBE( 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, -2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, Grey, Yellow, Grey, Blue, Orange, Grey)
        // For Yellow Blue Red sub-cube at (+X, +Y, +Z)
        SUB_CUBE(2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR, 2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR-OFF_MINOR*SCALE_FACTOR,  2.0f*OFF_MINOR+OFF_MINOR*SCALE_FACTOR, Grey, Yellow, Grey, Blue, Grey, Red)


};

// Winding order is clockwise. Each side uses a different color.
constexpr unsigned short c_cubeIndices[] = {
    // Begin 6 AXIS Sub-Cubes
    0,  1,  2,  3,  4,  5,   // -X
    6,  7,  8,  9,  10, 11,  // +X
    12, 13, 14, 15, 16, 17,  // -Y
    18, 19, 20, 21, 22, 23,  // +Y
    24, 25, 26, 27, 28, 29,  // -Z
    30, 31, 32, 33, 34, 35,  // +Z

    36, 37, 38, 39, 40, 41,  // -X
    42, 43, 44, 45, 46, 47,  // +X
    48, 49, 50, 51, 52, 53,  // -Y
    54, 55, 56, 57, 58, 59,  // +Y
    60, 61, 62, 63, 64, 65,  // -Z
    66, 67, 68, 69, 70, 71,  // +Z

    72, 73, 74, 75, 76, 77,  // -X
    78, 79, 80, 81, 82, 83,  // +X
    84, 85, 86, 87, 88, 89,  // -Y
    90, 91, 92, 93, 94, 95,  // +Y
    96, 97, 98, 99,100,101,  // -Z
    102,103,104,105,106,107, // +Z

    108,109,110,111,112,113, // -X
    114,115,116,117,118,119, // +X
    120,121,122,123,124,125, // -Y
    126,127,128,129,130,131, // +Y
    132,133,134,135,136,137, // -Z
    138,139,140,141,142,143, // +Z

    144,145,146,147,148,149, // -X
    150,151,152,153,154,155, // +X
    156,157,158,159,160,161, // -Y
    162,163,164,165,166,167, // +Y
    168,169,170,171,172,173, // -Z
    174,175,176,177,178,179, // +Z

    180,181,182,183,184,185, // -X
    186,187,188,189,190,191, // +X
    192,193,194,195,196,197, // -Y
    198,199,200,201,202,203, // +Y
    204,205,206,207,208,209, // -Z
    210,211,212,213,214,215, // +Z
    // End 6 AXIS Sub-Cubes

    // Begin 12 EDGE Sub-Cubes
    216,217,218,219,220,221, // -X
    222,223,224,225,226,227, // +X
    228,229,230,231,232,233, // -Y
    234,235,236,237,238,239, // +Y
    240,241,242,243,244,245, // -Z
    246,247,248,249,250,251, // +Z

    252,253,254,255,256,257, // -X
    258,259,260,261,262,263, // +X
    264,265,266,267,268,269, // -Y
    270,271,272,273,274,275, // +Y
    276,277,278,279,280,281, // -Z
    282,283,284,285,286,287, // +Z

    288,289,290,291,292,293, // -X
    294,295,296,297,298,299, // +X
    300,301,302,303,304,305, // -Y
    306,307,308,309,310,311, // +Y
    312,313,314,315,316,317, // -Z
    318,319,320,321,322,323, // +Z

    324,325,326,327,328,329, // -X
    330,331,332,333,334,335, // +X
    336,337,338,339,340,341, // -Y
    342,343,344,345,346,347, // +Y
    348,349,350,351,352,353, // -Z
    354,355,356,357,358,359, // +Z

    360,361,362,363,364,365, // -X
    366,367,368,369,370,371, // +X
    372,373,374,375,376,377, // -Y
    378,379,380,381,382,383, // +Y
    384,385,386,387,388,389, // -Z
    390,391,392,393,394,395, // +Z

    396,397,398,399,400,401, // -X
    402,403,404,405,406,407, // +X
    408,409,410,411,412,413, // -Y
    414,415,416,417,418,419, // +Y
    420,421,422,423,424,425, // -Z
    426,427,428,429,430,431, // +Z

    432,433,434,435,436,437, // -X
    438,439,440,441,442,443, // +X
    444,445,446,447,448,449, // -Y
    450,451,452,453,454,455, // +Y
    456,457,458,459,460,461, // -Z
    462,463,464,465,466,467, // +Z

    468,469,470,471,472,473, // -X
    474,475,476,477,478,479, // +X
    480,481,482,483,484,485, // -Y
    486,487,488,489,490,491, // +Y
    492,493,494,495,496,497, // -Z
    498,499,500,501,502,503, // +Z

    504,505,506,507,508,509, // -X
    510,511,512,513,514,515, // +X
    516,517,518,519,520,521, // -Y
    522,523,524,525,526,527, // +Y
    528,529,530,531,532,533, // -Z
    534,535,536,537,538,539, // +Z

    540,541,542,543,544,545, // -X
    546,547,548,549,550,551, // +X
    552,553,554,555,556,557, // -Y
    558,559,560,561,562,563, // +Y
    564,565,566,567,568,569, // -Z
    570,571,572,573,574,575, // +Z

    576,577,578,579,580,581, // -X
    582,583,584,585,586,587, // +X
    588,589,590,591,592,593, // -Y
    594,595,596,597,598,599, // +Y
    600,601,602,603,604,605, // -Z
    606,607,608,609,610,611, // +Z

    612,613,614,615,616,617, // -X
    618,619,620,621,622,623, // +X
    624,625,626,627,628,629, // -Y
    630,631,632,633,634,635, // +Y
    636,637,638,639,640,641, // -Z
    642,643,644,645,646,647, // +Z
    // End 12 EDGE Sub-Cubes

    // Begin 8 CORNER Sub-Cubes
    648,649,650,651,652,653, // -X
    654,655,656,657,658,659, // +X
    660,661,662,663,664,665, // -Y
    666,667,668,669,670,671, // +Y
    672,673,674,675,676,677, // -Z
    678,679,680,681,682,683, // +Z

    684,685,686,687,688,689, // -X
    690,691,692,693,694,695, // +X
    696,697,698,699,700,701, // -Y
    702,703,704,705,706,707, // +Y
    708,709,710,711,712,713, // -Z
    714,715,716,717,718,719, // +Z

    720,721,722,723,724,725, // -X
    726,727,728,729,730,731, // +X
    732,733,734,735,736,737, // -Y
    738,739,740,741,742,743, // +Y
    744,745,746,747,748,749, // -Z
    750,751,752,753,754,755, // +Z

    756,757,758,759,760,761, // -X
    762,763,764,765,766,767, // +X
    768,769,770,771,772,773, // -Y
    774,775,776,777,778,779, // +Y
    780,781,782,783,784,785, // -Z
    786,787,788,789,790,791, // +Z

    792,793,794,795,796,797, // -X
    798,799,800,801,802,803, // +X
    804,805,806,807,808,809, // -Y
    810,811,812,813,814,815, // +Y
    816,817,818,819,820,821, // -Z
    822,823,824,825,826,827, // +Z

    828,829,830,831,832,833, // -X
    834,835,836,837,838,839, // +X
    840,841,842,843,844,845, // -Y
    846,847,848,849,850,851, // +Y
    852,853,854,855,856,857, // -Z
    858,859,860,861,862,863, // +Z

    864,865,866,867,868,869, // -X
    870,871,872,873,874,875, // +X
    876,877,878,879,880,881, // -Y
    882,883,884,885,886,887, // +Y
    888,889,890,891,892,893, // -Z
    894,895,896,897,898,899, // +Z

    900,901,902,903,904,905, // -X
    906,907,908,909,910,911, // +X
    912,913,914,915,916,917, // -Y
    918,919,920,921,922,923, // +Y
    924,925,926,927,928,929, // -Z
    930,931,932,933,934,935, // +Z
    // End 8 CORNER Sub-Cubes
    };
}  // namespace Geometry
