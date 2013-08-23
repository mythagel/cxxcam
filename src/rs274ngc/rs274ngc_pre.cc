   /************************************************************************

       Copyright 2008 Mark Pictor

   This file is part of RS274NGC.

   RS274NGC is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   RS274NGC is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with RS274NGC.  If not, see <http://www.gnu.org/licenses/>.

   This software is based on software that was produced by the National
   Institute of Standards and Technology (NIST).

   ************************************************************************/

   /* rs274ngc.cc

   This rs274ngc.cc file contains the source code for (1) the kernel of
   several rs274ngc interpreters and (2) two of the four sets of interface
   functions declared in canon.hh:
   1. interface functions to call to tell the interpreter what to do.
   These all return a status value.
   2. interface functions to call to get information from the interpreter.

   Kernel functions call each other. A few kernel functions are called by
   interface functions.

   Interface function names all begin with "rs274ngc_".

   Error handling is by returning a status value of either a non-error
   code (RS274NGC_OK, RS274NGC_EXIT, etc.) or some specific error code
   from each function where there is a possibility of error.  If an error
   occurs, processing is always stopped, and control is passed back up
   through the function call hierarchy to an interface function; the
   error code is also passed back up. The stack of functions called is
   also recorded. The external program calling an interface function may
   then handle the error further as it sees fit.

   Since returned values are usually used as just described to handle the
   possibility of errors, an alternative method of passing calculated
   values is required. In general, if function A needs a value for
   variable V calculated by function B, this is handled by passing a
   pointer to V from A to B, and B calculates and sets V.

   There are a lot of functions named read_XXXX. All such functions read
   characters from a string using a counter. They all reset the counter
   to point at the character in the string following the last one used by
   the function. The counter is passed around from function to function
   by using pointers to it. The first character read by each of these
   functions is expected to be a member of some set of characters (often
   a specific character), and each function checks the first character.

   This version of the interpreter not saving input lines. A list of all
   lines will be needed in future versions to implement loops, and
   probably for other purposes.

   This version does not use any additional memory as it runs. No
   memory is allocated by the source code.

   This version does not suppress superfluous commands, such as a command
   to start the spindle when the spindle is already turning, or a command
   to turn on flood coolant, when flood coolant is already on.  When the
   interpreter is being used for direct control of the machining center,
   suppressing superfluous commands might confuse the user and could be
   dangerous, but when it is used to translate from one file to another,
   suppression can produce more concise output. Future versions might
   include an option for suppressing superfluous commands.

   */

   /****************************************************************************/

/*

Significant modifications by Nicholas Gill.

*/

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cctype>
#include <algorithm>
#include "rs274ngc.hh"
#include "rs274ngc_return.hh"

extern const char * _rs274ngc_errors[];

   /* numerical constants */
static const double TOLERANCE_INCH = 0.0002;
static const double TOLERANCE_MM  = 0.002;
static const double TOLERANCE_CONCAVE_CORNER = 0.01;
   /* angle threshold for concavity for
                        cutter compensation, in radians */
static const double TINY = 1e-12;                                /* for arc_data_r */
static const double UNKNOWN = 1e-20;
static const double TWO_PI =  6.2831853071795864;
static const double PI =      3.1415926535897932;
static const double PI2 =     1.5707963267948966;

   // English - Metric conversion (long number keeps error buildup down)
static const double MM_PER_INCH = 25.4;
static const double INCH_PER_MM = 0.039370078740157477;

   // G Codes are symbolic to be dialect-independent in source code
enum
{
	G_0 =       0,
	G_1 =      10,
	G_2 =      20,
	G_3 =      30,
	G_4 =      40,
	G_10 =    100,
	G_17 =    170,
	G_18 =    180,
	G_19 =    190,
	G_20 =    200,
	G_21 =    210,
	G_28 =    280,
	G_30 =    300,
	G_38_2 =  382,
	G_40 =    400,
	G_41 =    410,
	G_42 =    420,
	G_43 =    430,
	G_49 =    490,
	G_53 =    530,
	G_54 =    540,
	G_55 =    550,
	G_56 =    560,
	G_57 =    570,
	G_58 =    580,
	G_59 =    590,
	G_59_1 =  591,
	G_59_2 =  592,
	G_59_3 =  593,
	G_61 =    610,
	G_61_1 =  611,
	G_64 =    640,
	G_80 =    800,
	G_81 =    810,
	G_82 =    820,
	G_83 =    830,
	G_84 =    840,
	G_85 =    850,
	G_86 =    860,
	G_87 =    870,
	G_88 =    880,
	G_89 =    890,
	G_90 =    900,
	G_91 =    910,
	G_92 =    920,
	G_92_1 =  921,
	G_92_2 =  922,
	G_92_3 =  923,
	G_93 =    930,
	G_94 =    940,
	G_98 =    980,
	G_99 =    990
};

   // unary operations
   // These are not enums because the "&" operator is used in
   // reading the operation names and is illegal with an enum

#define ABS 1
#define ACOS 2
#define ASIN 3
#define ATAN 4
#define COS 5
#define EXP 6
#define FIX 7
#define FUP 8
#define LN 9
#define ROUND 10
#define SIN 11
#define SQRT 12
#define TAN 13

   // binary operations
#define NO_OPERATION 0
#define DIVIDED_BY 1
#define MODULO 2
#define POWER 3
#define TIMES 4
#define AND2 5
#define EXCLUSIVE_OR 6
#define MINUS 7
#define NON_EXCLUSIVE_OR 8
#define PLUS 9
#define RIGHT_BRACKET 10

   // name of parameter file for saving/restoring interpreter variables
#define RS274NGC_PARAMETER_FILE_NAME_DEFAULT "rs274ngc.var"
#define RS274NGC_PARAMETER_FILE_BACKUP_SUFFIX ".bak"

   // max number of m codes on one line
#define MAX_EMS  4

   // feed_mode
#define UNITS_PER_MINUTE 0
#define INVERSE_TIME 1

   // cutter radius compensation mode, OFF already defined to 0
   // not using CANON_SIDE since interpreter handles cutter radius comp
#define RIGHT 1
#define LEFT 2

#define DEBUG_EMC

void error_if(bool bad, int error_code)
{
	if(bad)
		throw error(error_code);
}

   /* Interpreter global arrays for g_codes and m_codes. The nth entry
   in each array is the modal group number corresponding to the nth
   code. Entries which are -1 represent illegal codes. Remember g_codes
   in this interpreter are multiplied by 10.

   The modal g groups and group numbers defined in [NCMS, pages 71 - 73]
   (see also [Fanuc, pages 43 - 45]) are used here, except the canned
   cycles (g80 - g89), which comprise modal g group 9 in [Fanuc], are
   treated here as being in the same modal group (group 1) with the
   straight moves and arcs (g0, g1, g2,g3).  [Fanuc, page 45] says only
   one g_code from any one group may appear on a line, and we are
   following that rule. The straight_probe move, g38.2, is in group 1; it
   is not defined in [NCMS].

   Some g_codes are non-modal (g4, g10, g28, g30, g53, g92, g92.1, g92.2,
   and g92.3 here - many more in [NCMS]). [Fanuc] and [NCMS] put all
   these in the same group 0, so we do also. Logically, there are two
   subgroups, those which require coordinate values (g10, g28, g30, and
   g92) and those which do not (g4, g53, g92.1, g92.2, and g92.3).
   The subgroups are identified by itemization when necessary.

   Those in group 0 which require coordinate values may not be on the
   same line as those in group 1 (except g80) because they would be
   competing for the coordinate values. Others in group 0 may be used on
   the same line as those in group 1.

   A total of 52 G-codes are implemented.

   The groups are:
   group  0 = {g4,g10,g28,g30,g53,g92,g92.1,g92.2,g92.3} - NON-MODAL
   dwell, setup, return to ref1, return to ref2,
   motion in machine coordinates, set and unset axis offsets
   group  1 = {g0,g1,g2,g3,g38.2,g80,g81,g82,g83,g84,g85,g86,g87,g88,g89} - motion
   group  2 = {g17,g18,g19}   - plane selection
   group  3 = {g90,g91}       - distance mode
   group  5 = {g93,g94}       - feed rate mode
   group  6 = {g20,g21}       - units
   group  7 = {g40,g41,g42}   - cutter diameter compensation
   group  8 = {g43,g49}       - tool length offset
   group 10 = {g98,g99}       - return mode in canned cycles
   group 12 = {g54,g55,g56,g57,g58,g59,g59.1,g59.2,g59.3} - coordinate system
   group 13 = {g61,g61.1,g64} - control mode

   */

static const int _gees[] =
{
    /*   0 */   1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /*  20 */   1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /*  40 */   0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /*  60 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /*  80 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 100 */   0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 120 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 140 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 160 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 180 */   2,-1,-1,-1,-1,-1,-1,-1,-1,-1, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 200 */   6,-1,-1,-1,-1,-1,-1,-1,-1,-1, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 220 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 240 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 260 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 280 */   0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 300 */   0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 320 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 340 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 360 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 380 */  -1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 400 */   7,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 420 */   7,-1,-1,-1,-1,-1,-1,-1,-1,-1, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 440 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 460 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 480 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 500 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 520 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 540 */  12,-1,-1,-1,-1,-1,-1,-1,-1,-1,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 560 */  12,-1,-1,-1,-1,-1,-1,-1,-1,-1,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 580 */  12,-1,-1,-1,-1,-1,-1,-1,-1,-1,12,12,12,12,-1,-1,-1,-1,-1,-1,
    /* 600 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,13,13,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 620 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 640 */  13,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 660 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 680 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 700 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 720 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 740 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 760 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 780 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 800 */   1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 820 */   1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 840 */   1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 860 */   1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 880 */   1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 900 */   3,-1,-1,-1,-1,-1,-1,-1,-1,-1, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 920 */   0, 0, 0, 0,-1,-1,-1,-1,-1,-1, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 940 */   5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 960 */  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /* 980 */  10,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

   /*

   Modal groups and modal group numbers for M codes are not described in
   [Fanuc]. We have used the groups from [NCMS] and added M60, as an
   extension of the language for pallet shuttle and stop. This version has
   no codes related to axis clamping.

   The groups are:
   group 4 = {m0,m1,m2,m30,m60} - stopping
   group 6 = {m6}               - tool change
   group 7 = {m3,m4,m5}         - spindle turning
   group 8 = {m7,m8,m9}         - coolant
   group 9 = {m48,m49}          - feed and speed override switch bypass

   */

static const int _ems[] =
{
    4,  4,  4,  7,  7,  7,  6,  8,  8,  8,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,  9,  9,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

   /* 

   This is an array of the index numbers of system parameters that must
   be included in a file used with the rs274ngc_restore_parameters
   function. The array is used by that function and by the
   rs274ngc_save_parameters function.

   */

    static const int _required_parameters[] =
    {
        5161, 5162, 5163,                         /* G28 home */
            5164,                                 /*AA*/
            5165,                                 /*BB*/
            5166,                                 /*CC*/
            5181, 5182, 5183,                     /* G30 home */
            5184,                                 /*AA*/
            5185,                                 /*BB*/
            5186,                                 /*CC*/
            5211, 5212, 5213,                     /* G92 offsets */
            5214,                                 /*AA*/
            5215,                                 /*BB*/
            5216,                                 /*CC*/
            5220,                                 /* selected coordinate */
            5221, 5222, 5223,                     /* coordinate system 1 */
            5224,                                 /*AA*/
            5225,                                 /*BB*/
            5226,                                 /*CC*/
            5241, 5242, 5243,                     /* coordinate system 2 */
            5244,                                 /*AA*/
            5245,                                 /*BB*/
            5246,                                 /*CC*/
            5261, 5262, 5263,                     /* coordinate system 3 */
            5264,                                 /*AA*/
            5265,                                 /*BB*/
            5266,                                 /*CC*/
            5281, 5282, 5283,                     /* coordinate system 4 */
            5284,                                 /*AA*/
            5285,                                 /*BB*/
            5286,                                 /*CC*/
            5301, 5302, 5303,                     /* coordinate system 5 */
            5304,                                 /*AA*/
            5305,                                 /*BB*/
            5306,                                 /*CC*/
            5321, 5322, 5323,                     /* coordinate system 6 */
            5324,                                 /*AA*/
            5325,                                 /*BB*/
            5326,                                 /*CC*/
            5341, 5342, 5343,                     /* coordinate system 7 */
            5344,                                 /*AA*/
            5345,                                 /*BB*/
            5346,                                 /*CC*/
            5361, 5362, 5363,                     /* coordinate system 8 */
            5364,                                 /*AA*/
            5365,                                 /*BB*/
            5366,                                 /*CC*/
            5381, 5382, 5383,                     /* coordinate system 9 */
            5384,                                 /*AA*/
            5385,                                 /*BB*/
            5386,                                 /*CC*/
            RS274NGC_MAX_PARAMETERS
    };

   /*

   _readers is an array of pointers to functions that read.
   It is used by read_one_item.

   */

error::error(int code)
 : code(code)
{
}
const char* error::what() const noexcept
{
	if (((code >= RS274NGC_MIN_ERROR) and (code <= RS274NGC_MAX_ERROR)) )
		return _rs274ngc_errors[code];
	else
		return "Unknown error";
}
error::~error() noexcept
{
}

rs274ngc::rs274ngc()
:	_readers
    {
        0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0, 0, 0, &rs274ngc::read_parameter_setting,0,      0,      0,      0,
        &rs274ngc::read_comment, 0, 0,     0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      &rs274ngc::read_a, &rs274ngc::read_b, &rs274ngc::read_c,
        &rs274ngc::read_d, 0,      &rs274ngc::read_f, &rs274ngc::read_g, &rs274ngc::read_h, &rs274ngc::read_i, &rs274ngc::read_j, &rs274ngc::read_k, &rs274ngc::read_l, &rs274ngc::read_m,
        0,      0,      &rs274ngc::read_p, &rs274ngc::read_q, &rs274ngc::read_r, &rs274ngc::read_s, &rs274ngc::read_t, 0     , 0,      0,
        &rs274ngc::read_x, &rs274ngc::read_y, &rs274ngc::read_z
    }
{
}

   /****************************************************************************/
   /****************************************************************************/

   /*

   The functions in this section are the interpreter kernel functions

   */

   /***********************************************************************/

   /* arc_data_comp_ijk

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The two calculable values of the radius differ by more than
   tolerance: NCE_RADIUS_TO_END_OF_ARC_DIFFERS_FROM_RADIUS_TO_START
   2. move is not G_2 or G_3: NCE_BUG_CODE_NOT_G2_OR_G3

   Side effects:
   This finds and sets the values of center_x, center_y, and turn.

   Called by: convert_arc_comp1

   This finds the center coordinates and number of full or partial turns
   counterclockwise of a helical or circular arc in ijk-format in the XY
   plane. The center is computed easily from the current point and center
   offsets, which are given. It is checked that the end point lies one
   tool radius from the arc.

   */

    void rs274ngc::arc_data_comp_ijk(                 /* ARGUMENTS                               */
    int move,                                     /* either G_2 (cw arc) or G_3 (ccw arc)             */
    int side,                                     /* either RIGHT or LEFT                             */
    double tool_radius,                           /* radius of the tool                               */
    double current_x,                             /* first coordinate of current point                */
    double current_y,                             /* second coordinate of current point               */
    double end_x,                                 /* first coordinate of arc end point                */
    double end_y,                                 /* second coordinate of arc end point               */
    double i_number,                              /* first coordinate offset of center from current   */
    double j_number,                              /* second coordinate offset of center from current  */
    double * center_x,                            /* pointer to first coordinate of center of arc     */
    double * center_y,                            /* pointer to second coordinate of center of arc    */
    int * turn,                                   /* pointer to number of full or partial circles CCW */
    double tolerance)                             /* tolerance of differing radii                     */
    {
        double arc_radius;
        double radius2;

        *center_x = (current_x + i_number);
        *center_y = (current_y + j_number);
        arc_radius = hypot(i_number, j_number);
        radius2 = hypot((*center_x - end_x), (*center_y - end_y));
        radius2 =
            (((side == LEFT ) and (move == 30)) or
            ((side == RIGHT) and (move == 20))) ?
            (radius2 - tool_radius): (radius2 + tool_radius);
        error_if((fabs(arc_radius - radius2) > tolerance), NCE_RADIUS_TO_END_OF_ARC_DIFFERS_FROM_RADIUS_TO_START);
   /* This catches an arc too small for the tool, also */
        if (move == G_2)
            *turn = -1;
        else if (move == G_3)
            *turn = 1;
        else
            throw error(NCE_BUG_CODE_NOT_G2_OR_G3);
    }

   /****************************************************************************/

   /* arc_data_comp_r

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The arc radius is too small to reach the end point:
   NCE_RADIUS_TOO_SMALL_TO_REACH_END_POINT
   2. The arc radius is not greater than the tool_radius, but should be:
   NCE_TOOL_RADIUS_NOT_LESS_THAN_ARC_RADIUS_WITH_COMP
   3. An imaginary value for offset would be found, which should never
   happen if the theory is correct: NCE_BUG_IN_TOOL_RADIUS_COMP

   Side effects:
   This finds and sets the values of center_x, center_y, and turn.

   Called by: convert_arc_comp1

   This finds the center coordinates and number of full or partial turns
   counterclockwise of a helical or circular arc (call it arc1) in
   r-format in the XY plane.  Arc2 is constructed so that it is tangent
   to a circle whose radius is tool_radius and whose center is at the
   point (current_x, current_y) and passes through the point (end_x,
   end_y). Arc1 has the same center as arc2. The radius of arc1 is one
   tool radius larger or smaller than the radius of arc2.

   If the value of the big_radius argument is negative, that means [NCMS,
   page 21] that an arc larger than a semicircle is to be made.
   Otherwise, an arc of a semicircle or less is made.

   The algorithm implemented here is to construct a line L from the
   current point to the end point, and a perpendicular to it from the
   center of the arc which intersects L at point P. Since the distance
   from the end point to the center and the distance from the current
   point to the center are known, two equations for the length of the
   perpendicular can be written. The right sides of the equations can be
   set equal to one another and the resulting equation solved for the
   length of the line from the current point to P. Then the location of
   P, the length of the perpendicular, the angle of the perpendicular,
   and the location of the center, can be found in turn.

   This needs to be better documented, with figures. There are eight
   possible arcs, since there are three binary possibilities: (1) tool
   inside or outside arc, (2) clockwise or counterclockwise (3) two
   positions for each arc (of the given radius) tangent to the tool
   outline and through the end point. All eight are calculated below,
   since theta, radius2, and turn may each have two values.

   To see two positions for each arc, imagine the arc is a hoop, the
   tool is a cylindrical pin, and the arc may rotate around the end point.
   The rotation covers all possible positions of the arc. It is easy to
   see the hoop is constrained by the pin at two different angles, whether
   the pin is inside or outside the hoop.

   */

    void rs274ngc::arc_data_comp_r(                   /* ARGUMENTS                                 */
    int move,                                     /* either G_2 (cw arc) or G_3 (ccw arc)             */
    int side,                                     /* either RIGHT or LEFT                             */
    double tool_radius,                           /* radius of the tool                               */
    double current_x,                             /* first coordinate of current point                */
    double current_y,                             /* second coordinate of current point               */
    double end_x,                                 /* first coordinate of arc end point                */
    double end_y,                                 /* second coordinate of arc end point               */
    double big_radius,                            /* radius of arc                                    */
    double * center_x,                            /* pointer to first coordinate of center of arc     */
    double * center_y,                            /* pointer to second coordinate of center of arc    */
    int * turn)                                   /* pointer to number of full or partial circles CCW */
    {
        double abs_radius;                        /* absolute value of big_radius          */
        double alpha;                             /* direction of line from current to end */
        double distance;                          /* length of line L from current to end  */
        double mid_length;                        /* length from current point to point P  */
        double offset;                            /* length of line from P to center       */
        double radius2;                           /* distance from center to current point */
        double mid_x;                             /* x-value of point P                    */
        double mid_y;                             /* y-value of point P                    */
        double theta;                             /* direction of line from P to center    */

        abs_radius = fabs(big_radius);
        error_if(((abs_radius <= tool_radius) and (((side == LEFT ) and (move == G_3)) or
            ((side == RIGHT) and (move == G_2)))),
            NCE_TOOL_RADIUS_NOT_LESS_THAN_ARC_RADIUS_WITH_COMP);

        distance = hypot((end_x - current_x), (end_y - current_y));
        alpha = atan2 ((end_y - current_y), (end_x - current_x));
        theta = (((move == G_3) and (big_radius > 0)) or
            ((move == G_2) and (big_radius < 0))) ?
            (alpha + PI2) : (alpha - PI2);
        radius2 = (((side == LEFT ) and (move == G_3)) or
            ((side == RIGHT) and (move == G_2))) ?
            (abs_radius - tool_radius) : (abs_radius + tool_radius);
        error_if((distance > (radius2 + abs_radius)), NCE_RADIUS_TOO_SMALL_TO_REACH_END_POINT);
        mid_length = (((radius2 * radius2) + (distance * distance) -
            (abs_radius * abs_radius)) / (2.0 * distance));
        mid_x = (current_x + (mid_length * cos(alpha)));
        mid_y = (current_y + (mid_length * sin(alpha)));
        error_if(((radius2 * radius2) <= (mid_length * mid_length)), NCE_BUG_IN_TOOL_RADIUS_COMP);
        offset = sqrt((radius2 * radius2) - (mid_length * mid_length));
        *center_x = mid_x + (offset * cos(theta));
        *center_y = mid_y + (offset * sin(theta));
        *turn = (move == G_2) ? -1 : 1;
    }

   /****************************************************************************/

   /* arc_data_ijk

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The two calculable values of the radius differ by more than
   tolerance: NCE_RADIUS_TO_END_OF_ARC_DIFFERS_FROM_RADIUS_TO_START
   2. The move code is not G_2 or G_3: NCE_BUG_CODE_NOT_G2_OR_G3
   3. Either of the two calculable values of the radius is zero:
   NCE_ZERO_RADIUS_ARC

   Side effects:
   This finds and sets the values of center_x, center_y, and turn.

   Called by:
   convert_arc2
   convert_arc_comp2

   This finds the center coordinates and number of full or partial turns
   counterclockwise of a helical or circular arc in ijk-format. This
   function is used by convert_arc2 for all three planes, so "x" and
   "y" really mean "first_coordinate" and "second_coordinate" wherever
   they are used here as suffixes of variable names. The i and j prefixes
   are handled similarly.

   */

    void rs274ngc::arc_data_ijk(                      /* ARGUMENTS                                       */
    int move,                                     /* either G_2 (cw arc) or G_3 (ccw arc)            */
    double current_x,                             /* first coordinate of current point               */
    double current_y,                             /* second coordinate of current point              */
    double end_x,                                 /* first coordinate of arc end point               */
    double end_y,                                 /* second coordinate of arc end point              */
    double i_number,                              /* first coordinate offset of center from current  */
    double j_number,                              /* second coordinate offset of center from current */
    double * center_x,                            /* pointer to first coordinate of center of arc    */
    double * center_y,                            /* pointer to second coordinate of center of arc   */
    int * turn,                                   /* pointer to no. of full or partial circles CCW   */
    double tolerance)                             /* tolerance of differing radii                    */
    {
        double radius;                            /* radius to current point */
        double radius2;                           /* radius to end point     */
        *center_x = (current_x + i_number);
        *center_y = (current_y + j_number);
        radius = hypot((*center_x - current_x), (*center_y - current_y));
        radius2 = hypot((*center_x - end_x), (*center_y - end_y));
        error_if(((radius == 0.0) or (radius2 == 0.0)), NCE_ZERO_RADIUS_ARC);
        error_if((fabs(radius - radius2) > tolerance),
            NCE_RADIUS_TO_END_OF_ARC_DIFFERS_FROM_RADIUS_TO_START);
        if (move == G_2)
            *turn = -1;
        else if (move == G_3)
            *turn = 1;
        else
            throw error(NCE_BUG_CODE_NOT_G2_OR_G3);
    }

   /****************************************************************************/

   /* arc_data_r

   Returned Value: int
   If any of the following errors occur, this returns the error shown.
   Otherwise, it returns RS274NGC_OK.
   1. The radius is too small to reach the end point:
   NCE_ARC_RADIUS_TOO_SMALL_TO_REACH_END_POINT
   2. The current point is the same as the end point of the arc
   (so that it is not possible to locate the center of the circle):
   NCE_CURRENT_POINT_SAME_AS_END_POINT_OF_ARC

   Side effects:
   This finds and sets the values of center_x, center_y, and turn.

   Called by:
   convert_arc2
   convert_arc_comp2

   This finds the center coordinates and number of full or partial turns
   counterclockwise of a helical or circular arc in the r format. This
   function is used by convert_arc2 for all three planes, so "x" and
   "y" really mean "first_coordinate" and "second_coordinate" wherever
   they are used here as suffixes of variable names.

   If the value of the radius argument is negative, that means [NCMS,
   page 21] that an arc larger than a semicircle is to be made.
   Otherwise, an arc of a semicircle or less is made.

   The algorithm used here is based on finding the midpoint M of the line
   L between the current point and the end point of the arc. The center
   of the arc lies on a line through M perpendicular to L.

   */

    void rs274ngc::arc_data_r(                        /* ARGUMENTS                                     */
    int move,                                     /* either G_2 (cw arc) or G_3 (ccw arc)          */
    double current_x,                             /* first coordinate of current point             */
    double current_y,                             /* second coordinate of current point            */
    double end_x,                                 /* first coordinate of arc end point             */
    double end_y,                                 /* second coordinate of arc end point            */
    double radius,                                /* radius of arc                                 */
    double * center_x,                            /* pointer to first coordinate of center of arc  */
    double * center_y,                            /* pointer to second coordinate of center of arc */
    int * turn)                                   /* pointer to no. of full or partial circles CCW */
    {
        double abs_radius;                        /* absolute value of given radius */
        double half_length;                       /* distance from M to end point   */
        double mid_x;                             /* first coordinate of M          */
        double mid_y;                             /* second coordinate of M         */
        double offset;                            /* distance from M to center      */
        double theta;                             /* angle of line from M to center */
        double turn2;                             /* absolute value of half of turn */

        error_if(((end_x == current_x) and (end_y == current_y)),
            NCE_CURRENT_POINT_SAME_AS_END_POINT_OF_ARC);
        abs_radius = fabs(radius);
        mid_x = (end_x + current_x)/2.0;
        mid_y = (end_y + current_y)/2.0;
        half_length = hypot((mid_x - end_x), (mid_y - end_y));
        error_if(((half_length/abs_radius) > (1+TINY)),
            NCE_ARC_RADIUS_TOO_SMALL_TO_REACH_END_POINT);
        if ((half_length/abs_radius) > (1-TINY))
            half_length = abs_radius;        /* allow a small error for semicircle */
   /* check needed before calling asin   */
        if (((move == G_2) and (radius > 0)) or
            ((move == G_3) and (radius < 0)))
            theta = atan2((end_y - current_y), (end_x - current_x)) - PI2;
        else
            theta = atan2((end_y - current_y), (end_x - current_x)) + PI2;

        turn2 = asin (half_length/abs_radius);
        offset = abs_radius * cos(turn2);
        *center_x = mid_x + (offset * cos(theta));
        *center_y = mid_y + (offset * sin(theta));
        *turn = (move == G_2) ? -1 : 1;
    }

   /****************************************************************************/

   /* check_g_codes

   Returned Value: int
   If any of the following errors occur, this returns the error shown.
   Otherwise, it returns RS274NGC_OK.
   1. NCE_DWELL_TIME_MISSING_WITH_G4
   2. NCE_MUST_USE_G0_OR_G1_WITH_G53
   3. NCE_CANNOT_USE_G53_INCREMENTAL
   4. NCE_LINE_WITH_G10_DOES_NOT_HAVE_L2
   5. NCE_P_VALUE_NOT_AN_INTEGER_WITH_G10_L2
   6. NCE_P_VALUE_OUT_OF_RANGE_WITH_G10_L2
   7. NCE_BUG_BAD_G_CODE_MODAL_GROUP_0

   Side effects: none

   Called by: check_items

   This runs checks on g_codes from a block of RS274/NGC instructions.
   Currently, all checks are on g_codes in modal group 0.

   The read_g function checks for errors which would foul up the reading.
   The enhance_block function checks for logical errors in the use of
   axis values by g-codes in modal groups 0 and 1.
   This function checks for additional logical errors in g_codes.

   [Fanuc, page 45, note 4] says there is no maximum for how many g_codes
   may be put on the same line, [NCMS] says nothing one way or the other,
   so the test for that is not used.

   We are suspending any implicit motion g_code when a g_code from our
   group 0 is used.  The implicit motion g_code takes effect again
   automatically after the line on which the group 0 g_code occurs.  It
   is not clear what the intent of [Fanuc] is in this regard. The
   alternative is to require that any implicit motion be explicitly
   cancelled.

   Not all checks on g_codes are included here. Those checks that are
   sensitive to whether other g_codes on the same line have been executed
   yet are made by the functions called by convert_g.

   Our reference sources differ regarding what codes may be used for
   dwell time.  [Fanuc, page 58] says use "p" or "x". [NCMS, page 23] says
   use "p", "x", or "u". We are allowing "p" only, since it is consistent
   with both sources and "x" would be confusing. However, "p" is also used
   with G10, where it must be an integer, so reading "p" values is a bit
   more trouble than would be nice.

   */

    void rs274ngc::check_g_codes(                     /* ARGUMENTS                        */
    block_t& block,                          /* pointer to a block to be checked */
    setup_t& settings)                       /* pointer to machine settings      */
    {
        int mode0;
        int p_int;

        mode0 = block.g_modes[0];

        if (mode0 == -1)
            {}
            else if (mode0 == G_4)
        {
            error_if((block.p_number == -1.0), NCE_DWELL_TIME_MISSING_WITH_G4);
        }
        else if (mode0 == G_10)
        {
            p_int = (int)(block.p_number + 0.0001);
            error_if((block.l_number != 2), NCE_LINE_WITH_G10_DOES_NOT_HAVE_L2);
            error_if((((block.p_number + 0.0001) - p_int) > 0.0002),
                NCE_P_VALUE_NOT_AN_INTEGER_WITH_G10_L2);
            error_if(((p_int < 1) or (p_int > 9)), NCE_P_VALUE_OUT_OF_RANGE_WITH_G10_L2);
        }
        else if (mode0 == G_28)
            {}
            else if (mode0 == G_30)
                {}
                else if (mode0 == G_53)
                {
                    error_if(((block.motion_to_be != G_0) and (block.motion_to_be != G_1)),
                NCE_MUST_USE_G0_OR_G1_WITH_G53);
            error_if(((block.g_modes[3] == G_91) or
                ((block.g_modes[3] != G_90) and
                (settings.distance_mode == MODE_INCREMENTAL))),
                NCE_CANNOT_USE_G53_INCREMENTAL);
        }
        else if (mode0 == G_92)
            {}
            else if ((mode0 == G_92_1) or (mode0 == G_92_2) or (mode0 == G_92_3))
                {}
                else
                    throw error(NCE_BUG_BAD_G_CODE_MODAL_GROUP_0);
    }

   /****************************************************************************/

   /* check_items

   Returned Value: int
   If any one of check_g_codes, check_m_codes, and check_other_codes
   returns an error code, this returns that code.
   Otherwise, it returns RS274NGC_OK.

   Side effects: none

   Called by: parse_line

   This runs checks on a block of RS274 code.

   The functions named read_XXXX check for errors which would foul up the
   reading. This function checks for additional logical errors.

   A block has an array of g_codes, which are initialized to -1
   (meaning no code). This calls check_g_codes to check the g_codes.

   A block has an array of m_codes, which are initialized to -1
   (meaning no code). This calls check_m_codes to check the m_codes.

   Items in the block which are not m or g codes are checked by
   check_other_codes.

   */

    void rs274ngc::check_items(                       /* ARGUMENTS                        */
    block_t& block,                          /* pointer to a block to be checked */
    setup_t& settings)                       /* pointer to machine settings      */
    {
        check_g_codes(block, settings);
        check_m_codes(block);
        check_other_codes(block);
    }

   /****************************************************************************/

   /* check_m_codes

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. There are too many m codes in the block: NCE_TOO_MANY_M_CODES_ON_LINE

   Side effects: none

   Called by: check_items

   This runs checks on m_codes from a block of RS274/NGC instructions.

   The read_m function checks for errors which would foul up the
   reading. This function checks for additional errors in m_codes.

   */

    void rs274ngc::check_m_codes(                     /* ARGUMENTS                        */
    block_t& block)                          /* pointer to a block to be checked */
    {
        error_if((block.m_count > MAX_EMS), NCE_TOO_MANY_M_CODES_ON_LINE);
    }

   /****************************************************************************/

   /* check_other_codes

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. An A-axis value is given with a canned cycle (g80 to g89):
   NCE_CANNOT_PUT_AN_A_IN_CANNED_CYCLE
   2. A B-axis value is given with a canned cycle (g80 to g89):
   NCE_CANNOT_PUT_A_B_IN_CANNED_CYCLE
   3. A C-axis value is given with a canned cycle (g80 to g89):
   NCE_CANNOT_PUT_A_C_IN_CANNED_CYCLE
   4. A d word is in a block with no cutter_radius_compensation_on command:
   NCE_D_WORD_WITH_NO_G41_OR_G42
   5. An h_number is in a block with no tool length offset setting:
   NCE_H_WORD_WITH_NO_G43
   6. An i_number is in a block with no G code that uses it:
   NCE_I_WORD_WITH_NO_G2_OR_G3_OR_G87_TO_USE_IT
   7. A j_number is in a block with no G code that uses it:
   NCE_J_WORD_WITH_NO_G2_OR_G3_OR_G87_TO_USE_IT
   8. A k_number is in a block with no G code that uses it:
   NCE_K_WORD_WITH_NO_G2_OR_G3_OR_G87_TO_USE_IT
   9. A l_number is in a block with no G code that uses it:
   NCE_L_WORD_WITH_NO_CANNED_CYCLE_OR_G10
   10. A p_number is in a block with no G code that uses it:
   NCE_P_WORD_WITH_NO_G4_G10_G82_G86_G88_G89
   11. A q_number is in a block with no G code that uses it:
   NCE_Q_WORD_WITH_NO_G83
   12. An r_number is in a block with no G code that uses it:
   NCE_R_WORD_WITH_NO_G_CODE_THAT_USES_IT

   Side effects: none

   Called by: check_items

   This runs checks on codes from a block of RS274/NGC code which are
   not m or g codes.

   The functions named read_XXXX check for errors which would foul up the
   reading. This function checks for additional logical errors in codes.

   */

    void rs274ngc::check_other_codes(                 /* ARGUMENTS                               */
    block_t& block)                          /* pointer to a block of RS274/NGC instructions */
    {
        int motion;

        motion = block.motion_to_be;
        if (block.a_flag != OFF)
        {
            error_if(((block.g_modes[1] > G_80) and (block.g_modes[1] < G_90)), NCE_CANNOT_PUT_AN_A_IN_CANNED_CYCLE);
        }
        if (block.b_flag != OFF)
        {
            error_if(((block.g_modes[1] > G_80) and (block.g_modes[1] < G_90)), NCE_CANNOT_PUT_A_B_IN_CANNED_CYCLE);
        }
        if (block.c_flag != OFF)
        {
            error_if(((block.g_modes[1] > G_80) and (block.g_modes[1] < G_90)), NCE_CANNOT_PUT_A_C_IN_CANNED_CYCLE);
        }
        if (block.d_number != -1)
        {
            error_if(((block.g_modes[7] != G_41) and (block.g_modes[7] != G_42)), NCE_D_WORD_WITH_NO_G41_OR_G42);
        }
        if (block.h_number != -1)
        {
            error_if((block.g_modes[8] != G_43), NCE_H_WORD_WITH_NO_G43);
        }

        if (block.i_flag == ON)                  /* could still be useless if yz_plane arc */
        {
            error_if(((motion != G_2) and (motion != G_3) and (motion != G_87)), NCE_I_WORD_WITH_NO_G2_OR_G3_OR_G87_TO_USE_IT);
        }

        if (block.j_flag == ON)                  /* could still be useless if xz_plane arc */
        {
            error_if(((motion != G_2) and (motion != G_3) and (motion != G_87)), NCE_J_WORD_WITH_NO_G2_OR_G3_OR_G87_TO_USE_IT);
        }

        if (block.k_flag == ON)                  /* could still be useless if xy_plane arc */
        {
            error_if(((motion != G_2) and (motion != G_3) and (motion != G_87)), NCE_K_WORD_WITH_NO_G2_OR_G3_OR_G87_TO_USE_IT);
        }

        if (block.l_number != -1)
        {
            error_if((((motion < G_81) or (motion > G_89)) and
                (block.g_modes[0] != G_10)),
                NCE_L_WORD_WITH_NO_CANNED_CYCLE_OR_G10);
        }

        if (block.p_number != -1.0)
        {
            error_if(((block.g_modes[0] != G_10) and
                (block.g_modes[0] != G_4) and
                (motion != G_82) and (motion != G_86) and
                (motion != G_88) and (motion != G_89)),
                NCE_P_WORD_WITH_NO_G4_G10_G82_G86_G88_G89);
        }

        if (block.q_number != -1.0)
        {
            error_if(motion != G_83, NCE_Q_WORD_WITH_NO_G83);
        }

        if (block.r_flag == ON)
        {
            error_if((((motion != G_2) and (motion != G_3)) and
                ((motion < G_81) or (motion > G_89))),
                NCE_R_WORD_WITH_NO_G_CODE_THAT_USES_IT);
        }
    }

   /****************************************************************************/

   /* close_and_downcase

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. A left parenthesis is found inside a comment:
   NCE_NESTED_COMMENT_FOUND
   2. The line ends before an open comment is closed:
   NCE_UNCLOSED_COMMENT_FOUND
   3. A newline character is found that is not followed by null:
   NCE_NULL_MISSING_AFTER_NEWLINE

   Side effects: See below

   Called by:  read_text

   To simplify handling upper case letters, spaces, and tabs, this
   function removes spaces and and tabs and downcases everything on a
   line which is not part of a comment.

   Comments are left unchanged in place. Comments are anything
   enclosed in parentheses. Nested comments, indicated by a left
   parenthesis inside a comment, are illegal.

   The line must have a null character at the end when it comes in.
   The line may have one newline character just before the end. If
   there is a newline, it will be removed.

   Although this software system detects and rejects all illegal characters
   and illegal syntax, this particular function does not detect problems
   with anything but comments.

   We are treating RS274 code here as case-insensitive and spaces and
   tabs as if they have no meaning. [RS274D, page 6] says spaces and tabs
   are to be ignored by control.

   The KT and NGC manuals say nothing about case or spaces and tabs.

   */

    void rs274ngc::close_and_downcase(                /* ARGUMENTS                   */
    char * line)                                  /* string: one line of NC code */
    {
        int m;
        int n;
        int comment;
        char item;
        comment = 0;
        for (n = 0, m = 0; (item = line[m]) != (char) NULL; m++)
        {
            if (comment)
            {
                line[n++] = item;
                if (item == ')')
                {
                    comment = 0;
                }
                else if (item == '(')
                    throw error(NCE_NESTED_COMMENT_FOUND);
            }
            else if ((item == ' ') or (item == '\t') or (item == '\r'));
   /* don't copy blank or tab or CR */
            else if (item == '\n')                /* don't copy newline            */
            {                                     /* but check null follows        */
                error_if(line[m+1] != 0, NCE_NULL_MISSING_AFTER_NEWLINE);
            }
            else if ((64 < item) and (item < 91)) /* downcase upper case letters */
            {
                line[n++] = (32 + item);
            }
            else if (item == '(')                 /* comment is starting */
            {
                comment = 1;
                line[n++] = item;
            }
            else
            {
                line[n++] = item;            /* copy anything else */
            }
        }
        error_if(comment, NCE_UNCLOSED_COMMENT_FOUND);
        line[n] = 0;
    }

   /****************************************************************************/

   /* convert_arc

   Returned Value: int
   If one of the following functions returns an error code,
   this returns that error code.
   convert_arc_comp1
   convert_arc_comp2
   convert_arc2
   If any of the following errors occur, this returns the error code shown.
   Otherwise, this returns RS274NGC_OK.
   1. The block has neither an r value nor any i,j,k values:
   NCE_R_I_J_K_WORDS_ALL_MISSING_FOR_ARC
   2. The block has both an r value and one or more i,j,k values:
   NCE_MIXED_RADIUS_IJK_FORMAT_FOR_ARC
   3. In the ijk format the XY-plane is selected and
   the block has a k value: NCE_K_WORD_GIVEN_FOR_ARC_IN_XY_PLANE
   4. In the ijk format the YZ-plane is selected and
   the block has an i value: NCE_I_WORD_GIVEN_FOR_ARC_IN_YZ_PLANE
   5. In the ijk format the XZ-plane is selected and
   the block has a j value: NCE_J_WORD_GIVEN_FOR_ARC_IN_XZ_PLANE
   6. In either format any of the following occurs.
   a. The XY-plane is selected and the block has no x or y value:
   NCE_X_AND_Y_WORDS_MISSING_FOR_ARC_IN_XY_PLANE
   b. The YZ-plane is selected and the block has no y or z value:
   NCE_Y_AND_Z_WORDS_MISSING_FOR_ARC_IN_YZ_PLANE
   c. The ZX-plane is selected and the block has no z or x value:
   NCE_X_AND_Z_WORDS_MISSING_FOR_ARC_IN_XZ_PLANE
   7. The selected plane is an unknown plane:
   NCE_BUG_PLANE_NOT_XY_YZ__OR_XZ
   8. The feed rate mode is UNITS_PER_MINUTE and feed rate is zero:
   NCE_CANNOT_MAKE_ARC_WITH_ZERO_FEED_RATE
   9. The feed rate mode is INVERSE_TIME and the block has no f word:
   NCE_F_WORD_MISSING_WITH_INVERSE_TIME_ARC_MOVE

   Side effects:
   This generates and executes an arc command at feed rate
   (and, possibly a second arc command). It also updates the setting
   of the position of the tool point to the end point of the move.

   Called by: convert_motion.

   This converts a helical or circular arc.  The function calls:
   convert_arc2 (when cutter radius compensation is off) or
   convert_arc_comp1 (when cutter comp is on and this is the first move) or
   convert_arc_comp2 (when cutter comp is on and this is not the first move).

   If the ijk format is used, at least one of the offsets in the current
   plane must be given in the block; it is common but not required to
   give both offsets. The offsets are always incremental [NCMS, page 21].

   */

    void rs274ngc::convert_arc(                       /* ARGUMENTS                                */
    int move,                                     /* either G_2 (cw arc) or G_3 (ccw arc)     */
    block_t& block,                          /* pointer to a block of RS274 instructions */
    setup_t& settings)                       /* pointer to machine settings              */
    {
        int first;                                /* flag set ON if this is first move after comp ON */
        int ijk_flag;                             /* flag set ON if any of i,j,k present in NC code  */
        double end_x;
        double end_y;
        double end_z;
        double AA_end;                            /*AA*/
        double BB_end;                            /*BB*/
        double CC_end;                            /*CC*/

        ijk_flag =
            ((block.i_flag or block.j_flag) or block.k_flag) ? ON : OFF;
        first = (settings.program_x == UNKNOWN);

        error_if(((block.r_flag != ON) and (ijk_flag != ON)), NCE_R_I_J_K_WORDS_ALL_MISSING_FOR_ARC);
        error_if(((block.r_flag == ON) and (ijk_flag == ON)), NCE_MIXED_RADIUS_IJK_FORMAT_FOR_ARC);
        if (settings.feed_mode == UNITS_PER_MINUTE)
        {
            error_if((settings.feed_rate == 0.0), NCE_CANNOT_MAKE_ARC_WITH_ZERO_FEED_RATE);
        }
        else if (settings.feed_mode == INVERSE_TIME)
        {
            error_if((block.f_number == -1.0), NCE_F_WORD_MISSING_WITH_INVERSE_TIME_ARC_MOVE);
        }
        if (ijk_flag)
        {
            if (settings.plane == CANON_PLANE_XY)
            {
                error_if((block.k_flag), NCE_K_WORD_GIVEN_FOR_ARC_IN_XY_PLANE);
                if (block.i_flag == OFF)         /* i or j flag on to get here */
                    block.i_number = 0.0;
                else if (block.j_flag == OFF)
                    block.j_number = 0.0;
            }
            else if (settings.plane == CANON_PLANE_YZ)
            {
                error_if((block.i_flag), NCE_I_WORD_GIVEN_FOR_ARC_IN_YZ_PLANE);
                if (block.j_flag == OFF)         /* j or k flag on to get here */
                    block.j_number = 0.0;
                else if (block.k_flag == OFF)
                    block.k_number = 0.0;
            }
            else if (settings.plane == CANON_PLANE_XZ)
            {
                error_if((block.j_flag), NCE_J_WORD_GIVEN_FOR_ARC_IN_XZ_PLANE);
                if (block.i_flag == OFF)         /* i or k flag on to get here */
                    block.i_number = 0.0;
                else if (block.k_flag == OFF)
                    block.k_number = 0.0;
            }
            else
                throw error(NCE_BUG_PLANE_NOT_XY_YZ_OR_XZ);
        }
        else
        {
             /* r format arc; no other checks needed specific to this format */
        }

        if (settings.plane == CANON_PLANE_XY)    /* checks for both formats */
        {
            error_if(((block.x_flag == OFF) and (block.y_flag == OFF)), NCE_X_AND_Y_WORDS_MISSING_FOR_ARC_IN_XY_PLANE);
        }
        else if (settings.plane == CANON_PLANE_YZ)
        {
            error_if(((block.y_flag == OFF) and (block.z_flag == OFF)), NCE_Y_AND_Z_WORDS_MISSING_FOR_ARC_IN_YZ_PLANE);
        }
        else if (settings.plane == CANON_PLANE_XZ)
        {
            error_if(((block.x_flag == OFF) and (block.z_flag == OFF)), NCE_X_AND_Z_WORDS_MISSING_FOR_ARC_IN_XZ_PLANE);
        }

        find_ends(block, settings, &end_x, &end_y,
            &end_z
            , &AA_end
            , &BB_end
            , &CC_end
            );
        settings.motion_mode = move;

        if (settings.plane == CANON_PLANE_XY)
        {
            if ((settings.cutter_comp_side == OFF) or
                (settings.cutter_comp_radius == 0.0))
            {
                    convert_arc2(move, block, settings,
                    &(settings.current.x), &(settings.current.y),
                    &(settings.current.z), end_x, end_y,
                    end_z
                    , AA_end
                    , BB_end
                    , CC_end
                    , block.i_number,
                    block.j_number);
            }
            else if (first)
            {
                    convert_arc_comp1(move, block, settings, end_x, end_y,
                    end_z
                    , AA_end
                    , BB_end
                    , CC_end
                    );
            }
            else
            {
                    convert_arc_comp2(move, block, settings, end_x, end_y,
                    end_z
                    , AA_end
                    , BB_end
                    , CC_end
                    );
            }
        }
        else if (settings.plane == CANON_PLANE_XZ)
        {
                convert_arc2 (move, block, settings,
                &(settings.current.z), &(settings.current.x),
                &(settings.current.y), end_z, end_x,
                end_y
                , AA_end
                , BB_end
                , CC_end
                , block.k_number,
                block.i_number);
        }
        else if (settings.plane == CANON_PLANE_YZ)
        {
                convert_arc2 (move, block, settings,
                &(settings.current.y), &(settings.current.z),
                &(settings.current.x), end_y, end_z,
                end_x
                , AA_end
                , BB_end
                , CC_end
                , block.j_number,
                block.k_number);
        }
        else
            throw error(NCE_BUG_PLANE_NOT_XY_YZ_OR_XZ);
    }

   /****************************************************************************/

   /* convert_arc2

   Returned Value: int
   If arc_data_ijk or arc_data_r returns an error code,
   this returns that code.
   Otherwise, it returns RS274NGC_OK.

   Side effects:
   This executes an arc command at feed rate. It also updates the
   setting of the position of the tool point to the end point of the move.
   If inverse time feed rate is in effect, it also resets the feed rate.

   Called by: convert_arc.

   This converts a helical or circular arc.

   */

    void rs274ngc::convert_arc2(                      /* ARGUMENTS                                */
    int move,                                     /* either G_2 (cw arc) or G_3 (ccw arc)     */
    block_t& block,                          /* pointer to a block of RS274 instructions */
    setup_t& settings,                       /* pointer to machine settings              */
    double * current1,                            /* pointer to current value of coordinate 1 */
    double * current2,                            /* pointer to current value of coordinate 2 */
    double * current3,                            /* pointer to current value of coordinate 3 */
    double end1,                                  /* coordinate 1 value at end of arc         */
    double end2,                                  /* coordinate 2 value at end of arc         */
    double end3,                                  /* coordinate 3 value at end of arc         */
    double AA_end,                                /* a-value at end of arc                    *//*AA*/
    double BB_end,                                /* b-value at end of arc                    *//*BB*/
    double CC_end,                                /* c-value at end of arc                    *//*CC*/
    double offset1,                               /* offset of center from current1           */
    double offset2)                               /* offset of center from current2           */
    {
        double center1;
        double center2;
        double tolerance;                         /* tolerance for difference of radii          */
        int turn;                                 /* number of full or partial turns CCW in arc */

        tolerance = (settings.length_units == CANON_UNITS_INCHES) ?
            TOLERANCE_INCH : TOLERANCE_MM;

        if (block.r_flag)
        {
            arc_data_r(move, *current1, *current2, end1, end2, block.r_number, &center1, &center2, &turn);
        }
        else
        {
            arc_data_ijk(move, *current1, *current2, end1, end2, offset1, offset2, &center1, &center2, &turn, tolerance);
        }

        if (settings.feed_mode == INVERSE_TIME)
            inverse_time_rate_arc(*current1, *current2, *current3, center1, center2,
                turn, end1, end2, end3, block, settings);
        ARC_FEED(end1, end2, center1, center2, turn,
            end3
            , AA_end
            , BB_end
            , CC_end
            );
        *current1 = end1;
        *current2 = end2;
        *current3 = end3;
        settings.current.a = AA_end;       /*AA*/
        settings.current.b = BB_end;       /*BB*/
        settings.current.c = CC_end;       /*CC*/
    }

   /****************************************************************************/

   /* convert_arc_comp1

   Returned Value: int
   If arc_data_comp_ijk or arc_data_comp_r returns an error code,
   this returns that code.
   Otherwise, it returns RS274NGC_OK.

   Side effects:
   This executes an arc command at
   feed rate. It also updates the setting of the position of
   the tool point to the end point of the move.

   Called by: convert_arc.

   This function converts a helical or circular arc, generating only one
   arc. The axis must be parallel to the z-axis. This is called when
   cutter radius compensation is on and this is the first cut after the
   turning on.

   The arc which is generated is derived from a second arc which passes
   through the programmed end point and is tangent to the cutter at its
   current location. The generated arc moves the tool so that it stays
   tangent to the second arc throughout the move.

   */

    void rs274ngc::convert_arc_comp1(                 /* ARGUMENTS                                   */
    int move,                                     /* either G_2 (cw arc) or G_3 (ccw arc)             */
    block_t& block,                          /* pointer to a block of RS274/NGC instructions     */
    setup_t& settings,                       /* pointer to machine settings                      */
    double end_x,                                 /* x-value at end of programmed (then actual) arc   */
    double end_y,                                 /* y-value at end of programmed (then actual) arc   */
    double end_z                                  /* z-value at end of arc                            */
    , double AA_end                               /* a-value at end of arc                      *//*AA*/
    , double BB_end                               /* b-value at end of arc                      *//*BB*/
    , double CC_end                               /* c-value at end of arc                      *//*CC*/
    )
    {
        double center_x;
        double center_y;
        double gamma;                             /* direction of perpendicular to arc at end */
        int side;                                 /* offset side - right or left              */
        double tolerance;                         /* tolerance for difference of radii        */
        double tool_radius;
        int turn;                                 /* 1 for counterclockwise, -1 for clockwise */

        side = settings.cutter_comp_side;
   /* always is positive */
        tool_radius = settings.cutter_comp_radius;
        tolerance = (settings.length_units == CANON_UNITS_INCHES) ?
            TOLERANCE_INCH : TOLERANCE_MM;

        error_if((hypot((end_x - settings.current.x),
            (end_y - settings.current.y)) <= tool_radius),
            NCE_CUTTER_GOUGING_WITH_CUTTER_RADIUS_COMP);

        if (block.r_flag)
        {
            arc_data_comp_r(move, side, tool_radius, settings.current.x, settings.current.y, end_x, end_y, block.r_number, &center_x, &center_y, &turn);
        }
        else
        {
            arc_data_comp_ijk(move, side, tool_radius, settings.current.x, settings.current.y, end_x, end_y, block.i_number, block.j_number, &center_x, &center_y, &turn, tolerance);
        }

        gamma =
            (((side == LEFT) and (move == G_3)) or
            ((side == RIGHT) and (move == G_2))) ?
            atan2 ((center_y - end_y), (center_x - end_x)) :
        atan2 ((end_y - center_y), (end_x - center_x));

        settings.program_x = end_x;
        settings.program_y = end_y;
   /* end_x reset actual */
        end_x = (end_x + (tool_radius * cos(gamma)));
   /* end_y reset actual */
        end_y = (end_y + (tool_radius * sin(gamma)));

        if (settings.feed_mode == INVERSE_TIME)
            inverse_time_rate_arc(settings.current.x, settings.current.y,
                settings.current.z, center_x, center_y, turn,
                end_x, end_y, end_z, block, settings);
        ARC_FEED(end_x, end_y, center_x, center_y, turn,
            end_z
            , AA_end
            , BB_end
            , CC_end
            );
        settings.current.x = end_x;
        settings.current.y = end_y;
        settings.current.z = end_z;
        settings.current.a = AA_end;       /*AA*/
        settings.current.b = BB_end;       /*BB*/
        settings.current.c = CC_end;       /*CC*/
    }

   /****************************************************************************/

   /* convert_arc_comp2

   Returned Value: int
   If arc_data_ijk or arc_data_r returns an error code,
   this returns that code.
   If any of the following errors occurs, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. A concave corner is found: NCE_CONCAVE_CORNER_WITH_CUTTER_RADIUS_COMP
   2. The tool will not fit inside an arc:
   NCE_TOOL_RADIUS_NOT_LESS_THAN_ARC_RADIUS_WITH_COMP

   Side effects:
   This executes an arc command feed rate. If needed, at also generates
   an arc to go around a convex corner. It also updates the setting of
   the position of the tool point to the end point of the move. If
   inverse time feed rate mode is in effect, the feed rate is reset.

   Called by: convert_arc.

   This function converts a helical or circular arc. The axis must be
   parallel to the z-axis. This is called when cutter radius compensation
   is on and this is not the first cut after the turning on.

   If one or more rotary axes is moved in this block and an extra arc is
   required to go around a sharp corner, all the rotary axis motion
   occurs on the main arc and none on the extra arc.  An alternative
   might be to distribute the rotary axis motion over the extra arc and
   the programmed arc in proportion to their lengths.

   If the Z-axis is moved in this block and an extra arc is required to
   go around a sharp corner, all the Z-axis motion occurs on the main arc
   and none on the extra arc.  An alternative might be to distribute the
   Z-axis motion over the extra arc and the main arc in proportion to
   their lengths.

   */

    void rs274ngc::convert_arc_comp2(                 /* ARGUMENTS                                 */
    int move,                                     /* either G_2 (cw arc) or G_3 (ccw arc)           */
    block_t& block,                          /* pointer to a block of RS274/NGC instructions   */
    setup_t& settings,                       /* pointer to machine settings                    */
    double end_x,                                 /* x-value at end of programmed (then actual) arc */
    double end_y,                                 /* y-value at end of programmed (then actual) arc */
    double end_z                                  /* z-value at end of arc                          */
    , double AA_end                               /* a-value at end of arc                    *//*AA*/
    , double BB_end                               /* b-value at end of arc                    *//*BB*/
    , double CC_end                               /* c-value at end of arc                    *//*CC*/
    )
    {
        double alpha;                             /* direction of tangent to start of arc */
        double arc_radius;
        double beta;                              /* angle between two tangents above */
        double center_x;                          /* center of arc */
        double center_y;
        double delta;                             /* direction of radius from start of arc to center of arc */
        double gamma;                             /* direction of perpendicular to arc at end */
        double mid_x;
        double mid_y;
        int side;
   /* angle for testing corners */
        double small = TOLERANCE_CONCAVE_CORNER;
        double start_x;
        double start_y;
        double theta;                             /* direction of tangent to last cut */
        double tolerance;
        double tool_radius;
        int turn;                                 /* number of full or partial circles CCW */

   /* find basic arc data: center_x, center_y, and turn */

        start_x = settings.program_x;
        start_y = settings.program_y;
        tolerance = (settings.length_units == CANON_UNITS_INCHES) ? TOLERANCE_INCH : TOLERANCE_MM;

        if (block.r_flag)
        {
            arc_data_r(move, start_x, start_y, end_x, end_y, block.r_number, &center_x, &center_y, &turn);
        }
        else
        {
            arc_data_ijk(move, start_x, start_y, end_x, end_y, block.i_number, block.j_number, &center_x, &center_y, &turn, tolerance);
        }

   /* compute other data */
        side = settings.cutter_comp_side;
   /* always is positive */
        tool_radius = settings.cutter_comp_radius;
        arc_radius = hypot((center_x - end_x), (center_y - end_y));
        theta =
            atan2(settings.current.y - start_y, settings.current.x - start_x);
        theta = (side == LEFT) ? (theta - PI2) : (theta + PI2);
        delta = atan2(center_y - start_y, center_x - start_x);
        alpha = (move == G_3) ? (delta - PI2) : (delta + PI2);
        beta = (side == LEFT) ? (theta - alpha) : (alpha - theta);
        beta = (beta > (1.5 * PI))  ? (beta - TWO_PI) :
        (beta < -PI2) ? (beta + TWO_PI) : beta;

        if (((side == LEFT)  and (move == G_3)) or
            ((side == RIGHT) and (move == G_2)))
        {
            gamma = atan2 ((center_y - end_y), (center_x - end_x));
            error_if((arc_radius <= tool_radius), NCE_TOOL_RADIUS_NOT_LESS_THAN_ARC_RADIUS_WITH_COMP);
        }
        else
        {
            gamma = atan2 ((end_y - center_y), (end_x - center_x));
            delta = (delta + PI);
        }

        settings.program_x = end_x;
        settings.program_y = end_y;
   /* end_x reset actual */
        end_x = (end_x + (tool_radius * cos(gamma)));
   /* end_y reset actual */
        end_y = (end_y + (tool_radius * sin(gamma)));

   /* check if extra arc needed and insert if so */

        error_if(((beta < -small) or (beta > (PI + small))), NCE_CONCAVE_CORNER_WITH_CUTTER_RADIUS_COMP);
        if (beta > small)                         /* two arcs needed */
        {
            mid_x = (start_x + (tool_radius * cos(delta)));
            mid_y = (start_y + (tool_radius * sin(delta)));
            if (settings.feed_mode == INVERSE_TIME)
                inverse_time_rate_arc2(start_x, start_y, (side == LEFT) ? -1 : 1,
                mid_x, mid_y, center_x, center_y, turn,
                end_x, end_y, end_z, block, settings);
            ARC_FEED(mid_x, mid_y, start_x, start_y, ((side == LEFT) ? -1 : 1),
                settings.current.z
                , AA_end
                , BB_end
                , CC_end
                );
            ARC_FEED(end_x, end_y, center_x, center_y, turn,
                end_z
                , AA_end
                , BB_end
                , CC_end
                );
        }
        else                                      /* one arc needed */
        {
            if (settings.feed_mode == INVERSE_TIME)
                inverse_time_rate_arc(settings.current.x, settings.current.y,
                    settings.current.z, center_x, center_y, turn,
                    end_x, end_y, end_z, block, settings);
            ARC_FEED(end_x, end_y, center_x, center_y, turn,
                end_z
                , AA_end
                , BB_end
                , CC_end
                );
        }

        settings.current.x = end_x;
        settings.current.y = end_y;
        settings.current.z = end_z;
        settings.current.a = AA_end;       /*AA*/
        settings.current.b = BB_end;       /*BB*/
        settings.current.c = CC_end;       /*CC*/
    }

   /****************************************************************************/

   /* convert_axis_offsets

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The function is called when cutter radius compensation is on:
   NCE_CANNOT_CHANGE_AXIS_OFFSETS_WITH_CUTTER_RADIUS_COMP
   2. The g_code argument is not G_92, G_92_1, G_92_2, or G_92_3
   NCE_BUG_CODE_NOT_IN_G92_SERIES

   Side effects:
   SET_PROGRAM_ORIGIN is called, and the coordinate
   values for the axis offsets are reset. The coordinates of the
   current point are reset. Parameters may be set.

   Called by: convert_modal_0.

   The action of G92 is described in [NCMS, pages 10 - 11] and {Fanuc,
   pages 61 - 63]. [NCMS] is ambiguous about the intent, but [Fanuc]
   is clear. When G92 is executed, an offset of the origin is calculated
   so that the coordinates of the current point with respect to the moved
   origin are as specified on the line containing the G92. If an axis
   is not mentioned on the line, the coordinates of the current point
   are not changed. The execution of G92 results in an axis offset being
   calculated and saved for each of the six axes, and the axis offsets
   are always used when motion is specified with respect to absolute
   distance mode using any of the nine coordinate systems (those designated
   by G54 - G59.3). Thus all nine coordinate systems are affected by G92.

   Being in incremental distance mode has no effect on the action of G92
   in this implementation. [NCMS] is not explicit about this, but it is
   implicit in the second sentence of [Fanuc, page 61].

   The offset is the amount the origin must be moved so that the
   coordinate of the controlled point has the specified value. For
   example, if the current point is at X=4 in the currently specified
   coordinate system and the current X-axis offset is zero, then "G92 x7"
   causes the X-axis offset to be reset to -3.

   Since a non-zero offset may be already be in effect when the G92 is
   called, that must be taken into account.

   In addition to causing the axis offset values in the _setup model to be
   set, G92 sets parameters 5211 to 5216 to the x,y,z,a,b,c axis offsets.

   The action of G92.2 is described in [NCMS, page 12]. There is no
   equivalent command in [Fanuc]. G92.2 resets axis offsets to zero.
   G92.1, also included in [NCMS, page 12] (but the usage here differs
   slightly from the spec), is like G92.2, except that it also causes
   the axis offset parameters to be set to zero, whereas G92.2 does not
   zero out the parameters.

   G92.3 is not in [NCMS]. It sets the axis offset values to the values
   given in the parameters.

   */

    void rs274ngc::convert_axis_offsets(              /* ARGUMENTS                               */
    int g_code,                                   /* g_code being executed (must be in G_92 series) */
    block_t& block,                          /* pointer to a block of RS274/NGC instructions   */
    setup_t& settings)                       /* pointer to machine settings                    */
    {
        double * pars;                            /* short name for settings.parameters            */

        error_if((settings.cutter_comp_side != OFF),/* not "IS ON" */
            NCE_CANNOT_CHANGE_AXIS_OFFSETS_WITH_CUTTER_RADIUS_COMP);
        pars = settings.parameters;
        if (g_code == G_92)
        {
            if (block.x_flag == ON)
            {
                settings.axis_offset.x =
                    (settings.current.x + settings.axis_offset.x - block.x_number);
                settings.current.x = block.x_number;
            }

            if (block.y_flag == ON)
            {
                settings.axis_offset.y =
                    (settings.current.y + settings.axis_offset.y - block.y_number);
                settings.current.y = block.y_number;
            }

            if (block.z_flag == ON)
            {
                settings.axis_offset.z =
                    (settings.current.z + settings.axis_offset.z - block.z_number);
                settings.current.z = block.z_number;
            }

            if (block.a_flag == ON)              /*AA*/
            {                                     /*AA*/
                settings.axis_offset.a = (settings.current.a +
                    settings.axis_offset.a - block.a_number);
                settings.current.a = block.a_number;
            }

            if (block.b_flag == ON)              /*BB*/
            {                                     /*BB*/
                settings.axis_offset.b = (settings.current.b +
                    settings.axis_offset.b - block.b_number);
                settings.current.b = block.b_number;
            }

            if (block.c_flag == ON)              /*CC*/
            {                                     /*CC*/
                settings.axis_offset.c = (settings.current.c +
                    settings.axis_offset.c - block.c_number);
                settings.current.c = block.c_number;
            }

            SET_ORIGIN_OFFSETS(settings.origin_offset.x + settings.axis_offset.x,
                settings.origin_offset.y + settings.axis_offset.y,
                settings.origin_offset.z + settings.axis_offset.z
                ,                      (settings.origin_offset.a + settings.axis_offset.a)
                ,                      (settings.origin_offset.b + settings.axis_offset.b)
                ,                      (settings.origin_offset.c + settings.axis_offset.c)
                );
            pars[5211] = settings.axis_offset.x;
            pars[5212] = settings.axis_offset.y;
            pars[5213] = settings.axis_offset.z;
            pars[5214] = settings.axis_offset.a;
            pars[5215] = settings.axis_offset.b;
            pars[5216] = settings.axis_offset.c;
        }
        else if ((g_code == G_92_1) or (g_code == G_92_2))
        {
            settings.current.x =
                settings.current.x + settings.axis_offset.x;
            settings.current.y =
                settings.current.y + settings.axis_offset.y;
            settings.current.z =
                settings.current.z + settings.axis_offset.z;
            settings.current.a =           /*AA*/
                (settings.current.a + settings.axis_offset.a);
            settings.current.b =           /*BB*/
                (settings.current.b + settings.axis_offset.b);
            settings.current.c =           /*CC*/
                (settings.current.c + settings.axis_offset.c);
            SET_ORIGIN_OFFSETS(settings.origin_offset.x,
                settings.origin_offset.y,
                settings.origin_offset.z
                ,            settings.origin_offset.a
                ,            settings.origin_offset.b
                ,            settings.origin_offset.c
                );
            settings.axis_offset.x = 0.0;
            settings.axis_offset.y = 0.0;
            settings.axis_offset.z = 0.0;
            settings.axis_offset.a = 0.0;  /*AA*/
            settings.axis_offset.b = 0.0;  /*BB*/
            settings.axis_offset.c = 0.0;  /*CC*/
            if (g_code == G_92_1)
            {
                pars[5211] = 0.0;
                pars[5212] = 0.0;
                pars[5213] = 0.0;
                pars[5214] = 0.0;            /*AA*/
                pars[5215] = 0.0;            /*BB*/
                pars[5216] = 0.0;            /*CC*/
            }
        }
        else if (g_code == G_92_3)
        {
            settings.current.x =
                settings.current.x + settings.axis_offset.x - pars[5211];
            settings.current.y =
                settings.current.y + settings.axis_offset.y - pars[5212];
            settings.current.z =
                settings.current.z + settings.axis_offset.z - pars[5213];
            settings.current.a =           /*AA*/
                settings.current.a + settings.axis_offset.a - pars[5214];
            settings.current.b =           /*BB*/
                settings.current.b + settings.axis_offset.b - pars[5215];
            settings.current.c =           /*CC*/
                settings.current.c + settings.axis_offset.c - pars[5216];
            settings.axis_offset.x = pars[5211];
            settings.axis_offset.y = pars[5212];
            settings.axis_offset.z = pars[5213];
            settings.axis_offset.a = pars[5214];
            settings.axis_offset.b = pars[5215];
            settings.axis_offset.c = pars[5216];
            SET_ORIGIN_OFFSETS(settings.origin_offset.x + settings.axis_offset.x,
                settings.origin_offset.y + settings.axis_offset.y,
                settings.origin_offset.z + settings.axis_offset.z
                ,                      (settings.origin_offset.a + settings.axis_offset.a)
                ,                      (settings.origin_offset.b + settings.axis_offset.b)
                ,                      (settings.origin_offset.c + settings.axis_offset.c)
                );
        }
        else
            throw error(NCE_BUG_CODE_NOT_IN_G92_SERIES);
    }

   /****************************************************************************/

   /* convert_comment

   Returned Value: int (RS274NGC_OK)

   Side effects:
   The message function is called if the string starts with "MSG,".
   Otherwise, the comment function is called.

   Called by: execute_block

   To be a message, the first four characters of the comment after the
   opening left parenthesis must be "MSG,", ignoring the case of the
   letters and allowing spaces or tabs anywhere before the comma (to make
   the treatment of case and white space consistent with how it is
   handled elsewhere).

   Messages are not provided for in [NCMS]. They are implemented here as a
   subtype of comment. This is an extension to the rs274NGC language.

   */

    void rs274ngc::convert_comment(                   /*ARGUMENTS            */
    char * comment)                               /* string with comment */
    {
        int m;
        int item;

        for (m = 0; ((item = comment[m]) == ' ') or (item == '\t') ; m++);
        if ((item != 'M') and (item != 'm'))
        {
            COMMENT(comment);
            return;
        }
        for (m++; ((item = comment[m]) == ' ') or (item == '\t') ; m++);
        if ((item != 'S') and (item != 's'))
        {
            COMMENT(comment);
            return;
        }
        for (m++; ((item = comment[m]) == ' ') or (item == '\t') ; m++);
        if ((item != 'G') and (item != 'g'))
        {
            COMMENT(comment);
            return;
        }
        for (m++; ((item = comment[m]) == ' ') or (item == '\t') ; m++);
        if (item != ',')
        {
            COMMENT(comment);
            return;
        }
        MESSAGE(comment + m + 1);
    }

   /****************************************************************************/

   /* convert_control_mode

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. g_code isn't G_61, G_61_1 or G_64: NCE_BUG_CODE_NOT_G61_G61_1_OR_G64

   Side effects: See below

   Called by: convert_g.

   The interpreter switches the machine settings to indicate the
   control mode (CANON_EXACT_STOP, CANON_EXACT_PATH or CANON_CONTINUOUS).

   A call is made to SET_MOTION_CONTROL_MODE(CANON_XXX), where CANON_XXX is
   CANON_EXACT_PATH if g_code is G_61, CANON_EXACT_STOP if g_code is G_61_1,
   and CANON_CONTINUOUS if g_code is G_64.

   Setting the control mode to CANON_EXACT_STOP on G_61 would correspond
   more closely to the meaning of G_61 as given in [NCMS, page 40], but
   CANON_EXACT_PATH has the advantage that the tool does not stop if it
   does not have to, and no evident disadvantage compared to
   CANON_EXACT_STOP, so it is being used for G_61. G_61_1 is not defined
   in [NCMS], so it is available and is used here for setting the control
   mode to CANON_EXACT_STOP.

   It is OK to call SET_MOTION_CONTROL_MODE(CANON_XXX) when CANON_XXX is
   already in force.

   */

    void rs274ngc::convert_control_mode(              /* ARGUMENTS                             */
    int g_code,                                   /* g_code being executed (G_61, G61_1, or G_64) */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        if (g_code == G_61)
        {
            SET_MOTION_CONTROL_MODE(CANON_EXACT_PATH);
            settings.control_mode = CANON_EXACT_PATH;
        }
        else if (g_code == G_61_1)
        {
            SET_MOTION_CONTROL_MODE(CANON_EXACT_STOP);
            settings.control_mode = CANON_EXACT_STOP;
        }
        else if (g_code == G_64)
        {
            SET_MOTION_CONTROL_MODE(CANON_CONTINUOUS);
            settings.control_mode = CANON_CONTINUOUS;
        }
        else
            throw error(NCE_BUG_CODE_NOT_G61_G61_1_OR_G64);
    }

   /****************************************************************************/

   /* convert_coordinate_system

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The value of the g_code argument is not 540, 550, 560, 570, 580, 590
   591, 592, or 593:
   NCE_BUG_CODE_NOT_IN_RANGE_G54_TO_G593

   Side effects:
   If the coordinate system selected by the g_code is not already in
   use, the canonical program coordinate system axis offset values are
   reset and the coordinate values of the current point are reset.

   Called by: convert_g.

   COORDINATE SYSTEMS (involves g10, g53, g54 - g59.3, g92)

   The canonical machining functions view of coordinate systems is:
   1. There are two coordinate systems: absolute and program.
   2. All coordinate values are given in terms of the program coordinate system.
   3. The offsets of the program coordinate system may be reset.

   The RS274/NGC view of coordinate systems, as given in section 3.2
   of [NCMS] is:
   1. there are ten coordinate systems: absolute and 9 program. The
   program coordinate systems are numbered 1 to 9.
   2. you can switch among the 9 but not to the absolute one. G54
   selects coordinate system 1, G55 selects 2, and so on through
   G56, G57, G58, G59, G59.1, G59.2, and G59.3.
   3. you can set the offsets of the 9 program coordinate systems
   using G10 L2 Pn (n is the number of the coordinate system) with
   values for the axes in terms of the absolute coordinate system.
   4. the first one of the 9 program coordinate systems is the default.
   5. data for coordinate systems is stored in parameters [NCMS, pages 59 - 60].
   6. g53 means to interpret coordinate values in terms of the absolute
   coordinate system for the one block in which g53 appears.
   7. You can offset the current coordinate system using g92. This offset
   will then apply to all nine program coordinate systems.

   The approach used in the interpreter mates the canonical and NGC views
   of coordinate systems as follows:

   During initialization, data from the parameters for the first NGC
   coordinate system is used in a SET_ORIGIN_OFFSETS function call and
   origin_index in the machine model is set to 1.

   If a g_code in the range g54 - g59.3 is encountered in an NC program,
   the data from the appropriate NGC coordinate system is copied into the
   origin offsets used by the interpreter, a SET_ORIGIN_OFFSETS function
   call is made, and the current position is reset.

   If a g10 is encountered, the convert_setup function is called to reset
   the offsets of the program coordinate system indicated by the P number
   given in the same block.

   If a g53 is encountered, the axis values given in that block are used
   to calculate what the coordinates are of that point in the current
   coordinate system, and a STRAIGHT_TRAVERSE or STRAIGHT_FEED function
   call to that point using the calculated values is made. No offset
   values are changed.

   If a g92 is encountered, that is handled by the convert_axis_offsets
   function. A g92 results in an axis offset for each axis being calculated
   and stored in the machine model. The axis offsets are applied to all
   nine coordinate systems. Axis offsets are initialized to zero.

   */

    void rs274ngc::convert_coordinate_system(         /* ARGUMENTS                         */
    int g_code,                                   /* g_code called (must be one listed above)      */
    setup_t& settings)                       /* pointer to machine settings                   */
    {
        int origin;
        double x;
        double y;
        double z;
        double a;                                 /*AA*/
        double b;                                 /*BB*/
        double c;                                 /*CC*/
        double * parameters;

        parameters = settings.parameters;
        switch(g_code)
        {
            case 540:
                origin = 1;
                break;
            case 550:
                origin = 2;
                break;
            case 560:
                origin = 3;
                break;
            case 570:
                origin = 4;
                break;
            case 580:
                origin = 5;
                break;
            case 590:
                origin = 6;
                break;
            case 591:
                origin = 7;
                break;
            case 592:
                origin = 8;
                break;
            case 593:
                origin = 9;
                break;
            default:
                throw error(NCE_BUG_CODE_NOT_IN_RANGE_G54_TO_G593);
        }

        if (origin == settings.origin_index)     /* already using this origin */
        {
#ifdef DEBUG_EMC
            COMMENT("interpreter: continuing to use same coordinate system");
#endif
            return;
        }

        settings.origin_index = origin;
        parameters[5220] = (double)origin;

   /* axis offsets could be included in the two set of calculations for
      current_x, current_y, etc., but do not need to be because the results
      would be the same. They would be added in then subtracted out. */
        settings.current.x =
            (settings.current.x + settings.origin_offset.x);
        settings.current.y =
            (settings.current.y + settings.origin_offset.y);
        settings.current.z =
            (settings.current.z + settings.origin_offset.z);
        settings.current.a =               /*AA*/
            (settings.current.a + settings.origin_offset.a);
        settings.current.b =               /*BB*/
            (settings.current.b + settings.origin_offset.b);
        settings.current.c =               /*CC*/
            (settings.current.c + settings.origin_offset.c);

        x = parameters[5201 + (origin * 20)];
        y = parameters[5202 + (origin * 20)];
        z = parameters[5203 + (origin * 20)];
        a = parameters[5204 + (origin * 20)];/*AA*/
        b = parameters[5205 + (origin * 20)];/*BB*/
        c = parameters[5206 + (origin * 20)];/*CC*/

        settings.origin_offset.x = x;
        settings.origin_offset.y = y;
        settings.origin_offset.z = z;
        settings.origin_offset.a = a;      /*AA*/
        settings.origin_offset.b = b;      /*BB*/
        settings.origin_offset.c = c;      /*CC*/

        settings.current.x = (settings.current.x - x);
        settings.current.y = (settings.current.y - y);
        settings.current.z = (settings.current.z - z);
        settings.current.a = (settings.current.a - a);
        settings.current.b = (settings.current.b - b);
        settings.current.c = (settings.current.c - c);

        SET_ORIGIN_OFFSETS(x + settings.axis_offset.x,
            y + settings.axis_offset.y,
            z + settings.axis_offset.z
            ,            a + settings.axis_offset.a
            ,            b + settings.axis_offset.b
            ,            c + settings.axis_offset.c
            );
    }

   /****************************************************************************/

   /* convert_cutter_compensation

   Returned Value: int
   If convert_cutter_compensation_on or convert_cutter_compensation_off
   is called and returns an error code, this returns that code.
   If any of the following errors occur, this returns the error shown.
   Otherwise, it returns RS274NGC_OK.
   1. g_code is not G_40, G_41, or G_42:
   NCE_BUG_CODE_NOT_G40_G41_OR_G42

   Side effects:
   The value of cutter_comp_side in the machine model mode is
   set to RIGHT, LEFT, or OFF. The currently active tool table index in
   the machine model (which is the index of the slot whose diameter
   value is used in cutter radius compensation) is updated.

   Since cutter radius compensation is performed in the interpreter, no
   call is made to any canonical function regarding cutter radius compensation.

   Called by: convert_g

   */

    void rs274ngc::convert_cutter_compensation(       /* ARGUMENTS                  */
    int g_code,                                   /* must be G_40, G_41, or G_42              */
    block_t& block,                          /* pointer to a block of RS274 instructions */
    setup_t& settings)                       /* pointer to machine settings              */
    {
        if (g_code == G_40)
        {
            convert_cutter_compensation_off(settings);
        }
        else if (g_code == G_41)
        {
            convert_cutter_compensation_on(LEFT, block, settings);
        }
        else if (g_code == G_42)
        {
            convert_cutter_compensation_on(RIGHT, block, settings);
        }
        else
            throw error(NCE_BUG_CODE_NOT_G40_G41_OR_G42);
    }

   /****************************************************************************/

   /* convert_cutter_compensation_off

   Returned Value: int (RS274NGC_OK)

   Side effects:
   A comment is made that cutter radius compensation is turned off.
   The machine model of the cutter radius compensation mode is set to OFF.
   The value of program_x in the machine model is set to UNKNOWN.
   This serves as a flag when cutter radius compensation is
   turned on again.

   Called by: convert_cutter_compensation

   */

    void rs274ngc::convert_cutter_compensation_off(   /* ARGUMENTS                   */
    setup_t& settings)                       /* pointer to machine settings */
    {
#ifdef DEBUG_EMC
        COMMENT("interpreter: cutter radius compensation off");
#endif
        settings.cutter_comp_side = OFF;
        settings.program_x = UNKNOWN;
    }

   /****************************************************************************/

   /* convert_cutter_compensation_on

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The selected plane is not the XY plane:
   NCE_CANNOT_TURN_CUTTER_RADIUS_COMP_ON_OUT_OF_XY_PLANE
   2. Cutter radius compensation is already on:
   NCE_CANNOT_TURN_CUTTER_RADIUS_COMP_ON_WHEN_ON

   Side effects:
   A COMMENT function call is made (conditionally) saying that the
   interpreter is switching mode so that cutter radius compensation is on.
   The value of cutter_comp_radius in the machine model mode is
   set to the absolute value of the radius given in the tool table.
   The value of cutter_comp_side in the machine model mode is
   set to RIGHT or LEFT. The currently active tool table index in
   the machine model is updated.

   Called by: convert_cutter_compensation

   check_other_codes checks that a d word occurs only in a block with g41
   or g42.

   Cutter radius compensation is carried out in the interpreter, so no
   call is made to a canonical function (although there is a canonical
   function, START_CUTTER_RADIUS_COMPENSATION, that could be called if
   the primitive level could execute it).

   This version uses a D word if there is one in the block, but it does
   not require a D word, since the sample programs which the interpreter
   is supposed to handle do not have them.  Logically, the D word is
   optional, since the D word is always (except in cases we have never
   heard of) the slot number of the tool in the spindle. Not requiring a
   D word is contrary to [Fanuc, page 116] and [NCMS, page 79], however.
   Both manuals require the use of the D-word with G41 and G42.

   This version handles a negative offset radius, which may be
   encountered if the programmed tool path is a center line path for
   cutting a profile and the path was constructed using a nominal tool
   diameter. Then the value in the tool table for the diameter is set to
   be the difference between the actual diameter and the nominal
   diameter. If the actual diameter is less than the nominal, the value
   in the table is negative. The method of handling a negative radius is
   to switch the side of the offset and use a positive radius. This
   requires that the profile use arcs (not straight lines) to go around
   convex corners.

   */

    void rs274ngc::convert_cutter_compensation_on(    /* ARGUMENTS               */
    int side,                                     /* side of path cutter is on (LEFT or RIGHT) */
    block_t& block,                          /* pointer to a block of RS274 instructions  */
    setup_t& settings)                       /* pointer to machine settings               */
    {
        double radius;
        int index;

        error_if((settings.plane != CANON_PLANE_XY), NCE_CANNOT_TURN_CUTTER_RADIUS_COMP_ON_OUT_OF_XY_PLANE);
        error_if((settings.cutter_comp_side != OFF), NCE_CANNOT_TURN_CUTTER_RADIUS_COMP_ON_WHEN_ON);
        index =
            (block.d_number != -1) ? block.d_number : settings.current_slot;
        radius = ((settings.tool_table[index].diameter)/2.0);

        if (radius < 0.0)                         /* switch side & make radius positive if radius negative */
        {
            radius = -radius;
            if (side == RIGHT)
                side = LEFT;
            else
                side = RIGHT;
        }

#ifdef DEBUG_EMC
        if (side == RIGHT)
            COMMENT("interpreter: cutter radius compensation on right");
        else
            COMMENT("interpreter: cutter radius compensation on left");
#endif

        settings.cutter_comp_radius = radius;
        settings.tool_table_index = index;
        settings.cutter_comp_side = side;
    }

   /****************************************************************************/

   /* convert_cycle

   Returned Value: int
   If any of the specific functions called returns an error code,
   this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The r-value is not given the first time this code is called after
   some other motion mode has been in effect:
   NCE_R_CLEARANCE_PLANE_UNSPECIFIED_IN_CYCLE
   2. The l number is zero: NCE_CANNOT_DO_ZERO_REPEATS_OF_CYCLE
   3. The currently selected plane in not XY, YZ, or XZ.
   NCE_BUG_PLANE_NOT_XY_YZ_OR_XZ

   Side effects:
   A number of moves are made to execute a canned cycle. The current
   position is reset. The values of the cycle attributes in the settings
   may be reset.

   Called by: convert_motion

   This function makes a couple checks and then calls one of three
   functions, according to which plane is currently selected.

   See the documentation of convert_cycle_xy for most of the details.

   */

    void rs274ngc::convert_cycle(                     /* ARGUMENTS                                      */
    int motion,                                   /* a g-code between G_81 and G_89, a canned cycle */
    block_t& block,                          /* pointer to a block of RS274 instructions       */
    setup_t& settings)                       /* pointer to machine settings                    */
    {
        CANON_PLANE plane;

        plane = settings.plane;
        if (block.r_flag == OFF)
        {
            if (settings.motion_mode == motion)
                block.r_number = settings.cycle.r;
            else
                throw error(NCE_R_CLEARANCE_PLANE_UNSPECIFIED_IN_CYCLE);
        }

        error_if((block.l_number == 0), NCE_CANNOT_DO_ZERO_REPEATS_OF_CYCLE);
        if (block.l_number == -1)
            block.l_number = 1;

        if (plane == CANON_PLANE_XY)
        {
            convert_cycle_xy(motion, block, settings);
        }
        else if (plane == CANON_PLANE_YZ)
        {
            convert_cycle_yz(motion, block, settings);
        }
        else if (plane == CANON_PLANE_XZ)
        {
            convert_cycle_zx(motion, block, settings);
        }
        else
            throw error(NCE_BUG_PLANE_NOT_XY_YZ_OR_XZ);

        settings.cycle.l = block.l_number;
        settings.cycle.r = block.r_number;
        settings.motion_mode = motion;
    }

   /****************************************************************************/

   /* convert_cycle_g81

   Returned Value: int (RS274NGC_OK)

   Side effects: See below

   Called by:
   convert_cycle_xy
   convert_cycle_yz
   convert_cycle_zx

   For the XY plane, this implements the following RS274/NGC cycle, which
   is usually drilling:
   1. Move the z-axis only at the current feed rate to the specified bottom_z.
   2. Retract the z-axis at traverse rate to clear_z.

   See [NCMS, page 99].

   CYCLE_MACRO has positioned the tool at (x, y, r, a, b, c) when this starts.

   For the XZ and YZ planes, this makes analogous motions.

   */

    void rs274ngc::convert_cycle_g81(                 /* ARGUMENTS                        */
    CANON_PLANE plane,                            /* selected plane                   */
    double x,                                     /* x-value where cycle is executed  */
    double y,                                     /* y-value where cycle is executed  */
    double clear_z,                               /* z-value of clearance plane       */
    double bottom_z)                              /* value of z at bottom of cycle    */
    {
        cycle_feed(plane, x, y, bottom_z);
        cycle_traverse(plane, x, y, clear_z);
    }

   /****************************************************************************/

   /* convert_cycle_g82

   Returned Value: int (RS274NGC_OK)

   Side effects: See below

   Called by:
   convert_cycle_xy
   convert_cycle_yz
   convert_cycle_zx

   For the XY plane, this implements the following RS274/NGC cycle, which
   is usually drilling:
   1. Move the z_axis only at the current feed rate to the specified z-value.
   2. Dwell for the given number of seconds.
   3. Retract the z-axis at traverse rate to the clear_z.

   CYCLE_MACRO has positioned the tool at (x, y, r, a, b, c) when this starts.

   For the XZ and YZ planes, this makes analogous motions.

   */

    void rs274ngc::convert_cycle_g82(                 /* ARGUMENTS                        */
    CANON_PLANE plane,                            /* selected plane                   */
    double x,                                     /* x-value where cycle is executed  */
    double y,                                     /* y-value where cycle is executed  */
    double clear_z,                               /* z-value of clearance plane       */
    double bottom_z,                              /* value of z at bottom of cycle    */
    double dwell)                                 /* dwell time                       */
    {
        cycle_feed(plane, x, y, bottom_z);
        DWELL(dwell);
        cycle_traverse(plane, x, y, clear_z);
    }

   /****************************************************************************/

   /* convert_cycle_g83

   Returned Value: int (RS274NGC_OK)

   Side effects: See below

   Called by:
   convert_cycle_xy
   convert_cycle_yz
   convert_cycle_zx

   For the XY plane, this implements the following RS274/NGC cycle,
   which is usually peck drilling:
   1. Move the z-axis only at the current feed rate downward by delta or
   to the specified bottom_z, whichever is less deep.
   2. Rapid back out to the clear_z.
   3. Rapid back down to the current hole bottom, backed off a bit.
   4. Repeat steps 1, 2, and 3 until the specified bottom_z is reached.
   5. Retract the z-axis at traverse rate to clear_z.

   CYCLE_MACRO has positioned the tool at (x, y, r, a, b, c) when this starts.

   The rapid out and back in causes any long stringers (which are common
   when drilling in aluminum) to be cut off and clears chips from the
   hole.

   For the XZ and YZ planes, this makes analogous motions.

   */

#define G83_RAPID_DELTA 0.010
   /* how far above hole bottom for rapid
                      return, in inches */

    void rs274ngc::convert_cycle_g83(                 /* ARGUMENTS                        */
    CANON_PLANE plane,                            /* selected plane                   */
    double x,                                     /* x-value where cycle is executed  */
    double y,                                     /* y-value where cycle is executed  */
    double r,                                     /* initial z-value                  */
    double clear_z,                               /* z-value of clearance plane       */
    double bottom_z,                              /* value of z at bottom of cycle    */
    double delta)                                 /* size of z-axis feed increment    */
    {
        double current_depth;
        double rapid_delta;

        rapid_delta = G83_RAPID_DELTA;
        if (_setup.length_units == CANON_UNITS_MM)
            rapid_delta = (rapid_delta * 25.4);

        for (current_depth = (r - delta);
            current_depth > bottom_z;
            current_depth = (current_depth - delta))
        {
            cycle_feed(plane, x, y, current_depth);
            cycle_traverse(plane, x, y, clear_z);
            cycle_traverse(plane, x, y, current_depth + rapid_delta);
        }
        cycle_feed(plane, x, y, bottom_z);
        cycle_traverse(plane, x, y, clear_z);
    }

   /****************************************************************************/

   /* convert_cycle_g84

   Returned Value: int
   If the spindle is not turning clockwise, this returns
   NCE_SPINDLE_NOT_TURNING_CLOCKWISE_IN_G84.
   Otherwise, it returns RS274NGC_OK.

   Side effects: See below

   Called by:
   convert_cycle_xy
   convert_cycle_yz
   convert_cycle_zx

   For the XY plane, this implements the following RS274/NGC cycle,
   which is right-hand tapping:
   1. Start speed-feed synchronization.
   2. Move the z-axis only at the current feed rate to the specified bottom_z.
   3. Stop the spindle.
   4. Start the spindle counterclockwise.
   5. Retract the z-axis at current feed rate to clear_z.
   6. If speed-feed synch was not on before the cycle started, stop it.
   7. Stop the spindle.
   8. Start the spindle clockwise.

   CYCLE_MACRO has positioned the tool at (x, y, r, a, b, c) when this starts.
   The direction argument must be clockwise.

   For the XZ and YZ planes, this makes analogous motions.

   */

    void rs274ngc::convert_cycle_g84(                 /* ARGUMENTS                           */
    CANON_PLANE plane,                            /* selected plane                      */
    double x,                                     /* x-value where cycle is executed     */
    double y,                                     /* y-value where cycle is executed     */
    double clear_z,                               /* z-value of clearance plane          */
    double bottom_z,                              /* value of z at bottom of cycle       */
    CANON_DIRECTION direction,                    /* direction spindle turning at outset */
    CANON_SPEED_FEED_MODE mode)                   /* the speed-feed mode at outset       */
    {
        error_if((direction != CANON_CLOCKWISE), NCE_SPINDLE_NOT_TURNING_CLOCKWISE_IN_G84);
        START_SPEED_FEED_SYNCH();
        cycle_feed(plane, x, y, bottom_z);
        STOP_SPINDLE_TURNING();
        START_SPINDLE_COUNTERCLOCKWISE();
        cycle_feed(plane, x, y, clear_z);
        if (mode != CANON_SYNCHED)
            STOP_SPEED_FEED_SYNCH();
        STOP_SPINDLE_TURNING();
        START_SPINDLE_CLOCKWISE();
    }

   /****************************************************************************/

   /* convert_cycle_g85

   Returned Value: int (RS274NGC_OK)

   Side effects:
   A number of moves are made as described below.

   Called by:
   convert_cycle_xy
   convert_cycle_yz
   convert_cycle_zx

   For the XY plane, this implements the following RS274/NGC cycle,
   which is usually boring or reaming:
   1. Move the z-axis only at the current feed rate to the specified z-value.
   2. Retract the z-axis at the current feed rate to clear_z.

   CYCLE_MACRO has positioned the tool at (x, y, r, ?, ?) when this starts.

   For the XZ and YZ planes, this makes analogous motions.

   */

    void rs274ngc::convert_cycle_g85(                 /* ARGUMENTS                        */
    CANON_PLANE plane,                            /* selected plane                   */
    double x,                                     /* x-value where cycle is executed  */
    double y,                                     /* y-value where cycle is executed  */
    double clear_z,                               /* z-value of clearance plane       */
    double bottom_z)                              /* value of z at bottom of cycle    */
    {
        cycle_feed(plane, x, y, bottom_z);
        cycle_feed(plane, x, y, clear_z);
    }

   /****************************************************************************/

   /* convert_cycle_g86

   Returned Value: int
   If the spindle is not turning clockwise or counterclockwise,
   this returns NCE_SPINDLE_NOT_TURNING_IN_G86.
   Otherwise, it returns RS274NGC_OK.

   Side effects:
   A number of moves are made as described below.

   Called by:
   convert_cycle_xy
   convert_cycle_yz
   convert_cycle_zx

   For the XY plane, this implements the RS274/NGC following cycle,
   which is usually boring:
   1. Move the z-axis only at the current feed rate to bottom_z.
   2. Dwell for the given number of seconds.
   3. Stop the spindle turning.
   4. Retract the z-axis at traverse rate to clear_z.
   5. Restart the spindle in the direction it was going.

   CYCLE_MACRO has positioned the tool at (x, y, r, a, b, c) when this starts.

   For the XZ and YZ planes, this makes analogous motions.

   */

    void rs274ngc::convert_cycle_g86(                 /* ARGUMENTS                           */
    CANON_PLANE plane,                            /* selected plane                      */
    double x,                                     /* x-value where cycle is executed     */
    double y,                                     /* y-value where cycle is executed     */
    double clear_z,                               /* z-value of clearance plane          */
    double bottom_z,                              /* value of z at bottom of cycle       */
    double dwell,                                 /* dwell time                          */
    CANON_DIRECTION direction)                    /* direction spindle turning at outset */
    {
        error_if(((direction != CANON_CLOCKWISE) and
            (direction != CANON_COUNTERCLOCKWISE)),
            NCE_SPINDLE_NOT_TURNING_IN_G86);

        cycle_feed(plane, x, y, bottom_z);
        DWELL(dwell);
        STOP_SPINDLE_TURNING();
        cycle_traverse(plane, x, y, clear_z);
        if (direction == CANON_CLOCKWISE)
            START_SPINDLE_CLOCKWISE();
        else
            START_SPINDLE_COUNTERCLOCKWISE();
    }

   /****************************************************************************/

   /* convert_cycle_g87

   Returned Value: int
   If the spindle is not turning clockwise or counterclockwise,
   this returns NCE_SPINDLE_NOT_TURNING_IN_G87.
   Otherwise, it returns RS274NGC_OK.

   Side effects:
   A number of moves are made as described below. This cycle is a
   modified version of [Monarch, page 5-24] since [NCMS, pages 98 - 100]
   gives no clue as to what the cycle is supposed to do. [KT] does not
   have a back boring cycle. [Fanuc, page 132] in "Canned cycle II"
   describes the G87 cycle as given here, except that the direction of
   spindle turning is always clockwise and step 7 below is omitted
   in [Fanuc].

   Called by:
   convert_cycle_xy
   convert_cycle_yz
   convert_cycle_zx

   For the XY plane, this implements the following RS274/NGC cycle, which
   is usually back boring.  The situation is that you have a through hole
   and you want to counterbore the bottom of hole. To do this you put an
   L-shaped tool in the spindle with a cutting surface on the UPPER side
   of its base. You stick it carefully through the hole when it is not
   spinning and is oriented so it fits through the hole, then you move it
   so the stem of the L is on the axis of the hole, start the spindle,
   and feed the tool upward to make the counterbore. Then you get the
   tool out of the hole.

   1. Move at traverse rate parallel to the XY-plane to the point
   with x-value offset_x and y-value offset_y.
   2. Stop the spindle in a specific orientation.
   3. Move the z-axis only at traverse rate downward to the bottom_z.
   4. Move at traverse rate parallel to the XY-plane to the x,y location.
   5. Start the spindle in the direction it was going before.
   6. Move the z-axis only at the given feed rate upward to the middle_z.
   7. Move the z-axis only at the given feed rate back down to bottom_z.
   8. Stop the spindle in the same orientation as before.
   9. Move at traverse rate parallel to the XY-plane to the point
   with x-value offset_x and y-value offset_y.
   10. Move the z-axis only at traverse rate to the clear z value.
   11. Move at traverse rate parallel to the XY-plane to the specified x,y
   location.
   12. Restart the spindle in the direction it was going before.

   CYCLE_MACRO has positioned the tool at (x, y, r, a, b, c) before this starts.

   It might be useful to add a check that clear_z > middle_z > bottom_z.
   Without the check, however, this can be used to counterbore a hole in
   material that can only be accessed through a hole in material above it.

   For the XZ and YZ planes, this makes analogous motions.

   */

    void rs274ngc::convert_cycle_g87(                 /* ARGUMENTS                           */
    CANON_PLANE plane,                            /* selected plane                      */
    double x,                                     /* x-value where cycle is executed     */
    double offset_x,                              /* x-axis offset position              */
    double y,                                     /* y-value where cycle is executed     */
    double offset_y,                              /* y-axis offset position              */
    double r,                                     /* z_value of r_plane                  */
    double clear_z,                               /* z-value of clearance plane          */
    double middle_z,                              /* z-value of top of back bore         */
    double bottom_z,                              /* value of z at bottom of cycle       */
    CANON_DIRECTION direction)                    /* direction spindle turning at outset */
    {
        error_if(((direction != CANON_CLOCKWISE) and
            (direction != CANON_COUNTERCLOCKWISE)),
            NCE_SPINDLE_NOT_TURNING_IN_G87);

        cycle_traverse(plane, offset_x, offset_y, r);
        STOP_SPINDLE_TURNING();
        ORIENT_SPINDLE(0.0, direction);
        cycle_traverse(plane, offset_x, offset_y, bottom_z);
        cycle_traverse(plane, x, y, bottom_z);
        if (direction == CANON_CLOCKWISE)
            START_SPINDLE_CLOCKWISE();
        else
            START_SPINDLE_COUNTERCLOCKWISE();
        cycle_feed(plane, x, y, middle_z);
        cycle_feed(plane, x, y, bottom_z);
        STOP_SPINDLE_TURNING();
        ORIENT_SPINDLE(0.0, direction);
        cycle_traverse(plane, offset_x, offset_y, bottom_z);
        cycle_traverse(plane, offset_x, offset_y, clear_z);
        cycle_traverse(plane, x, y, clear_z);
        if (direction == CANON_CLOCKWISE)
            START_SPINDLE_CLOCKWISE();
        else
            START_SPINDLE_COUNTERCLOCKWISE();
    }

   /****************************************************************************/

   /* convert_cycle_g88

   Returned Value: int
   If the spindle is not turning clockwise or counterclockwise, this
   returns NCE_SPINDLE_NOT_TURNING_IN_G88.
   Otherwise, it returns RS274NGC_OK.

   Side effects: See below

   Called by:
   convert_cycle_xy
   convert_cycle_yz
   convert_cycle_zx

   For the XY plane, this implements the following RS274/NGC cycle,
   which is usually boring:
   1. Move the z-axis only at the current feed rate to the specified z-value.
   2. Dwell for the given number of seconds.
   3. Stop the spindle turning.
   4. Stop the program so the operator can retract the spindle manually.
   5. Restart the spindle.

   CYCLE_MACRO has positioned the tool at (x, y, r, a, b, c) when this starts.

   For the XZ and YZ planes, this makes analogous motions.

   */

    void rs274ngc::convert_cycle_g88(                 /* ARGUMENTS                           */
    CANON_PLANE plane,                            /* selected plane                      */
    double x,                                     /* x-value where cycle is executed     */
    double y,                                     /* y-value where cycle is executed     */
    double bottom_z,                              /* value of z at bottom of cycle       */
    double dwell,                                 /* dwell time                          */
    CANON_DIRECTION direction)                    /* direction spindle turning at outset */
    {
        error_if(((direction != CANON_CLOCKWISE) and
            (direction != CANON_COUNTERCLOCKWISE)),
            NCE_SPINDLE_NOT_TURNING_IN_G88);

        cycle_feed(plane, x, y, bottom_z);
        DWELL(dwell);
        STOP_SPINDLE_TURNING();
        PROGRAM_STOP();                           /* operator retracts the spindle here */
        if (direction == CANON_CLOCKWISE)
            START_SPINDLE_CLOCKWISE();
        else
            START_SPINDLE_COUNTERCLOCKWISE();
    }

   /****************************************************************************/

   /* convert_cycle_g89

   Returned Value: int (RS274NGC_OK)

   Side effects: See below

   Called by:
   convert_cycle_xy
   convert_cycle_yz
   convert_cycle_zx

   This implements the following RS274/NGC cycle, which is intended for boring:
   1. Move the z-axis only at the current feed rate to the specified z-value.
   2. Dwell for the given number of seconds.
   3. Retract the z-axis at the current feed rate to clear_z.

   CYCLE_MACRO has positioned the tool at (x, y, r, a, b, c) when this starts.

   For the XZ and YZ planes, this makes analogous motions.

   */

    void rs274ngc::convert_cycle_g89(                 /* ARGUMENTS                        */
    CANON_PLANE plane,                            /* selected plane                   */
    double x,                                     /* x-value where cycle is executed  */
    double y,                                     /* y-value where cycle is executed  */
    double clear_z,                               /* z-value of clearance plane       */
    double bottom_z,                              /* value of z at bottom of cycle    */
    double dwell)                                 /* dwell time                       */
    {
        cycle_feed(plane, x, y, bottom_z);
        DWELL(dwell);
        cycle_feed(plane, x, y, clear_z);
    }

   /****************************************************************************/

   /* convert_cycle_xy

   Returned Value: int
   If any of the specific functions called returns an error code,
   this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The z-value is not given the first time this code is called after
   some other motion mode has been in effect:
   NCE_Z_VALUE_UNSPECIFIED_IN_XY_PLANE_CANNED_CYCLE
   2. The r clearance plane is below the bottom_z:
   NCE_R_LESS_THAN_Z_IN_CYCLE_IN_XY_PLANE
   3. the distance mode is neither absolute or incremental:
   NCE_BUG_DISTANCE_MODE_NOT_G90_OR_G91
   4. G82, G86, G88, or G89 is called when it is not already in effect,
   and no p number is in the block:
   NCE_DWELL_TIME_P_WORD_MISSING_WITH_G82
   NCE_DWELL_TIME_P_WORD_MISSING_WITH_G86
   NCE_DWELL_TIME_P_WORD_MISSING_WITH_G88
   NCE_DWELL_TIME_P_WORD_MISSING_WITH_G89
   5. G83 is called when it is not already in effect,
   and no q number is in the block: NCE_Q_WORD_MISSING_WITH_G83
   6. G87 is called when it is not already in effect,
   and any of the i number, j number, or k number is missing:
   NCE_I_WORD_MISSING_WITH_G87
   NCE_J_WORD_MISSING_WITH_G87
   NCE_K_WORD_MISSING_WITH_G87
   7. the G code is not between G_81 and G_89.
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED

   Side effects:
   A number of moves are made to execute the g-code

   Called by: convert_cycle

   The function does not require that any of x,y,z, or r be specified in
   the block, except that if the last motion mode command executed was
   not the same as this one, the r-value and z-value must be specified.

   This function is handling the repeat feature of RS274/NGC, wherein
   the L word represents the number of repeats [NCMS, page 99]. We are
   not allowing L=0, contrary to the manual. We are allowing L > 1
   in absolute distance mode to mean "do the same thing in the same
   place several times", as provided in the manual, although this seems
   abnormal.

   In incremental distance mode, x, y, and r values are treated as
   increments to the current position and z as an increment from r.  In
   absolute distance mode, x, y, r, and z are absolute. In g87, i and j
   will always be increments, regardless of the distance mode setting, as
   implied in [NCMS, page 98], but k (z-value of top of counterbore) will
   be an absolute z-value in absolute distance mode, and an increment
   (from bottom z) in incremental distance mode.

   If the r position of a cycle is above the current_z position, this
   retracts the z-axis to the r position before moving parallel to the
   XY plane.

   In the code for this function, there is a nearly identical "for" loop
   in every case of the switch. The loop is the done with a compiler
   macro, "CYCLE_MACRO" so that the code is easy to read, automatically
   kept identical from case to case and, and much shorter than it would
   be without the macro. The loop could be put outside the switch, but
   then the switch would run every time around the loop, not just once,
   as it does here. The loop could also be placed in the called
   functions, but then it would not be clear that all the loops are the
   same, and it would be hard to keep them the same when the code is
   modified.  The macro would be very awkward as a regular function
   because it would have to be passed all of the arguments used by any of
   the specific cycles, and, if another switch in the function is to be
   avoided, it would have to passed a function pointer, but the different
   cycle functions have different arguments so the type of the pointer
   could not be declared unless the cycle functions were re-written to
   take the same arguments (in which case most of them would have several
   unused arguments).

   The motions within the CYCLE_MACRO (but outside a specific cycle) are
   a straight traverse parallel to the selected plane to the given
   position in the plane and a straight traverse of the third axis only
   (if needed) to the r position.

   The CYCLE_MACRO is defined here but is also used in convert_cycle_yz
   and convert_cycle_zx. The variables aa, bb, and cc are used in
   CYCLE_MACRO and in the other two functions just mentioned. Those
   variables represent the first axis of the selected plane, the second
   axis of the selected plane, and third axis which is perpendicular to
   the selected plane.  In this function aa represents x, bb represents
   y, and cc represents z. This usage makes it possible to have only one
   version of each of the cycle functions.  The cycle_traverse and
   cycle_feed functions help accomplish this.

   The height of the retract move at the end of each repeat of a cycle is
   determined by the setting of the retract_mode: either to the r
   position (if the retract_mode is R_PLANE) or to the original
   z-position (if that is above the r position and the retract_mode is
   not R_PLANE). This is a slight departure from [NCMS, page 98], which
   does not require checking that the original z-position is above r.

   The rotary axes may not move during a canned cycle.

   */

#define CYCLE_MACRO(call) for (repeat = block.l_number; \
repeat > 0; \
repeat--) \
{ \
    aa = (aa + aa_increment); \
    bb = (bb + bb_increment); \
    cycle_traverse(plane, aa, bb, old_cc); \
    if (old_cc != r) \
    cycle_traverse(plane, aa, bb, r); \
    call; \
    old_cc = clear_cc; \
}

    void rs274ngc::convert_cycle_xy(              /* ARGUMENTS                                 */
    int motion,                               /* a g-code between G_81 and G_89, a canned cycle */
    block_t& block,                      /* pointer to a block of RS274 instructions       */
    setup_t& settings)                   /* pointer to machine settings                    */
    {
        double aa;
        double aa_increment;
        double bb;
        double bb_increment;
        double cc;
        double clear_cc;
        double i;
        double j;
        double k;
        double old_cc;
        CANON_PLANE plane;
        double r;
        int repeat;
        CANON_MOTION_MODE save_mode;

        plane = CANON_PLANE_XY;
        if (settings.motion_mode != motion)
        {
            error_if((block.z_flag == OFF), NCE_Z_VALUE_UNSPECIFIED_IN_XY_PLANE_CANNED_CYCLE);
        }
        block.z_number =
            block.z_flag == ON ? block.z_number : settings.cycle.cc;
        old_cc = settings.current.z;

        if (settings.distance_mode == MODE_ABSOLUTE)
        {
            aa_increment = 0.0;
            bb_increment = 0.0;
            r = block.r_number;
            cc = block.z_number;
            aa = block.x_flag == ON ? block.x_number : settings.current.x;
            bb = block.y_flag == ON ? block.y_number : settings.current.y;
        }
        else if (settings.distance_mode == MODE_INCREMENTAL)
        {
            aa_increment = block.x_number;
            bb_increment = block.y_number;
            r = (block.r_number + old_cc);
            cc = (r + block.z_number);      /* [NCMS, page 98] */
            aa = settings.current.x;
            bb = settings.current.y;
        }
        else
            throw error(NCE_BUG_DISTANCE_MODE_NOT_G90_OR_G91);
        error_if((r < cc), NCE_R_LESS_THAN_Z_IN_CYCLE_IN_XY_PLANE);

        if (old_cc < r)
        {
            STRAIGHT_TRAVERSE(settings.current.x, settings.current.y, r
                ,     settings.current.a
                ,  settings.current.b
                ,  settings.current.c
                );
            old_cc = r;
        }
        clear_cc = (settings.retract_mode == R_PLANE) ? r : old_cc;

        save_mode = GET_EXTERNAL_MOTION_CONTROL_MODE();
        if (save_mode != CANON_EXACT_PATH)
            SET_MOTION_CONTROL_MODE(CANON_EXACT_PATH);

        switch(motion)
        {
            case G_81:
                CYCLE_MACRO(convert_cycle_g81(CANON_PLANE_XY, aa, bb, clear_cc, cc))
                    break;
            case G_82:
                error_if(((settings.motion_mode != G_82) and (block.p_number == -1.0)), NCE_DWELL_TIME_P_WORD_MISSING_WITH_G82);
                block.p_number =
                    block.p_number == -1.0 ? settings.cycle.p : block.p_number;
                CYCLE_MACRO(convert_cycle_g82 (CANON_PLANE_XY, aa, bb, clear_cc, cc,
                    block.p_number))
                    settings.cycle.p = block.p_number;
                break;
            case G_83:
                error_if(((settings.motion_mode != G_83) and (block.q_number == -1.0)), NCE_Q_WORD_MISSING_WITH_G83);
                block.q_number =
                    block.q_number == -1.0 ? settings.cycle.q : block.q_number;
                CYCLE_MACRO(convert_cycle_g83 (CANON_PLANE_XY, aa, bb, r, clear_cc, cc,
                    block.q_number))
                    settings.cycle.q = block.q_number;
                break;
            case G_84:
                CYCLE_MACRO(convert_cycle_g84 (CANON_PLANE_XY, aa, bb, clear_cc, cc,
                    settings.spindle_turning, settings.speed_feed_mode))
                    break;
            case G_85:
                CYCLE_MACRO(convert_cycle_g85 (CANON_PLANE_XY, aa, bb, clear_cc, cc))
                    break;
            case G_86:
                error_if(((settings.motion_mode != G_86) and (block.p_number == -1.0)), NCE_DWELL_TIME_P_WORD_MISSING_WITH_G86);
                block.p_number =
                    block.p_number == -1.0 ? settings.cycle.p : block.p_number;
                CYCLE_MACRO(convert_cycle_g86 (CANON_PLANE_XY, aa, bb, clear_cc, cc,
                    block.p_number, settings.spindle_turning))
                    settings.cycle.p = block.p_number;
                break;
            case G_87:
                if (settings.motion_mode != G_87)
                {
                    error_if((block.i_flag == OFF), NCE_I_WORD_MISSING_WITH_G87);
                    error_if((block.j_flag == OFF), NCE_J_WORD_MISSING_WITH_G87);
                    error_if((block.k_flag == OFF), NCE_K_WORD_MISSING_WITH_G87);
                }
                i = block.i_flag == ON ? block.i_number : settings.cycle.i;
                j = block.j_flag == ON ? block.j_number : settings.cycle.j;
                k = block.k_flag == ON ? block.k_number : settings.cycle.k;
                settings.cycle.i = i;
                settings.cycle.j = j;
                settings.cycle.k = k;
                if (settings.distance_mode == MODE_INCREMENTAL)
                {
                    k = (cc + k);            /* k always absolute in function call below */
                }
                CYCLE_MACRO(convert_cycle_g87 (CANON_PLANE_XY, aa, (aa + i), bb,
                    (bb + j), r, clear_cc, k, cc, settings.spindle_turning))
                    break;
            case G_88:
                error_if(((settings.motion_mode != G_88) and (block.p_number == -1.0)), NCE_DWELL_TIME_P_WORD_MISSING_WITH_G88);
                block.p_number =
                    block.p_number == -1.0 ? settings.cycle.p : block.p_number;
                CYCLE_MACRO(convert_cycle_g88 (CANON_PLANE_XY, aa, bb, cc,
                    block.p_number, settings.spindle_turning))
                    settings.cycle.p = block.p_number;
                break;
            case G_89:
                error_if(((settings.motion_mode != G_89) and (block.p_number == -1.0)), NCE_DWELL_TIME_P_WORD_MISSING_WITH_G89);
                block.p_number =
                    block.p_number == -1.0 ? settings.cycle.p : block.p_number;
                CYCLE_MACRO(convert_cycle_g89 (CANON_PLANE_XY, aa, bb, clear_cc, cc,
                    block.p_number))
                    settings.cycle.p = block.p_number;
                break;
            default:
                throw error(NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        }
        settings.current.x = aa;            /* CYCLE_MACRO updates aa and bb */
        settings.current.y = bb;
        settings.current.z = clear_cc;
        settings.cycle.cc = block.z_number;

        if (save_mode != CANON_EXACT_PATH)
            SET_MOTION_CONTROL_MODE(save_mode);
    }

   /****************************************************************************/

   /* convert_cycle_yz

   Returned Value: int
   If any of the specific functions called returns an error code,
   this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The x-value is not given the first time this code is called after
   some other motion mode has been in effect:
   NCE_X_VALUE_UNSPECIFIED_IN_YZ_PLANE_CANNED_CYCLE
   2. The r clearance plane is below the bottom_x:
   NCE_R_LESS_THAN_X_IN_CYCLE_IN_YZ_PLANE
   3. the distance mode is neither absolute or incremental:
   NCE_BUG_DISTANCE_MODE_NOT_G90_OR_G91
   4. G82, G86, G88, or G89 is called when it is not already in effect,
   and no p number is in the block:
   NCE_DWELL_TIME_P_WORD_MISSING_WITH_G82
   NCE_DWELL_TIME_P_WORD_MISSING_WITH_G86
   NCE_DWELL_TIME_P_WORD_MISSING_WITH_G88
   NCE_DWELL_TIME_P_WORD_MISSING_WITH_G89
   5. G83 is called when it is not already in effect,
   and no q number is in the block: NCE_Q_WORD_MISSING_WITH_G83
   6. G87 is called when it is not already in effect,
   and any of the i number, j number, or k number is missing:
   NCE_I_WORD_MISSING_WITH_G87
   NCE_J_WORD_MISSING_WITH_G87
   NCE_K_WORD_MISSING_WITH_G87
   7. the G code is not between G_81 and G_89.
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED

   Side effects:
   A number of moves are made to execute a canned cycle.

   Called by: convert_cycle

   See the documentation of convert_cycle_xy. This function is entirely
   similar. In this function aa represents y, bb represents z, and cc
   represents x.

   The CYCLE_MACRO is defined just before the convert_cycle_xy function.

   Tool length offsets work only when the tool axis is parallel to the
   Z-axis, so if this function is used, tool length offsets should be
   turned off, and the NC code written to take tool length into account.

   */

    void rs274ngc::convert_cycle_yz(                  /* ARGUMENTS                                 */
    int motion,                                   /* a g-code between G_81 and G_89, a canned cycle */
    block_t& block,                          /* pointer to a block of RS274/NGC instructions   */
    setup_t& settings)                       /* pointer to machine settings                    */
    {
        double aa;
        double aa_increment;
        double bb;
        double bb_increment;
        double cc;
        double clear_cc;
        double i;
        double j;
        double k;
        double old_cc;
        CANON_PLANE plane;
        double r;
        int repeat;
        CANON_MOTION_MODE save_mode;

        plane = CANON_PLANE_YZ;
        if (settings.motion_mode != motion)
        {
            error_if((block.x_flag == OFF), NCE_X_VALUE_UNSPECIFIED_IN_YZ_PLANE_CANNED_CYCLE);
        }
        block.x_number =
            block.x_flag == ON ? block.x_number : settings.cycle.cc;
        old_cc = settings.current.x;

        if (settings.distance_mode == MODE_ABSOLUTE)
        {
            aa_increment = 0.0;
            bb_increment = 0.0;
            r = block.r_number;
            cc = block.x_number;
            aa = block.y_flag == ON ? block.y_number : settings.current.y;
            bb = block.z_flag == ON ? block.z_number : settings.current.z;
        }
        else if (settings.distance_mode == MODE_INCREMENTAL)
        {
            aa_increment = block.y_number;
            bb_increment = block.z_number;
            r = (block.r_number + old_cc);
            cc = (r + block.x_number);      /* [NCMS, page 98] */
            aa = settings.current.y;
            bb = settings.current.z;
        }
        else
            throw error(NCE_BUG_DISTANCE_MODE_NOT_G90_OR_G91);
        error_if((r < cc), NCE_R_LESS_THAN_X_IN_CYCLE_IN_YZ_PLANE);

        if (old_cc < r)
        {
            STRAIGHT_TRAVERSE(r, settings.current.y, settings.current.z
                ,      settings.current.a
                ,  settings.current.b
                ,  settings.current.c
                );
            old_cc = r;
        }
        clear_cc = (settings.retract_mode == R_PLANE) ? r : old_cc;

        save_mode = GET_EXTERNAL_MOTION_CONTROL_MODE();
        if (save_mode != CANON_EXACT_PATH)
            SET_MOTION_CONTROL_MODE(CANON_EXACT_PATH);

        switch(motion)
        {
            case G_81:
                CYCLE_MACRO(convert_cycle_g81(CANON_PLANE_YZ, aa, bb, clear_cc, cc))
                    break;
            case G_82:
                error_if(((settings.motion_mode != G_82) and (block.p_number == -1.0)), NCE_DWELL_TIME_P_WORD_MISSING_WITH_G82);
                block.p_number =
                    block.p_number == -1.0 ? settings.cycle.p : block.p_number;
                CYCLE_MACRO(convert_cycle_g82 (CANON_PLANE_YZ, aa, bb, clear_cc, cc,
                    block.p_number))
                    settings.cycle.p = block.p_number;
                break;
            case G_83:
                error_if(((settings.motion_mode != G_83) and (block.q_number == -1.0)), NCE_Q_WORD_MISSING_WITH_G83);
                block.q_number =
                    block.q_number == -1.0 ? settings.cycle.q : block.q_number;
                CYCLE_MACRO(convert_cycle_g83 (CANON_PLANE_YZ, aa, bb, r, clear_cc, cc,
                    block.q_number))
                    settings.cycle.q = block.q_number;
                break;
            case G_84:
                CYCLE_MACRO(convert_cycle_g84 (CANON_PLANE_YZ, aa, bb, clear_cc, cc,
                    settings.spindle_turning, settings.speed_feed_mode))
                    break;
            case G_85:
                CYCLE_MACRO(convert_cycle_g85 (CANON_PLANE_YZ, aa, bb, clear_cc, cc))
                    break;
            case G_86:
                error_if(((settings.motion_mode != G_86) and (block.p_number == -1.0)), NCE_DWELL_TIME_P_WORD_MISSING_WITH_G86);
                block.p_number =
                    block.p_number == -1.0 ? settings.cycle.p : block.p_number;
                CYCLE_MACRO(convert_cycle_g86 (CANON_PLANE_YZ, aa, bb, clear_cc, cc,
                    block.p_number, settings.spindle_turning))
                    settings.cycle.p = block.p_number;
                break;
            case G_87:
                if (settings.motion_mode != G_87)
                {
                    error_if((block.i_flag == OFF), NCE_I_WORD_MISSING_WITH_G87);
                    error_if((block.j_flag == OFF), NCE_J_WORD_MISSING_WITH_G87);
                    error_if((block.k_flag == OFF), NCE_K_WORD_MISSING_WITH_G87);
                }
                i = block.i_flag == ON ? block.i_number : settings.cycle.i;
                j = block.j_flag == ON ? block.j_number : settings.cycle.j;
                k = block.k_flag == ON ? block.k_number : settings.cycle.k;
                settings.cycle.i = i;
                settings.cycle.j = j;
                settings.cycle.k = k;
                if (settings.distance_mode == MODE_INCREMENTAL)
                {
                    i = (cc + i);            /* i always absolute in function call below */
                }
                CYCLE_MACRO(convert_cycle_g87 (CANON_PLANE_YZ, aa, (aa + j), bb,
                    (bb + k), r, clear_cc, i, cc, settings.spindle_turning))
                    break;
            case G_88:
                error_if(((settings.motion_mode != G_88) and (block.p_number == -1.0)), NCE_DWELL_TIME_P_WORD_MISSING_WITH_G88);
                block.p_number =
                    block.p_number == -1.0 ? settings.cycle.p : block.p_number;
                CYCLE_MACRO(convert_cycle_g88 (CANON_PLANE_YZ, aa, bb, cc,
                    block.p_number, settings.spindle_turning))
                    settings.cycle.p = block.p_number;
                break;
            case G_89:
                error_if(((settings.motion_mode != G_89) and (block.p_number == -1.0)), NCE_DWELL_TIME_P_WORD_MISSING_WITH_G89);
                block.p_number =
                    block.p_number == -1.0 ? settings.cycle.p : block.p_number;
                CYCLE_MACRO(convert_cycle_g89 (CANON_PLANE_YZ, aa, bb, clear_cc, cc,
                    block.p_number))
                    settings.cycle.p = block.p_number;
                break;
            default:
                throw error(NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        }
        settings.current.y = aa;            /* CYCLE_MACRO updates aa and bb */
        settings.current.z = bb;
        settings.current.x = clear_cc;
        settings.cycle.cc = block.x_number;

        if (save_mode != CANON_EXACT_PATH)
            SET_MOTION_CONTROL_MODE(save_mode);
    }

   /****************************************************************************/

   /* convert_cycle_zx

   Returned Value: int
   If any of the specific functions called returns an error code,
   this returns that code.
   If any of the following errors occur, this returns the ERROR code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The y-value is not given the first time this code is called after
   some other motion mode has been in effect:
   NCE_Y_VALUE_UNSPECIFIED_IN_XZ_PLANE_CANNED_CYCLE
   2. The r clearance plane is below the bottom_y:
   NCE_R_LESS_THAN_Y_IN_CYCLE_IN_XZ_PLANE
   3. the distance mode is neither absolute or incremental:
   NCE_BUG_DISTANCE_MODE_NOT_G90_OR_G91
   4. G82, G86, G88, or G89 is called when it is not already in effect,
   and no p number is in the block:
   NCE_DWELL_TIME_P_WORD_MISSING_WITH_G82
   NCE_DWELL_TIME_P_WORD_MISSING_WITH_G86
   NCE_DWELL_TIME_P_WORD_MISSING_WITH_G88
   NCE_DWELL_TIME_P_WORD_MISSING_WITH_G89
   5. G83 is called when it is not already in effect,
   and no q number is in the block: NCE_Q_WORD_MISSING_WITH_G83
   6. G87 is called when it is not already in effect,
   and any of the i number, j number, or k number is missing:
   NCE_I_WORD_MISSING_WITH_G87
   NCE_J_WORD_MISSING_WITH_G87
   NCE_K_WORD_MISSING_WITH_G87
   7. the G code is not between G_81 and G_89.
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED

   Side effects:
   A number of moves are made to execute a canned cycle.

   Called by: convert_cycle

   See the documentation of convert_cycle_xy. This function is entirely
   similar. In this function aa represents z, bb represents x, and cc
   represents y.

   The CYCLE_MACRO is defined just before the convert_cycle_xy function.

   Tool length offsets work only when the tool axis is parallel to the
   Z-axis, so if this function is used, tool length offsets should be
   turned off, and the NC code written to take tool length into account.

   It is a little distracting that this function uses zx in some places
   and xz in others; uniform use of zx would be nice, since that is the
   order for a right-handed coordinate system. Also with that usage,
   permutation of the symbols x, y, and z would allow for automatically
   converting the convert_cycle_xy function (or convert_cycle_yz) into
   the convert_cycle_xz function. However, the canonical interface uses
   CANON_PLANE_XZ.

   */

    void rs274ngc::convert_cycle_zx(                  /* ARGUMENTS                                 */
    int motion,                                   /* a g-code between G_81 and G_89, a canned cycle */
    block_t& block,                          /* pointer to a block of RS274 instructions       */
    setup_t& settings)                       /* pointer to machine settings                    */
    {
        double aa;
        double aa_increment;
        double bb;
        double bb_increment;
        double cc;
        double clear_cc;
        double i;
        double j;
        double k;
        double old_cc;
        CANON_PLANE plane;
        double r;
        int repeat;
        CANON_MOTION_MODE save_mode;

        plane = CANON_PLANE_XZ;
        if (settings.motion_mode != motion)
        {
            error_if((block.y_flag == OFF), NCE_Y_VALUE_UNSPECIFIED_IN_XZ_PLANE_CANNED_CYCLE);
        }
        block.y_number =
            block.y_flag == ON ? block.y_number : settings.cycle.cc;
        old_cc = settings.current.y;

        if (settings.distance_mode == MODE_ABSOLUTE)
        {
            aa_increment = 0.0;
            bb_increment = 0.0;
            r = block.r_number;
            cc = block.y_number;
            aa = block.z_flag == ON ? block.z_number : settings.current.z;
            bb = block.x_flag == ON ? block.x_number : settings.current.x;
        }
        else if (settings.distance_mode == MODE_INCREMENTAL)
        {
            aa_increment = block.z_number;
            bb_increment = block.x_number;
            r = (block.r_number + old_cc);
            cc = (r + block.y_number);      /* [NCMS, page 98] */
            aa = settings.current.z;
            bb = settings.current.x;
        }
        else
            throw error(NCE_BUG_DISTANCE_MODE_NOT_G90_OR_G91);
        error_if((r < cc), NCE_R_LESS_THAN_Y_IN_CYCLE_IN_XZ_PLANE);

        if (old_cc < r)
        {
            STRAIGHT_TRAVERSE(settings.current.x, r, settings.current.z
                ,           settings.current.a
                ,  settings.current.b
                ,  settings.current.c
                );
            old_cc = r;
        }
        clear_cc = (settings.retract_mode == R_PLANE) ? r : old_cc;

        save_mode = GET_EXTERNAL_MOTION_CONTROL_MODE();
        if (save_mode != CANON_EXACT_PATH)
            SET_MOTION_CONTROL_MODE(CANON_EXACT_PATH);

        switch(motion)
        {
            case G_81:
                CYCLE_MACRO(convert_cycle_g81(CANON_PLANE_XZ, aa, bb, clear_cc, cc))
                    break;
            case G_82:
                error_if(((settings.motion_mode != G_82) and (block.p_number == -1.0)), NCE_DWELL_TIME_P_WORD_MISSING_WITH_G82);
                block.p_number =
                    block.p_number == -1.0 ? settings.cycle.p : block.p_number;
                CYCLE_MACRO(convert_cycle_g82 (CANON_PLANE_XZ, aa, bb, clear_cc, cc,
                    block.p_number))
                    settings.cycle.p = block.p_number;
                break;
            case G_83:
                error_if(((settings.motion_mode != G_83) and (block.q_number == -1.0)), NCE_Q_WORD_MISSING_WITH_G83);
                block.q_number =
                    block.q_number == -1.0 ? settings.cycle.q : block.q_number;
                CYCLE_MACRO(convert_cycle_g83 (CANON_PLANE_XZ, aa, bb, r, clear_cc, cc,
                    block.q_number))
                    settings.cycle.q = block.q_number;
                break;
            case G_84:
                CYCLE_MACRO(convert_cycle_g84 (CANON_PLANE_XZ, aa, bb, clear_cc, cc,
                    settings.spindle_turning, settings.speed_feed_mode))
                    break;
            case G_85:
                CYCLE_MACRO(convert_cycle_g85 (CANON_PLANE_XZ, aa, bb, clear_cc, cc))
                    break;
            case G_86:
                error_if(((settings.motion_mode != G_86) and (block.p_number == -1.0)), NCE_DWELL_TIME_P_WORD_MISSING_WITH_G86);
                block.p_number =
                    block.p_number == -1.0 ? settings.cycle.p : block.p_number;
                CYCLE_MACRO(convert_cycle_g86 (CANON_PLANE_XZ, aa, bb, clear_cc, cc,
                    block.p_number, settings.spindle_turning))
                    settings.cycle.p = block.p_number;
                break;
            case G_87:
                if (settings.motion_mode != G_87)
                {
                    error_if((block.i_flag == OFF), NCE_I_WORD_MISSING_WITH_G87);
                    error_if((block.j_flag == OFF), NCE_J_WORD_MISSING_WITH_G87);
                    error_if((block.k_flag == OFF), NCE_K_WORD_MISSING_WITH_G87);
                }
                i = block.i_flag == ON ? block.i_number : settings.cycle.i;
                j = block.j_flag == ON ? block.j_number : settings.cycle.j;
                k = block.k_flag == ON ? block.k_number : settings.cycle.k;
                settings.cycle.i = i;
                settings.cycle.j = j;
                settings.cycle.k = k;
                if (settings.distance_mode == MODE_INCREMENTAL)
                {
                    j = (cc + j);            /* j always absolute in function call below */
                }
                CYCLE_MACRO(convert_cycle_g87 (CANON_PLANE_XZ, aa, (aa + k), bb,
                    (bb + i), r, clear_cc, j, cc, settings.spindle_turning))
                    break;
            case G_88:
                error_if(((settings.motion_mode != G_88) and (block.p_number == -1.0)), NCE_DWELL_TIME_P_WORD_MISSING_WITH_G88);
                block.p_number =
                    block.p_number == -1.0 ? settings.cycle.p : block.p_number;
                CYCLE_MACRO(convert_cycle_g88 (CANON_PLANE_XZ, aa, bb, cc,
                    block.p_number, settings.spindle_turning))
                    settings.cycle.p = block.p_number;
                break;
            case G_89:
                error_if(((settings.motion_mode != G_89) and (block.p_number == -1.0)), NCE_DWELL_TIME_P_WORD_MISSING_WITH_G89);
                block.p_number =
                    block.p_number == -1.0 ? settings.cycle.p : block.p_number;
                CYCLE_MACRO(convert_cycle_g89 (CANON_PLANE_XZ, aa, bb, clear_cc, cc,
                    block.p_number))
                    settings.cycle.p = block.p_number;
                break;
            default:
                throw error(NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        }
        settings.current.z = aa;            /* CYCLE_MACRO updates aa and bb */
        settings.current.x = bb;
        settings.current.y = clear_cc;
        settings.cycle.cc = block.y_number;

        if (save_mode != CANON_EXACT_PATH)
            SET_MOTION_CONTROL_MODE(save_mode);
    }

   /****************************************************************************/

   /* convert_distance_mode

   Returned Value: int
   If any of the following errors occur, this returns the error shown.
   Otherwise, it returns RS274NGC_OK.
   1. g_code isn't G_90 or G_91: NCE_BUG_CODE_NOT_G90_OR_G91

   Side effects:
   The interpreter switches the machine settings to indicate the current
   distance mode (absolute or incremental).

   The canonical machine to which commands are being sent does not have
   an incremental mode, so no command setting the distance mode is
   generated in this function. A comment function call explaining the
   change of mode is made (conditionally), however, if there is a change.

   Called by: convert_g.

   */

    void rs274ngc::convert_distance_mode(             /* ARGUMENTS                             */
    int g_code,                                   /* g_code being executed (must be G_90 or G_91) */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        if (g_code == G_90)
        {
            if (settings.distance_mode != MODE_ABSOLUTE)
            {
#ifdef DEBUG_EMC
                COMMENT("interpreter: distance mode changed to absolute");
#endif
                settings.distance_mode = MODE_ABSOLUTE;
            }
        }
        else if (g_code == G_91)
        {
            if (settings.distance_mode != MODE_INCREMENTAL)
            {
#ifdef DEBUG_EMC
                COMMENT("interpreter: distance mode changed to incremental");
#endif
                settings.distance_mode = MODE_INCREMENTAL;
            }
        }
        else
            throw error(NCE_BUG_CODE_NOT_G90_OR_G91);
    }

   /****************************************************************************/

   /* convert_dwell

   Returned Value: int (RS274NGC_OK)

   Side effects:
   A dwell command is executed.

   Called by: convert_g.

   */

    void rs274ngc::convert_dwell(                     /* ARGUMENTS                 */
    double time)                                  /* time in seconds to dwell  */
    {
        DWELL(time);
    }

   /****************************************************************************/

   /* convert_feed_mode

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1.  g_code isn't G_93 or G_94: NCE_BUG_CODE_NOT_G93_OR_G94

   Side effects:
   The interpreter switches the machine settings to indicate the current
   feed mode (UNITS_PER_MINUTE or INVERSE_TIME).

   The canonical machine to which commands are being sent does not have
   a feed mode, so no command setting the distance mode is generated in
   this function. A comment function call is made (conditionally)
   explaining the change in mode, however.

   Called by: execute_block.

   */

    void rs274ngc::convert_feed_mode(                 /* ARGUMENTS                                 */
    int g_code,                                   /* g_code being executed (must be G_93 or G_94) */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        if (g_code == G_93)
        {
#ifdef DEBUG_EMC
            COMMENT("interpreter: feed mode set to inverse time");
#endif
            settings.feed_mode = INVERSE_TIME;
        }
        else if (g_code == G_94)
        {
#ifdef DEBUG_EMC
            COMMENT("interpreter: feed mode set to units per minute");
#endif
            settings.feed_mode = UNITS_PER_MINUTE;
        }
        else
            throw error(NCE_BUG_CODE_NOT_G93_OR_G94);
    }

   /****************************************************************************/

   /* convert_feed_rate

   Returned Value: int (RS274NGC_OK)

   Side effects:
   The machine feed_rate is set to the value of f_number in the
   block by function call.
   The machine model feed_rate is set to that value.

   Called by: execute_block

   This is called only if the feed mode is UNITS_PER_MINUTE.

   */

    void rs274ngc::convert_feed_rate(                 /* ARGUMENTS                                */
    block_t& block,                          /* pointer to a block of RS274 instructions */
    setup_t& settings)                       /* pointer to machine settings              */
    {
        SET_FEED_RATE(block.f_number);
        settings.feed_rate = block.f_number;
    }

   /****************************************************************************/

   /* convert_g

   Returned Value: int
   If one of the following functions is called and returns an error code,
   this returns that code.
   convert_control_mode
   convert_coordinate_system
   convert_cutter_compensation
   convert_distance_mode
   convert_dwell
   convert_length_units
   convert_modal_0
   convert_motion
   convert_retract_mode
   convert_set_plane
   convert_tool_length_offset
   Otherwise, it returns RS274NGC_OK.

   Side effects:
   Any g_codes in the block (excluding g93 and 94) and any implicit
   motion g_code are executed.

   Called by: execute_block.

   This takes a pointer to a block of RS274/NGC instructions (already
   read in) and creates the appropriate output commands corresponding to
   any "g" codes in the block.

   Codes g93 and g94, which set the feed mode, are executed earlier by
   execute_block before reading the feed rate.

   G codes are are executed in the following order.
   1.  mode 0, G4 only - dwell. Left here from earlier versions.
   2.  mode 2, one of (G17, G18, G19) - plane selection.
   3.  mode 6, one of (G20, G21) - length units.
   4.  mode 7, one of (G40, G41, G42) - cutter radius compensation.
   5.  mode 8, one of (G43, G49) - tool length offset
   6.  mode 12, one of (G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3)
   - coordinate system selection.
   7.  mode 13, one of (G61, G61.1, G64) - control mode
   8.  mode 3, one of (G90, G91) - distance mode.
   9.  mode 10, one of (G98, G99) - retract mode.
   10. mode 0, one of (G10, G28, G30, G92, G92.1, G92.2, G92.3) -
   setting coordinate system locations, return to reference point 1,
   return to reference point 2, setting or cancelling axis offsets.
   11. mode 1, one of (G0, G1, G2, G3, G38.2, G80, G81 to G89) - motion or cancel.
   G53 from mode 0 is also handled here, if present.

   Some mode 0 and most mode 1 G codes must be executed after the length units
   are set, since they use coordinate values. Mode 1 codes also must wait
   until most of the other modes are set.

   */

    void rs274ngc::convert_g(                         /* ARGUMENTS                                    */
    block_t& block,                          /* pointer to a block of RS274/NGC instructions */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        if (block.g_modes[0] == G_4)
        {
            convert_dwell(block.p_number);
        }
        if (block.g_modes[2] != -1)
        {
            convert_set_plane(block.g_modes[2], settings);
        }
        if (block.g_modes[6] != -1)
        {
            convert_length_units(block.g_modes[6], settings);
        }
        if (block.g_modes[7] != -1)
        {
            convert_cutter_compensation(block.g_modes[7], block, settings);
        }
        if (block.g_modes[8] != -1)
        {
            convert_tool_length_offset(block.g_modes[8], block, settings);
        }
        if (block.g_modes[12] != -1)
        {
            convert_coordinate_system(block.g_modes[12], settings);
        }
        if (block.g_modes[13] != -1)
        {
            convert_control_mode(block.g_modes[13], settings);
        }
        if (block.g_modes[3] != -1)
        {
            convert_distance_mode(block.g_modes[3], settings);
        }
        if (block.g_modes[10] != -1)
        {
            convert_retract_mode(block.g_modes[10], settings);
        }
        if (block.g_modes[0] != -1)
        {
            convert_modal_0(block.g_modes[0], block, settings);
        }
        if (block.motion_to_be != -1)
        {
            convert_motion(block.motion_to_be, block, settings);
        }
    }

   /****************************************************************************/

   /* convert_home

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. cutter radius compensation is on:
   NCE_CANNOT_USE_G28_OR_G30_WITH_CUTTER_RADIUS_COMP
   2. The code is not G28 or G30: NCE_BUG_CODE_NOT_G28_OR_G30

   Side effects:
   This executes a straight traverse to the programmed point, using
   the current coordinate system, tool length offset, and motion mode
   to interpret the coordinate values. Then it executes a straight
   traverse to the location of reference point 1 (if G28) or reference
   point 2 (if G30). It also updates the setting of the position of the
   tool point to the end point of the move.

   Called by: convert_modal_0.

   During the motion from the intermediate point to the home point, this
   function currently makes the A and C axes turn counterclockwise if a
   turn is needed.  This is not necessarily the most efficient way to do
   it. A check might be made of which direction to turn to have the least
   turn to get to the reference position, and the axis would turn that
   way.

   */

    void rs274ngc::convert_home(                      /* ARGUMENTS                                */
    int move,                                     /* G code, must be G_28 or G_30             */
    block_t& block,                          /* pointer to a block of RS274 instructions */
    setup_t& settings)                       /* pointer to machine settings              */
    {
        double end_x;
        double end_y;
        double end_z;
        double AA_end;                            /*AA*/
        double AA_end2;                           /*AA*/
        double BB_end;                            /*BB*/
        double BB_end2;                           /*BB*/
        double CC_end;                            /*CC*/
        double CC_end2;                           /*CC*/
        double * parameters;

        parameters = settings.parameters;
        find_ends(block, settings, &end_x, &end_y,
            &end_z
            , &AA_end
            , &BB_end
            , &CC_end
            );

        error_if((settings.cutter_comp_side != OFF), NCE_CANNOT_USE_G28_OR_G30_WITH_CUTTER_RADIUS_COMP);
        STRAIGHT_TRAVERSE(end_x, end_y, end_z
            ,           AA_end
            ,  BB_end
            ,  CC_end
            );
        if (move == G_28)
        {
            find_relative
                (parameters[5161], parameters[5162], parameters[5163],
                parameters[5164],                 /*AA*/
                parameters[5165],                 /*BB*/
                parameters[5166],                 /*CC*/
                &end_x, &end_y, &end_z
                , &AA_end2
                , &BB_end2
                , &CC_end2
                , settings);
        }
        else if (move == G_30)
        {
            find_relative
                (parameters[5181], parameters[5182], parameters[5183],
                parameters[5184],                 /*AA*/
                parameters[5185],                 /*BB*/
                parameters[5186],                 /*CC*/
                &end_x, &end_y, &end_z
                , &AA_end2
                , &BB_end2
                , &CC_end2
                , settings);
        }
        else
            throw error(NCE_BUG_CODE_NOT_G28_OR_G30);
        STRAIGHT_TRAVERSE(end_x, end_y, end_z
            ,           AA_end
            ,  BB_end
            ,  CC_end
            );
        settings.current.x = end_x;
        settings.current.y = end_y;
        settings.current.z = end_z;
        settings.current.a = AA_end2;      /*AA*/
        settings.current.b = BB_end2;      /*BB*/
        settings.current.c = CC_end2;      /*CC*/
    }

   /****************************************************************************/

   /* convert_length_units

   Returned Value: int
   If any of the following errors occur, this returns the error shown.
   Otherwise, it returns RS274NGC_OK.
   1. The g_code argument isnt G_20 or G_21:
   NCE_BUG_CODE_NOT_G20_OR_G21
   2. Cutter radius compensation is on:
   NCE_CANNOT_CHANGE_UNITS_WITH_CUTTER_RADIUS_COMP

   Side effects:
   A command setting the length units is executed. The machine
   settings are reset regarding length units and current position.

   Called by: convert_g.

   We are not changing tool length offset values or tool diameter values.
   Those values must be given in the table in the correct units. Thus it
   will generally not be feasible to switch units in the middle of a
   program.

   We are not changing the parameters that represent the positions
   of the nine work coordinate systems.

   We are also not changing feed rate values when length units are
   changed, so the actual behavior may change.

   Several other distance items in the settings (such as the various
   parameters for cycles) are also not reset.

   We are changing origin offset and axis offset values, which are
   critical. If this were not done, when length units are set and the new
   length units are not the same as the default length units
   (millimeters), and any XYZ origin or axis offset is not zero, then any
   subsequent change in XYZ origin or axis offset values will be
   incorrect.  Also, g53 (motion in absolute coordinates) will not work
   correctly.

   */

    void rs274ngc::convert_length_units(              /* ARGUMENTS                             */
    int g_code,                                   /* g_code being executed (must be G_20 or G_21) */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        error_if((settings.cutter_comp_side != OFF), NCE_CANNOT_CHANGE_UNITS_WITH_CUTTER_RADIUS_COMP);
        if (g_code == G_20)
        {
            USE_LENGTH_UNITS(CANON_UNITS_INCHES);
            if (settings.length_units != CANON_UNITS_INCHES)
            {
                settings.length_units = CANON_UNITS_INCHES;
                settings.current.x = (settings.current.x * INCH_PER_MM);
                settings.current.y = (settings.current.y * INCH_PER_MM);
                settings.current.z = (settings.current.z * INCH_PER_MM);
                settings.axis_offset.x =
                    (settings.axis_offset.x * INCH_PER_MM);
                settings.axis_offset.y =
                    (settings.axis_offset.y * INCH_PER_MM);
                settings.axis_offset.z =
                    (settings.axis_offset.z * INCH_PER_MM);
                settings.origin_offset.x =
                    (settings.origin_offset.x * INCH_PER_MM);
                settings.origin_offset.y =
                    (settings.origin_offset.y * INCH_PER_MM);
                settings.origin_offset.z =
                    (settings.origin_offset.z * INCH_PER_MM);
            }
        }
        else if (g_code == G_21)
        {
            USE_LENGTH_UNITS(CANON_UNITS_MM);
            if (settings.length_units != CANON_UNITS_MM)
            {
                settings.length_units = CANON_UNITS_MM;
                settings.current.x = (settings.current.x * MM_PER_INCH);
                settings.current.y = (settings.current.y * MM_PER_INCH);
                settings.current.z = (settings.current.z * MM_PER_INCH);
                settings.axis_offset.x =
                    (settings.axis_offset.x * MM_PER_INCH);
                settings.axis_offset.y =
                    (settings.axis_offset.y * MM_PER_INCH);
                settings.axis_offset.z =
                    (settings.axis_offset.z * MM_PER_INCH);
                settings.origin_offset.x =
                    (settings.origin_offset.x * MM_PER_INCH);
                settings.origin_offset.y =
                    (settings.origin_offset.y * MM_PER_INCH);
                settings.origin_offset.z =
                    (settings.origin_offset.z * MM_PER_INCH);
            }
        }
        else
            throw error(NCE_BUG_CODE_NOT_G20_OR_G21);
    }

   /****************************************************************************/

   /* convert_m

   Returned Value: int
   If convert_tool_change returns an error code, this returns that code.
   Otherwise, it returns RS274NGC_OK.

   Side effects:
   m_codes in the block are executed. For each m_code
   this consists of making a function call(s) to a canonical machining
   function(s) and setting the machine model.

   Called by: execute_block.

   This handles four separate types of activity in order:
   1. changing the tool (m6) - which also retracts and stops the spindle.
   2. Turning the spindle on or off (m3, m4, and m5)
   3. Turning coolant on and off (m7, m8, and m9)
   4. turning a-axis clamping on and off (m26, m27) - commented out.
   5. enabling or disabling feed and speed overrides (m49, m49).
   Within each group, only the first code encountered will be executed.

   This does nothing with m0, m1, m2, m30, or m60 (which are handled in
   convert_stop).

   */

    void rs274ngc::convert_m(                         /* ARGUMENTS                                    */
    block_t& block,                          /* pointer to a block of RS274/NGC instructions */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        if (block.m_modes[6] != -1)
        {
            convert_tool_change(settings);
        }

        if (block.m_modes[7] == 3)
        {
            START_SPINDLE_CLOCKWISE();
            settings.spindle_turning = CANON_CLOCKWISE;
        }
        else if (block.m_modes[7] == 4)
        {
            START_SPINDLE_COUNTERCLOCKWISE();
            settings.spindle_turning = CANON_COUNTERCLOCKWISE;
        }
        else if (block.m_modes[7] == 5)
        {
            STOP_SPINDLE_TURNING();
            settings.spindle_turning = CANON_STOPPED;
        }

        if (block.m_modes[8] == 7)
        {
            MIST_ON();
            settings.mist = ON;
        }
        else if (block.m_modes[8] == 8)
        {
            FLOOD_ON();
            settings.flood = ON;
        }
        else if (block.m_modes[8] == 9)
        {
            MIST_OFF();
            settings.mist = OFF;
            FLOOD_OFF();
            settings.flood = OFF;
        }

   /* No axis clamps in this version
     if (block.m_modes[2] == 26)
       {
   #ifdef DEBUG_EMC
   COMMENT("interpreter: automatic A-axis clamping turned on");
   #endif
   settings.a_axis_clamping = ON;
   }
   else if (block.m_modes[2] == 27)
   {
   #ifdef DEBUG_EMC
   COMMENT("interpreter: automatic A-axis clamping turned off");
   #endif
   settings.a_axis_clamping = OFF;
   }
   */

        if (block.m_modes[9] == 48)
        {
            ENABLE_FEED_OVERRIDE();
            ENABLE_SPEED_OVERRIDE();
            settings.feed_override = ON;
            settings.speed_override = ON;
        }
        else if (block.m_modes[9] == 49)
        {
            DISABLE_FEED_OVERRIDE();
            DISABLE_SPEED_OVERRIDE();
            settings.feed_override = OFF;
            settings.speed_override = OFF;
        }
    }

   /****************************************************************************/

   /* convert_modal_0

   Returned Value: int
   If one of the following functions is called and returns an error code,
   this returns that code.
   convert_axis_offsets
   convert_home
   convert_setup
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. code is not G_4, G_10, G_28, G_30, G_53, G92, G_92_1, G_92_2, or G_92_3:
   NCE_BUG_CODE_NOT_G4_G10_G28_G30_G53_OR_G92_SERIES

   Side effects: See below

   Called by: convert_g

   If the g_code is g10, g28, g30, g92, g92.1, g92.2, or g92.3 (all are in
   modal group 0), it is executed. The other two in modal group 0 (G4 and
   G53) are executed elsewhere.

   */

    void rs274ngc::convert_modal_0(                   /* ARGUMENTS                                    */
    int code,                                     /* G code, must be from group 0                 */
    block_t& block,                          /* pointer to a block of RS274/NGC instructions */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        if (code == G_10)
        {
            convert_setup(block, settings);
        }
        else if ((code == G_28) or (code == G_30))
        {
            convert_home(code, block, settings);
        }
        else if ((code == G_92)   or (code == G_92_1) or
            (code == G_92_2) or (code == G_92_3))
        {
            convert_axis_offsets(code, block, settings);
        }
        else if ((code == G_4) or (code == G_53));/* handled elsewhere */
        else
            throw error(NCE_BUG_CODE_NOT_G4_G10_G28_G30_G53_OR_G92_SERIES);
    }

   /****************************************************************************/

   /* convert_motion

   Returned Value: int
   If one of the following functions is called and returns an error code,
   this returns that code.
   convert_arc
   convert_cycle
   convert_probe
   convert_straight
   If any of the following errors occur, this returns the error shown.
   Otherwise, it returns RS274NGC_OK.
   1. The motion code is not 0,1,2,3,38.2,80,81,82,83,84,85,86,87, 88, or 89:
   NCE_BUG_UNKNOWN_MOTION_CODE

   Side effects:
   A g_code from the group causing motion (mode 1) is executed.

   Called by: convert_g.

   */

    void rs274ngc::convert_motion(                    /* ARGUMENTS                                 */
    int motion,                                   /* g_code for a line, arc, canned cycle      */
    block_t& block,                          /* pointer to a block of RS274 instructions  */
    setup_t& settings)                       /* pointer to machine settings               */
    {
        if ((motion == G_0) or (motion == G_1))
        {
            convert_straight(motion, block, settings);
        }
        else if ((motion == G_3) or (motion == G_2))
        {
            convert_arc(motion, block, settings);
        }
        else if (motion == G_38_2)
        {
            convert_probe(block, settings);
        }
        else if (motion == G_80)
        {
#ifdef DEBUG_EMC
            COMMENT("interpreter: motion mode set to none");
#endif
            settings.motion_mode = G_80;
        }
        else if ((motion > G_80) and (motion < G_90))
        {
            convert_cycle(motion, block, settings);
        }
        else
            throw error(NCE_BUG_UNKNOWN_MOTION_CODE);
    }

   /****************************************************************************/

   /* convert_probe

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. No value is given in the block for any of X, Y, or Z:
   NCE_X_Y_AND_Z_WORDS_ALL_MISSING_WITH_G38_2
   2. feed mode is inverse time: NCE_CANNOT_PROBE_IN_INVERSE_TIME_FEED_MODE
   3. cutter radius comp is on: NCE_CANNOT_PROBE_WITH_CUTTER_RADIUS_COMP_ON
   4. Feed rate is zero: NCE_CANNOT_PROBE_WITH_ZERO_FEED_RATE
   5. Rotary axis motion is programmed:
   NCE_CANNOT_MOVE_ROTARY_AXES_DURING_PROBING
   6. The starting point for the probe move is within 0.01 inch or 0.254
   millimeters of the point to be probed:
   NCE_START_POINT_TOO_CLOSE_TO_PROBE_POINT

   Side effects:
   This executes a straight_probe command.
   The probe_flag in the settings is set to ON.
   The motion mode in the settings is set to G_38_2.

   Called by: convert_motion.

   The approach to operating in incremental distance mode (g91) is to
   put the the absolute position values into the block before using the
   block to generate a move.

   After probing is performed, the location of the probe cannot be
   predicted. This differs from every other command, all of which have
   predictable results. The next call to the interpreter (with either
   rs274ngc_read or rs274ngc_execute) will result in updating the
   current position by calls to get_external_position_x, etc.

   */

    void rs274ngc::convert_probe(                     /* ARGUMENTS                                */
    block_t& block,                          /* pointer to a block of RS274 instructions */
    setup_t& settings)                       /* pointer to machine settings              */
    {
        double distance;
        double end_x;
        double end_y;
        double end_z;
        double AA_end;                            /*AA*/
        double BB_end;                            /*BB*/
        double CC_end;                            /*CC*/

        error_if((((block.x_flag == OFF) and (block.y_flag == OFF)) and
            (block.z_flag == OFF)), NCE_X_Y_AND_Z_WORDS_ALL_MISSING_WITH_G38_2);
        error_if((settings.feed_mode == INVERSE_TIME), NCE_CANNOT_PROBE_IN_INVERSE_TIME_FEED_MODE);
        error_if((settings.cutter_comp_side != OFF), NCE_CANNOT_PROBE_WITH_CUTTER_RADIUS_COMP_ON);
        error_if((settings.feed_rate == 0.0), NCE_CANNOT_PROBE_WITH_ZERO_FEED_RATE);
        find_ends(block, settings, &end_x, &end_y,
            &end_z
            , &AA_end
            , &BB_end
            , &CC_end
            );
        if (0
            or (AA_end != settings.current.a) /*AA*/
            or (BB_end != settings.current.b) /*BB*/
            or (CC_end != settings.current.c) /*CC*/
            )
            throw error(NCE_CANNOT_MOVE_ROTARY_AXES_DURING_PROBING);
        distance = sqrt(pow((settings.current.x - end_x), 2) +
            pow((settings.current.y - end_y), 2) +
            pow((settings.current.z - end_z), 2));
        error_if((distance < ((settings.length_units == CANON_UNITS_MM) ? 0.254 : 0.01)), NCE_START_POINT_TOO_CLOSE_TO_PROBE_POINT);
        TURN_PROBE_ON();
        STRAIGHT_PROBE(end_x, end_y, end_z
            ,        AA_end
            ,  BB_end
            ,  CC_end
            );
        TURN_PROBE_OFF();
        settings.motion_mode = G_38_2;
        settings.probe_flag = ON;
    }

   /****************************************************************************/

   /* convert_retract_mode

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. g_code isn't G_98 or G_99: NCE_BUG_CODE_NOT_G98_OR_G99

   Side effects:
   The interpreter switches the machine settings to indicate the current
   retract mode for canned cycles (OLD_Z or R_PLANE).

   Called by: convert_g.

   The canonical machine to which commands are being sent does not have a
   retract mode, so no command setting the retract mode is generated in
   this function.

   */

    void rs274ngc::convert_retract_mode(              /* ARGUMENTS                             */
    int g_code,                                   /* g_code being executed (must be G_98 or G_99) */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        if (g_code == G_98)
        {
#ifdef DEBUG_EMC
            COMMENT("interpreter: retract mode set to old_z");
#endif
            settings.retract_mode = OLD_Z;
        }
        else if (g_code == G_99)
        {
#ifdef DEBUG_EMC
            COMMENT("interpreter: retract mode set to r_plane");
#endif
            settings.retract_mode = R_PLANE;
        }
        else
            throw error(NCE_BUG_CODE_NOT_G98_OR_G99);
    }

   /****************************************************************************/

   /* convert_setup

   Returned Value: int (RS274NGC_OK)

   Side effects:
   SET_PROGRAM_ORIGIN is called, and the coordinate
   values for the program origin are reset.
   If the program origin is currently in use, the values of the
   the coordinates of the current point are updated.

   Called by: convert_modal_0.

   This is called only if g10 is called. g10 L2 may be used to alter the
   location of coordinate systems as described in [NCMS, pages 9 - 10] and
   [Fanuc, page 65]. [Fanuc] has only six coordinate systems, while
   [NCMS] has nine (the first six of which are the same as the six [Fanuc]
   has). All nine are implemented here.

   Being in incremental distance mode has no effect on the action of G10
   in this implementation. The manual is not explicit about what is
   intended.

   See documentation of convert_coordinate_system for more information.

   */

    void rs274ngc::convert_setup(                     /* ARGUMENTS                                    */
    block_t& block,                          /* pointer to a block of RS274/NGC instructions */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        double x;
        double y;
        double z;
        double a;                                 /*AA*/
        double b;                                 /*BB*/
        double c;                                 /*CC*/
        double * parameters;
        int p_int;

        parameters = settings.parameters;
        p_int = (int)(block.p_number + 0.0001);

        if (block.x_flag == ON)
        {
            x = block.x_number;
            parameters[5201 + (p_int * 20)] = x;
        }
        else
            x = parameters[5201 + (p_int * 20)];

        if (block.y_flag == ON)
        {
            y = block.y_number;
            parameters[5202 + (p_int * 20)] = y;
        }
        else
            y = parameters[5202 + (p_int * 20)];
        if (block.z_flag == ON)
        {
            z = block.z_number;
            parameters[5203 + (p_int * 20)] = z;
        }
        else
            z = parameters[5203 + (p_int * 20)];

        if (block.a_flag == ON)
        {
            a = block.a_number;
            parameters[5204 + (p_int * 20)] = a;
        }
        else
            a = parameters[5204 + (p_int * 20)];

        if (block.b_flag == ON)
        {
            b = block.b_number;
            parameters[5205 + (p_int * 20)] = b;
        }
        else
            b = parameters[5205 + (p_int * 20)];

        if (block.c_flag == ON)
        {
            c = block.c_number;
            parameters[5206 + (p_int * 20)] = c;
        }
        else
            c = parameters[5206 + (p_int * 20)];

   /* axis offsets could be included in the two sets of calculations for
      current_x, current_y, etc., but do not need to be because the results
      would be the same. They would be added in then subtracted out. */
        if (p_int == settings.origin_index)      /* system is currently used */
        {
            settings.current.x =
                (settings.current.x + settings.origin_offset.x);
            settings.current.y =
                (settings.current.y + settings.origin_offset.y);
            settings.current.z =
                (settings.current.z + settings.origin_offset.z);
            settings.current.a =           /*AA*/
                (settings.current.a + settings.origin_offset.a);
            settings.current.b =           /*BB*/
                (settings.current.b + settings.origin_offset.b);
            settings.current.c =           /*CC*/
                (settings.current.c + settings.origin_offset.c);

            settings.origin_offset.x = x;
            settings.origin_offset.y = y;
            settings.origin_offset.z = z;
            settings.origin_offset.a = a;  /*AA*/
            settings.origin_offset.b = b;  /*BB*/
            settings.origin_offset.c = c;  /*CC*/

            settings.current.x = (settings.current.x - x);
            settings.current.y = (settings.current.y - y);
            settings.current.z = (settings.current.z - z);
            settings.current.a = (settings.current.a - a);
            settings.current.b = (settings.current.b - b);
            settings.current.c = (settings.current.c - c);

            SET_ORIGIN_OFFSETS(x + settings.axis_offset.x,
                y + settings.axis_offset.y,
                z + settings.axis_offset.z
                ,            a + settings.axis_offset.a
                ,            b + settings.axis_offset.b
                ,            c + settings.axis_offset.c
                );
        }
#ifdef DEBUG_EMC
        else
            COMMENT("interpreter: setting coordinate system origin");
#endif
    }

   /****************************************************************************/

   /* convert_set_plane

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. G_18 or G_19 is called when cutter radius compensation is on:
   NCE_CANNOT_USE_XZ_PLANE_WITH_CUTTER_RADIUS_COMP
   NCE_CANNOT_USE_YZ_PLANE_WITH_CUTTER_RADIUS_COMP
   2. The g_code is not G_17, G_18, or G_19:
   NCE_BUG_CODE_NOT_G17_G18_OR_G19

   Side effects:
   A canonical command setting the current plane is executed.

   Called by: convert_g.

   */

    void rs274ngc::convert_set_plane(                 /* ARGUMENTS                    */
    int g_code,                                   /* must be G_17, G_18, or G_19  */
    setup_t& settings)                       /* pointer to machine settings  */
    {
        if (g_code == G_17)
        {
            SELECT_PLANE(CANON_PLANE_XY);
            settings.plane = CANON_PLANE_XY;
        }
        else if (g_code == G_18)
        {
            error_if((settings.cutter_comp_side != OFF), NCE_CANNOT_USE_XZ_PLANE_WITH_CUTTER_RADIUS_COMP);
            SELECT_PLANE(CANON_PLANE_XZ);
            settings.plane = CANON_PLANE_XZ;
        }
        else if (g_code == G_19)
        {
            error_if((settings.cutter_comp_side != OFF), NCE_CANNOT_USE_YZ_PLANE_WITH_CUTTER_RADIUS_COMP);
            SELECT_PLANE(CANON_PLANE_YZ);
            settings.plane = CANON_PLANE_YZ;
        }
        else
            throw error(NCE_BUG_CODE_NOT_G17_G18_OR_G19);
    }

   /****************************************************************************/

   /* convert_speed

   Returned Value: int (RS274NGC_OK)

   Side effects:
   The machine spindle speed is set to the value of s_number in the
   block by a call to SET_SPINDLE_SPEED.
   The machine model for spindle speed is set to that value.

   Called by: execute_block.

   */

    void rs274ngc::convert_speed(                     /* ARGUMENTS                                */
    block_t& block,                          /* pointer to a block of RS274 instructions */
    setup_t& settings)                       /* pointer to machine settings              */
    {
        SET_SPINDLE_SPEED(block.s_number);
        settings.speed = block.s_number;
    }

   /****************************************************************************/

   /* convert_stop

   Returned Value: int
   When an m2 or m30 (program_end) is encountered, this returns RS274NGC_EXIT.
   If the code is not m0, m1, m2, m30, or m60, this returns
   NCE_BUG_CODE_NOT_M0_M1_M2_M30_M60
   Otherwise, it returns RS274NGC_OK.

   Side effects:
   An m0, m1, m2, m30, or m60 in the block is executed.

   For m0, m1, and m60, this makes a function call to the PROGRAM_STOP
   canonical machining function (which stops program execution).
   In addition, m60 calls PALLET_SHUTTLE.

   For m2 and m30, this resets the machine and then calls PROGRAM_END.
   In addition, m30 calls PALLET_SHUTTLE.

   Called by: execute_block.

   This handles stopping or ending the program (m0, m1, m2, m30, m60)

   [NCMS] specifies how the following modes should be reset at m2 or
   m30. The descriptions are not collected in one place, so this list
   may be incomplete.

   G52 offsetting coordinate zero points [NCMS, page 10]
   G92 coordinate offset using tool position [NCMS, page 10]

   The following should have reset values, but no description of reset
   behavior could be found in [NCMS].
   G17, G18, G19 selected plane [NCMS, pages 14, 20]
   G90, G91 distance mode [NCMS, page 15]
   G93, G94 feed mode [NCMS, pages 35 - 37]
   M48, M49 overrides enabled, disabled [NCMS, pages 37 - 38]
   M3, M4, M5 spindle turning [NCMS, page 7]

   The following should be set to some value at machine start-up but
   not automatically reset by any of the stopping codes.
   1. G20, G21 length units [NCMS, page 15]. This is up to the installer.
   2. motion_control_mode. This is set in rs274ngc_init but not reset here.
   Might add it here.

   The following resets have been added by calling the appropriate
   canonical machining command and/or by resetting interpreter
   settings. They occur on M2 or M30.

   1. Axis offsets are set to zero (like g92.2) and      - SET_ORIGIN_OFFSETS
   origin offsets are set to the default (like G54)
   2. Selected plane is set to CANON_PLANE_XY (like G17) - SELECT_PLANE
   3. Distance mode is set to MODE_ABSOLUTE (like G90)   - no canonical call
   4. Feed mode is set to UNITS_PER_MINUTE (like G94)    - no canonical call
   5. Feed and speed overrides are set to ON (like M48)  - ENABLE_FEED_OVERRIDE
   - ENABLE_SPEED_OVERRIDE
   6. Cutter compensation is turned off (like G40)       - no canonical call
   7. The spindle is stopped (like M5)                   - STOP_SPINDLE_TURNING
   8. The motion mode is set to G_1 (like G1)            - no canonical call
   9. Coolant is turned off (like M9)                    - FLOOD_OFF & MIST_OFF

   */

    int rs274ngc::convert_stop(                      /* ARGUMENTS                                    */
    block_t& block,                          /* pointer to a block of RS274/NGC instructions */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        if (block.m_modes[4] == 0)
        {
            PROGRAM_STOP();
        }
        else if (block.m_modes[4] == 60)
        {
            PALLET_SHUTTLE();
            PROGRAM_STOP();
        }
        else if (block.m_modes[4] == 1)
        {
            OPTIONAL_PROGRAM_STOP();
        }
        else if ((block.m_modes[4] == 2) or (block.m_modes[4] == 30))
        {                                         /* reset stuff here */
   /*1*/
            settings.current.x = settings.current.x
                + settings.origin_offset.x + settings.axis_offset.x;
            settings.current.y = settings.current.y
                + settings.origin_offset.y + settings.axis_offset.y;
            settings.current.z = settings.current.z
                + settings.origin_offset.z + settings.axis_offset.z;
            settings.current.a = settings.current.a
                + settings.origin_offset.a + settings.axis_offset.a;
            settings.current.b = settings.current.b
                + settings.origin_offset.b + settings.axis_offset.b;
            settings.current.c = settings.current.c
                + settings.origin_offset.c + settings.axis_offset.c;

            settings.origin_index = 1;
            settings.parameters[5220] = 1.0;
            settings.origin_offset.x = settings.parameters[5221];
            settings.origin_offset.y = settings.parameters[5222];
            settings.origin_offset.z = settings.parameters[5223];
            settings.origin_offset.a = settings.parameters[5224];
            settings.origin_offset.b = settings.parameters[5225];
            settings.origin_offset.c = settings.parameters[5226];

            settings.axis_offset.x = 0;
            settings.axis_offset.x = 0;
            settings.axis_offset.x = 0;
            settings.axis_offset.a = 0;    /*AA*/
            settings.axis_offset.b = 0;    /*BB*/
            settings.axis_offset.c = 0;    /*CC*/

            settings.current.x = settings.current.x -
                settings.origin_offset.x;
            settings.current.y = settings.current.y -
                settings.origin_offset.y;
            settings.current.z = settings.current.z -
                settings.origin_offset.z;
            settings.current.a = settings.current.a -
                settings.origin_offset.a;       /*AA*/
            settings.current.b = settings.current.b -
                settings.origin_offset.b;       /*BB*/
            settings.current.c = settings.current.c -
                settings.origin_offset.c;       /*CC*/

            SET_ORIGIN_OFFSETS(settings.origin_offset.x,
                settings.origin_offset.y,
                settings.origin_offset.z
                ,            settings.origin_offset.a
                ,            settings.origin_offset.b
                ,            settings.origin_offset.c
                );

            /*2*/ if (settings.plane != CANON_PLANE_XY)
            {
                SELECT_PLANE(CANON_PLANE_XY);
                settings.plane = CANON_PLANE_XY;
            }

            /*3*/ settings.distance_mode = MODE_ABSOLUTE;

            /*4*/ settings.feed_mode = UNITS_PER_MINUTE;

            /*5*/ if (settings.feed_override != ON)
            {
                ENABLE_FEED_OVERRIDE();
                settings.feed_override = ON;
            }
            if (settings.speed_override != ON)
            {
                ENABLE_SPEED_OVERRIDE();
                settings.speed_override = ON;
            }

            /*6*/ settings.cutter_comp_side = OFF;
            settings.program_x = UNKNOWN;

            /*7*/ STOP_SPINDLE_TURNING();
            settings.spindle_turning = CANON_STOPPED;

            /*8*/ settings.motion_mode = G_1;

            /*9*/ if (settings.mist == ON)
            {
                MIST_OFF();
                settings.mist = OFF;
            }
            if (settings.flood == ON)
            {
                FLOOD_OFF();
                settings.flood = OFF;
            }

            if (block.m_modes[4] == 30)
                PALLET_SHUTTLE();
            PROGRAM_END();
            return RS274NGC_EXIT;
        }
        else
            throw error(NCE_BUG_CODE_NOT_M0_M1_M2_M30_M60);
        
        return RS274NGC_OK;
    }

   /****************************************************************************/

   /* convert_straight

   Returned Value: int
   If convert_straight_comp1 or convert_straight_comp2 is called
   and returns an error code, this returns that code.
   If any of the following errors occur, this returns the error shown.
   Otherwise, it returns RS274NGC_OK.
   1. x, y, z, a, b, and c are all missing from the block:
   NCE_ALL_AXES_MISSING_WITH_G0_OR_G1
   2. The value of move is not G_0 or G_1:
   NCE_BUG_CODE_NOT_G0_OR_G1
   3. A straight feed (g1) move is called with feed rate set to 0:
   NCE_CANNOT_DO_G1_WITH_ZERO_FEED_RATE
   4. A straight feed (g1) move is called with inverse time feed in effect
   but no f word (feed time) is provided:
   NCE_F_WORD_MISSING_WITH_INVERSE_TIME_G1_MOVE
   5. A move is called with G53 and cutter radius compensation on:
   NCE_CANNOT_USE_G53_WITH_CUTTER_RADIUS_COMP

   Side effects:
   This executes a STRAIGHT_FEED command at cutting feed rate
   (if move is G_1) or a STRAIGHT_TRAVERSE command (if move is G_0).
   It also updates the setting of the position of the tool point to the
   end point of the move. If cutter radius compensation is on, it may
   also generate an arc before the straight move. Also, in INVERSE_TIME
   feed mode, SET_FEED_RATE will be called the feed rate setting changed.

   Called by: convert_motion.

   The approach to operating in incremental distance mode (g91) is to
   put the the absolute position values into the block before using the
   block to generate a move.

   In inverse time feed mode, a lower bound of 0.1 is placed on the feed
   rate so that the feed rate is never set to zero. If the destination
   point is the same as the current point, the feed rate would be
   calculated as zero otherwise.

   */

    void rs274ngc::convert_straight(                  /* ARGUMENTS                                */
    int move,                                     /* either G_0 or G_1                        */
    block_t& block,                          /* pointer to a block of RS274 instructions */
    setup_t& settings)                       /* pointer to machine settings              */
    {
        double end_x;
        double end_y;
        double end_z;
        double AA_end;                            /*AA*/
        double BB_end;                            /*BB*/
        double CC_end;                            /*CC*/

        if (move == G_1)
        {
            if (settings.feed_mode == UNITS_PER_MINUTE)
            {
                error_if((settings.feed_rate == 0.0), NCE_CANNOT_DO_G1_WITH_ZERO_FEED_RATE);
            }
            else if (settings.feed_mode == INVERSE_TIME)
            {
                error_if((block.f_number == -1.0), NCE_F_WORD_MISSING_WITH_INVERSE_TIME_G1_MOVE);
            }
        }

        settings.motion_mode = move;
        find_ends(block, settings, &end_x, &end_y,
            &end_z
            , &AA_end
            , &BB_end
            , &CC_end
            );
   /* not "IS ON" */
        if ((settings.cutter_comp_side != OFF) and
            (settings.cutter_comp_radius > 0.0)) /* radius always is >= 0 */
        {
            error_if((block.g_modes[0] == G_53), NCE_CANNOT_USE_G53_WITH_CUTTER_RADIUS_COMP);
            if (settings.program_x == UNKNOWN)
            {
                    convert_straight_comp1(move, block, settings, end_x, end_y,
                    end_z
                    , AA_end
                    , BB_end
                    , CC_end
                    );
            }
            else
            {
                    convert_straight_comp2 (move, block, settings, end_x, end_y,
                    end_z
                    , AA_end
                    , BB_end
                    , CC_end
                    );
            }
        }
        else if (move == G_0)
        {
            STRAIGHT_TRAVERSE(end_x, end_y, end_z
                ,           AA_end
                ,  BB_end
                ,  CC_end
                );
            settings.current.x = end_x;
            settings.current.y = end_y;
        }
        else if (move == G_1)
        {
            if (settings.feed_mode == INVERSE_TIME)
                inverse_time_rate_straight
                    (end_x, end_y, end_z
                    , AA_end
                    , BB_end
                    , CC_end
                    , block, settings);
            STRAIGHT_FEED(end_x, end_y, end_z
                ,           AA_end
                ,  BB_end
                ,  CC_end
                );
            settings.current.x = end_x;
            settings.current.y = end_y;
        }
        else
            throw error(NCE_BUG_CODE_NOT_G0_OR_G1);

        settings.current.z = end_z;
        settings.current.a = AA_end;       /*AA*/
        settings.current.b = BB_end;       /*BB*/
        settings.current.c = CC_end;       /*CC*/
    }

   /****************************************************************************/

   /* convert_straight_comp1

   Returned Value: int
   If any of the following errors occur, this returns the error shown.
   Otherwise, it returns RS274NGC_OK.
   1. The side is not RIGHT or LEFT:
   NCE_BUG_SIDE_NOT_RIGHT_OR_LEFT
   2. The destination tangent point is not more than a tool radius
   away (indicating gouging): NCE_CUTTER_GOUGING_WITH_CUTTER_RADIUS_COMP
   3. The value of move is not G_0 or G_1
   NCE_BUG_CODE_NOT_G0_OR_G1

   Side effects:
   This executes a STRAIGHT_MOVE command at cutting feed rate
   or a STRAIGHT_TRAVERSE command.
   It also updates the setting of the position of the tool point
   to the end point of the move and updates the programmed point.
   If INVERSE_TIME feed rate mode is in effect, it resets the feed rate.

   Called by: convert_straight.

   This is called if cutter radius compensation is on and settings.program_x
   is UNKNOWN, indicating that this is the first move after cutter radius
   compensation is turned on.

   The algorithm used here for determining the path is to draw a straight
   line from the destination point which is tangent to a circle whose
   center is at the current point and whose radius is the radius of the
   cutter. The destination point of the cutter tip is then found as the
   center of a circle of the same radius tangent to the tangent line at
   the destination point.

   */

    void rs274ngc::convert_straight_comp1(            /* ARGUMENTS                       */
    int move,                                     /* either G_0 or G_1                         */
    block_t& block,                          /* pointer to a block of RS274 instructions  */
    setup_t& settings,                       /* pointer to machine settings               */
    double px,                                    /* X coordinate of end point                 */
    double py,                                    /* Y coordinate of end point                 */
    double end_z                                  /* Z coordinate of end point                 */
    , double AA_end                               /* A coordinate of end point           *//*AA*/
    , double BB_end                               /* B coordinate of end point           *//*BB*/
    , double CC_end                               /* C coordinate of end point           *//*CC*/
    )
    {
        double alpha;
        double cx;                                /* first current point x then end point x */
        double cy;                                /* first current point y then end point y */
        double distance;
        double radius;
        int side;
        double theta;

        side = settings.cutter_comp_side;
        cx = settings.current.x;
        cy = settings.current.y;

   /* always will be positive */
        radius = settings.cutter_comp_radius;
        distance = hypot((px - cx), (py -cy));

        error_if(((side != LEFT) and (side != RIGHT)), NCE_BUG_SIDE_NOT_RIGHT_OR_LEFT);
        error_if((distance <= radius), NCE_CUTTER_GOUGING_WITH_CUTTER_RADIUS_COMP);

        theta = acos(radius/distance);
        alpha = (side == LEFT) ? (atan2((cy - py), (cx - px)) - theta) :
        (atan2((cy - py), (cx - px)) + theta);
        cx = (px + (radius * cos(alpha)));   /* reset to end location */
        cy = (py + (radius * sin(alpha)));
        if (move == G_0)
            STRAIGHT_TRAVERSE(cx, cy, end_z
                ,             AA_end
                ,  BB_end
                ,  CC_end
                );
        else if (move == G_1)
        {
            if (settings.feed_mode == INVERSE_TIME)
                inverse_time_rate_straight
                    (cx, cy, end_z
                    , AA_end
                    , BB_end
                    , CC_end
                    , block, settings);
            STRAIGHT_FEED(cx, cy, end_z
                ,           AA_end
                ,  BB_end
                ,  CC_end
                );
        }
        else
            throw error(NCE_BUG_CODE_NOT_G0_OR_G1);

        settings.current.x = cx;
        settings.current.y = cy;
        settings.program_x = px;
        settings.program_y = py;
    }

   /****************************************************************************/

   /* convert_straight_comp2

   Returned Value: int
   If any of the following errors occur, this returns the error shown.
   Otherwise, it returns RS274NGC_OK.
   1. The compensation side is not RIGHT or LEFT:
   NCE_BUG_SIDE_NOT_RIGHT_OR_LEFT
   2. A concave corner is found:
   NCE_CONCAVE_CORNER_WITH_CUTTER_RADIUS_COMP

   Side effects:
   This executes a STRAIGHT_FEED command at cutting feed rate
   or a STRAIGHT_TRAVERSE command.
   It also generates an ARC_FEED to go around a corner, if necessary.
   It also updates the setting of the position of the tool point to
   the end point of the move and updates the programmed point.
   If INVERSE_TIME feed mode is in effect, it also calls SET_FEED_RATE
   and resets the feed rate in the machine model.

   Called by: convert_straight.

   This is called if cutter radius compensation is on and
   settings.program_x is not UNKNOWN, indicating that this is not the
   first move after cutter radius compensation is turned on.

   The algorithm used here is:
   1. Determine the direction of the last motion. This is done by finding
   the direction of the line from the last programmed point to the
   current tool tip location. This line is a radius of the tool and is
   perpendicular to the direction of motion since the cutter is tangent
   to that direction.
   2. Determine the direction of the programmed motion.
   3. If there is a convex corner, insert an arc to go around the corner.
   4. Find the destination point for the tool tip. The tool will be
   tangent to the line from the last programmed point to the present
   programmed point at the present programmed point.
   5. Go in a straight line from the current tool tip location to the
   destination tool tip location.

   This uses an angle tolerance of TOLERANCE_CONCAVE_CORNER (0.01 radian)
   to determine if:
   1) an illegal concave corner exists (tool will not fit into corner),
   2) no arc is required to go around the corner (i.e. the current line
   is in the same direction as the end of the previous move), or
   3) an arc is required to go around a convex corner and start off in
   a new direction.

   If a rotary axis is moved in this block and an extra arc is required
   to go around a sharp corner, all the rotary axis motion occurs on the
   arc.  An alternative might be to distribute the rotary axis motion
   over the arc and the straight move in proportion to their lengths.

   If the Z-axis is moved in this block and an extra arc is required to
   go around a sharp corner, all the Z-axis motion occurs on the straight
   line and none on the extra arc.  An alternative might be to distribute
   the Z-axis motion over the extra arc and the straight line in
   proportion to their lengths.

   This handles inverse time feed rates by computing the length of the
   compensated path.

   This handles the case of there being no XY motion.

   This handles G0 moves. Where an arc is inserted to round a corner in a
   G1 move, no arc is inserted for a G0 move; a STRAIGHT_TRAVERSE is made
   from the current point to the end point. The end point for a G0
   move is the same as the end point for a G1 move, however.

   */

    void rs274ngc::convert_straight_comp2(            /* ARGUMENTS                       */
    int move,                                     /* either G_0 or G_1                         */
    block_t& block,                          /* pointer to a block of RS274 instructions  */
    setup_t& settings,                       /* pointer to machine settings               */
    double px,                                    /* X coordinate of programmed end point      */
    double py,                                    /* Y coordinate of programmed end point      */
    double end_z                                  /* Z coordinate of end point                 */
    , double AA_end                               /* A coordinate of end point           *//*AA*/
    , double BB_end                               /* B coordinate of end point           *//*BB*/
    , double CC_end                               /* C coordinate of end point           *//*CC*/
    )
    {
        double alpha;
        double beta;
        double end_x;                             /* x-coordinate of actual end point */
        double end_y;                             /* y-coordinate of actual end point */
        double gamma;
        double mid_x;                             /* x-coordinate of end of added arc, if needed */
        double mid_y;                             /* y-coordinate of end of added arc, if needed */
        double radius;
        int side;
   /* radians, testing corners */
        double small = TOLERANCE_CONCAVE_CORNER;
        double start_x, start_y;                  /* programmed beginning point */
        double theta;

        start_x = settings.program_x;
        start_y = settings.program_y;
        if ((py == start_y) and (px == start_x))  /* no XY motion */
        {
            end_x = settings.current.x;
            end_y = settings.current.y;
            if (move == G_0)
                STRAIGHT_TRAVERSE(end_x, end_y, end_z
                    ,             AA_end
                    ,  BB_end
                    ,  CC_end
                    );
            else if (move == G_1)
            {
                if (settings.feed_mode == INVERSE_TIME)
                    inverse_time_rate_straight
                        (end_x, end_y, end_z
                        , AA_end
                        , BB_end
                        , CC_end
                        , block, settings);
                STRAIGHT_FEED(end_x, end_y, end_z
                    ,           AA_end
                    ,  BB_end
                    ,  CC_end
                    );
            }
            else
                throw error(NCE_BUG_CODE_NOT_G0_OR_G1);
        }
        else
        {
            side = settings.cutter_comp_side;
   /* will always be positive */
            radius = settings.cutter_comp_radius;
            theta = atan2(settings.current.y - start_y,
                settings.current.x - start_x);
            alpha = atan2(py - start_y, px - start_x);

            if (side == LEFT)
            {
                if (theta < alpha)
                    theta = (theta + TWO_PI);
                beta = ((theta - alpha) - PI2);
                gamma = PI2;
            }
            else if (side == RIGHT)
            {
                if (alpha < theta)
                    alpha = (alpha + TWO_PI);
                beta = ((alpha - theta) - PI2);
                gamma = -PI2;
            }
            else
                throw error(NCE_BUG_SIDE_NOT_RIGHT_OR_LEFT);
            end_x = (px + (radius * cos(alpha + gamma)));
            end_y = (py + (radius * sin(alpha + gamma)));
            mid_x = (start_x + (radius * cos(alpha + gamma)));
            mid_y = (start_y + (radius * sin(alpha + gamma)));

            error_if(((beta < -small) or (beta > (PI + small))), NCE_CONCAVE_CORNER_WITH_CUTTER_RADIUS_COMP);
            if (move == G_0)
                STRAIGHT_TRAVERSE(end_x, end_y, end_z
                    ,             AA_end
                    ,  BB_end
                    ,  CC_end
                    );
            else if (move == G_1)
            {
                if (beta > small)                 /* ARC NEEDED */
                {
                    if (settings.feed_mode == INVERSE_TIME)
                        inverse_time_rate_as(start_x, start_y, (side == LEFT) ? -1 : 1,
                        mid_x, mid_y, end_x, end_y,
                        end_z
                        , AA_end
                        , BB_end
                        , CC_end
                        ,
                        block, settings);
                    ARC_FEED(mid_x,mid_y,start_x, start_y,((side == LEFT) ? -1 : 1),
                        settings.current.z
                        , AA_end
                        , BB_end
                        , CC_end
                        );
                    STRAIGHT_FEED(end_x, end_y, end_z
                        ,               AA_end
                        ,  BB_end
                        ,  CC_end
                        );
                }
                else
                {
                    if (settings.feed_mode == INVERSE_TIME)
                        inverse_time_rate_straight
                            (end_x,end_y,end_z
                            , AA_end
                            , BB_end
                            , CC_end
                            , block, settings);
                    STRAIGHT_FEED(end_x, end_y, end_z
                        ,               AA_end
                        ,  BB_end
                        ,  CC_end
                        );
                }
            }
            else
                throw error(NCE_BUG_CODE_NOT_G0_OR_G1);
        }

        settings.current.x = end_x;
        settings.current.y = end_y;
        settings.program_x = px;
        settings.program_y = py;
    }

   /****************************************************************************/

   /* convert_tool_change

   Returned Value: int (RS274NGC_OK)

   Side effects:
   This makes function calls to canonical machining functions, and sets
   the machine model as described below.

   Called by: convert_m

   This function carries out an m6 command, which changes the tool in the
   spindle. The only function call this makes is to the CHANGE_TOOL
   function. The semantics of this function call is that when it is
   completely carried out, the tool that was selected is in the spindle,
   the tool that was in the spindle (if any) is returned to its changer
   slot, the spindle will be stopped (but the spindle speed setting will
   not have changed) and the x, y, z, a, b, and c positions will be the same
   as they were before (although they may have moved around during the
   change).

   It would be nice to add more flexibility to this function by allowing
   more changes to occur (position changes, for example) as a result of
   the tool change. There are at least two ways of doing this:

   1. Require that certain machine settings always have a given fixed
   value after a tool change (which may be different from what the value
   was before the change), and record the fixed values somewhere (in the
   world model that is read at initialization, perhaps) so that this
   function can retrieve them and reset any settings that have changed.
   Fixed values could even be hard coded in this function.

   2. Allow the executor of the CHANGE_TOOL function to change the state
   of the world however it pleases, and have the interpreter read the
   executor's world model after the CHANGE_TOOL function is carried out.
   Implementing this would require a change in other parts of the EMC
   system, since calls to the interpreter would then have to be
   interleaved with execution of the function calls output by the
   interpreter.

   There may be other commands in the block that includes the tool change.
   They will be executed in the order described in execute_block.

   This implements the "Next tool in T word" approach to tool selection.
   The tool is selected when the T word is read (and the carousel may
   move at that time) but is changed when M6 is read.

   Note that if a different tool is put into the spindle, the current_z
   location setting may be incorrect for a time. It is assumed the
   program will contain an appropriate USE_TOOL_LENGTH_OFFSET command
   near the CHANGE_TOOL command, so that the incorrect setting is only
   temporary.

   In [NCMS, page 73, 74] there are three other legal approaches in addition
   to this one.

   */

    void rs274ngc::convert_tool_change(               /* ARGUMENTS                   */
    setup_t& settings)                       /* pointer to machine settings */
    {
        CHANGE_TOOL(settings.selected_tool_slot);
        settings.current_slot = settings.selected_tool_slot;
        settings.spindle_turning = CANON_STOPPED;
    }

   /****************************************************************************/

   /* convert_tool_length_offset

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The block has no offset index (h number): NCE_OFFSET_INDEX_MISSING
   2. The g_code argument is not G_43 or G_49:
   NCE_BUG_CODE_NOT_G43_OR_G49

   Side effects:
   A USE_TOOL_LENGTH_OFFSET function call is made. Current_z,
   tool_length_offset, and length_offset_index are reset.

   Called by: convert_g

   This is called to execute g43 or g49.

   The g49 RS274/NGC command translates into a USE_TOOL_LENGTH_OFFSET(0.0)
   function call.

   The g43 RS274/NGC command translates into a USE_TOOL_LENGTH_OFFSET(length)
   function call, where length is the value of the entry in the tool length
   offset table whose index is the H number in the block.

   The H number in the block (if present) was checked for being a non-negative
   integer when it was read, so that check does not need to be repeated.

   */

    void rs274ngc::convert_tool_length_offset(        /* ARGUMENTS                      */
    int g_code,                                   /* g_code being executed (must be G_43 or G_49) */
    block_t& block,                          /* pointer to a block of RS274/NGC instructions */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        int index;
        double offset;

        if (g_code == G_49)
        {
            USE_TOOL_LENGTH_OFFSET(0.0);
            settings.current.z = (settings.current.z +
                settings.tool_length_offset);
            settings.tool_length_offset = 0.0;
            settings.length_offset_index = 0;
        }
        else if (g_code == G_43)
        {
            index = block.h_number;
            error_if((index == -1), NCE_OFFSET_INDEX_MISSING);
            offset = settings.tool_table[index].length;
            USE_TOOL_LENGTH_OFFSET(offset);
            settings.current.z =
                (settings.current.z + settings.tool_length_offset - offset);
            settings.tool_length_offset = offset;
            settings.length_offset_index = index;
        }
        else
            throw error(NCE_BUG_CODE_NOT_G43_OR_G49);
    }

   /****************************************************************************/

   /* convert_tool_select

   Returned Value: int
   If the tool slot given in the block is larger than allowed,
   this returns NCE_SELECTED_TOOL_SLOT_NUMBER_TOO_LARGE.
   Otherwise, it returns RS274NGC_OK.

   Side effects: See below

   Called by: execute_block

   A select tool command is given, which causes the changer chain to move
   so that the slot with the t_number given in the block is next to the
   tool changer, ready for a tool change.  The
   settings.selected_tool_slot is set to the given slot.

   An alternative in this function is to select by tool id. This was used
   in the K&T and VGER interpreters. It is easy to code.

   A check that the t_number is not negative has already been made in read_t.
   A zero t_number is allowed and means no tool should be selected.

   */

    void rs274ngc::convert_tool_select(               /* ARGUMENTS                                */
    block_t& block,                          /* pointer to a block of RS274 instructions */
    setup_t& settings)                       /* pointer to machine settings              */
    {
        error_if((block.t_number > settings.tool_max), NCE_SELECTED_TOOL_SLOT_NUMBER_TOO_LARGE);
        SELECT_TOOL(block.t_number);
        settings.selected_tool_slot = block.t_number;
    }

   /****************************************************************************/

   /* cycle_feed

   Returned Value: int (RS274NGC_OK)

   Side effects:
   STRAIGHT_FEED is called.

   Called by:
   convert_cycle_g81
   convert_cycle_g82
   convert_cycle_g83
   convert_cycle_g84
   convert_cycle_g85
   convert_cycle_g86
   convert_cycle_g87
   convert_cycle_g88
   convert_cycle_g89

   This writes a STRAIGHT_FEED command appropriate for a cycle move with
   respect to the given plane. No rotary axis motion takes place.

   */

    void rs274ngc::cycle_feed(                        /* ARGUMENTS                  */
    CANON_PLANE plane,                            /* currently selected plane   */
    double end1,                                  /* first coordinate value     */
    double end2,                                  /* second coordinate value    */
    double end3)                                  /* third coordinate value     */
    {
        if (plane == CANON_PLANE_XY)
            STRAIGHT_FEED(end1, end2, end3
                ,         _setup.current.a
                ,  _setup.current.b
                ,  _setup.current.c
                );
        else if (plane == CANON_PLANE_YZ)
            STRAIGHT_FEED(end3, end1, end2
                    ,         _setup.current.a
                    ,  _setup.current.b
                    ,  _setup.current.c
                    );
        else                                      /* if (plane == CANON_PLANE_XZ) */
            STRAIGHT_FEED(end2, end3, end1
                ,         _setup.current.a
                ,  _setup.current.b
                ,  _setup.current.c
                );
    }

   /****************************************************************************/

   /* cycle_traverse

   Returned Value: int (RS274NGC_OK)

   Side effects:
   STRAIGHT_TRAVERSE is called.

   Called by:
   convert_cycle
   convert_cycle_g81
   convert_cycle_g82
   convert_cycle_g83
   convert_cycle_g86
   convert_cycle_g87
   convert_cycle_xy (via CYCLE_MACRO)
   convert_cycle_yz (via CYCLE_MACRO)
   convert_cycle_zx (via CYCLE_MACRO)

   This writes a STRAIGHT_TRAVERSE command appropriate for a cycle
   move with respect to the given plane. No rotary axis motion takes place.

   */

    void rs274ngc::cycle_traverse(                    /* ARGUMENTS                 */
    CANON_PLANE plane,                            /* currently selected plane  */
    double end1,                                  /* first coordinate value    */
    double end2,                                  /* second coordinate value   */
    double end3)                                  /* third coordinate value    */
    {
        if (plane == CANON_PLANE_XY)
            STRAIGHT_TRAVERSE(end1, end2, end3
                ,             _setup.current.a
                ,  _setup.current.b
                ,  _setup.current.c
                );
        else if (plane == CANON_PLANE_YZ)
            STRAIGHT_TRAVERSE(end3, end1, end2
                    ,             _setup.current.a
                    ,  _setup.current.b
                    ,  _setup.current.c
                    );
        else                                      /* if (plane == CANON_PLANE_XZ) */
            STRAIGHT_TRAVERSE(end2, end3, end1
                ,             _setup.current.a
                ,  _setup.current.b
                ,  _setup.current.c
                );
    }

   /****************************************************************************/

   /* enhance_block

   Returned Value:
   If any of the following errors occur, this returns the error shown.
   Otherwise, it returns RS274NGC_OK.
   1. A g80 is in the block, no modal group 0 code that uses axes
   is in the block, and one or more axis values is given:
   NCE_CANNOT_USE_AXIS_VALUES_WITH_G80
   2. A g92 is in the block and no axis value is given:
   NCE_ALL_AXES_MISSING_WITH_G92
   3. One g-code from group 1 and one from group 0, both of which can use
   axis values, are in the block:
   NCE_CANNOT_USE_TWO_G_CODES_THAT_BOTH_USE_AXIS_VALUES
   4. A g-code from group 1 which can use axis values is in the block,
   but no axis value is given: NCE_ALL_AXES_MISSING_WITH_MOTION_CODE
   5. Axis values are given, but there is neither a g-code in the block
   nor an active previously given modal g-code that uses axis values:
   NCE_CANNOT_USE_AXIS_VALUES_WITHOUT_A_G_CODE_THAT_USES_THEM

   Side effects:
   The value of motion_to_be in the block is set.

   Called by: parse_line

   If there is a g-code for motion in the block (in g_modes[1]),
   set motion_to_be to that. Otherwise, if there is an axis value in the
   block and no g-code to use it (any such would be from group 0 in
   g_modes[0]), set motion_to_be to be the last motion saved (in
   settings.motion mode).

   This also make the checks described above.

   */

    void rs274ngc::enhance_block(                     /* ARGUMENTS                         */
    block_t& block,                          /* pointer to a block to be checked  */
    setup_t& settings)                       /* pointer to machine settings       */
    {
        int axis_flag;
        int mode_zero_covets_axes;

        axis_flag = ((block.x_flag == ON) or
            (block.y_flag == ON) or
            (block.a_flag == ON) or              /*AA*/
            (block.b_flag == ON) or              /*BB*/
            (block.c_flag == ON) or              /*CC*/
            (block.z_flag == ON));
        mode_zero_covets_axes = ((block.g_modes[0] == G_10) or
            (block.g_modes[0] == G_28) or
            (block.g_modes[0] == G_30) or
            (block.g_modes[0] == G_92));

        if (block.g_modes[1] != -1)
        {
            if (block.g_modes[1] == G_80)
            {
                error_if((axis_flag and (not mode_zero_covets_axes)), NCE_CANNOT_USE_AXIS_VALUES_WITH_G80);
                error_if(((not axis_flag) and (block.g_modes[0] == G_92)), NCE_ALL_AXES_MISSING_WITH_G92);
            }
            else
            {
                error_if(mode_zero_covets_axes, NCE_CANNOT_USE_TWO_G_CODES_THAT_BOTH_USE_AXIS_VALUES);
                error_if((not axis_flag), NCE_ALL_AXES_MISSING_WITH_MOTION_CODE);
            }
            block.motion_to_be = block.g_modes[1];
        }
        else if (mode_zero_covets_axes)
        {                                         /* other 3 can get by without axes but not G92 */
            error_if(((not axis_flag) and (block.g_modes[0] == G_92)), NCE_ALL_AXES_MISSING_WITH_G92);
        }
        else if (axis_flag)
        {
            error_if(((settings.motion_mode == -1) or (settings.motion_mode == G_80)),
                NCE_CANNOT_USE_AXIS_VALUES_WITHOUT_A_G_CODE_THAT_USES_THEM);
            block.motion_to_be = settings.motion_mode;
        }
    }

   /****************************************************************************/

   /* execute binary

   Returned value: int
   If execute_binary1 or execute_binary2 returns an error code, this
   returns that code.
   Otherwise, it returns RS274NGC_OK.

   Side effects: The value of left is set to the result of applying
   the operation to left and right.

   Called by: read_real_expression

   This just calls either execute_binary1 or execute_binary2.

   */

    void rs274ngc::execute_binary(
    double * left,
    int operation,
    double * right)
    {
        if (operation < AND2)
            execute_binary1(left, operation, right);
        else
            execute_binary2(left, operation, right);
    }

   /****************************************************************************/

   /* execute_binary1

   Returned Value: int
   If any of the following errors occur, this returns the error shown.
   Otherwise, it returns RS274NGC_OK.
   1. operation is unknown: NCE_BUG_UNKNOWN_OPERATION
   2. An attempt is made to divide by zero: NCE_ATTEMPT_TO_DIVIDE_BY_ZERO
   3. An attempt is made to raise a negative number to a non-integer power:
   NCE_ATTEMPT_TO_RAISE_NEGATIVE_TO_NON_INTEGER_POWER

   Side effects:
   The result from performing the operation is put into what left points at.

   Called by: read_real_expression.

   This executes the operations: DIVIDED_BY, MODULO, POWER, TIMES.

   */

    void rs274ngc::execute_binary1(                   /* ARGUMENTS                       */
    double * left,                                /* pointer to the left operand     */
    int operation,                                /* integer code for the operation  */
    double * right)                               /* pointer to the right operand    */
    {
        switch (operation)
        {
            case DIVIDED_BY:
                error_if((*right == 0.0), NCE_ATTEMPT_TO_DIVIDE_BY_ZERO);
                *left = (*left / *right);
                break;
            case MODULO:                          /* always calculates a positive answer */
                *left = fmod(*left, *right);
                if (*left < 0.0)
                {
                    *left = (*left + fabs(*right));
                }
                break;
            case POWER:
                error_if(((*left < 0.0) and (floor(*right) != *right)), NCE_ATTEMPT_TO_RAISE_NEGATIVE_TO_NON_INTEGER_POWER);
                *left = pow(*left, *right);
                break;
            case TIMES:
                *left = (*left * *right);
                break;
            default:
                throw error(NCE_BUG_UNKNOWN_OPERATION);
        }
    }

   /****************************************************************************/

   /* execute_binary2

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. operation is unknown: NCE_BUG_UNKNOWN_OPERATION

   Side effects:
   The result from performing the operation is put into what left points at.

   Called by: read_real_expression.

   This executes the operations: AND2, EXCLUSIVE_OR, MINUS,
   NON_EXCLUSIVE_OR, PLUS. The RS274/NGC manual [NCMS] does not say what
   the calculated value of the three logical operations should be. This
   function calculates either 1.0 (meaning true) or 0.0 (meaning false).
   Any non-zero input value is taken as meaning true, and only 0.0 means
   false.

   */

    void rs274ngc::execute_binary2(                   /* ARGUMENTS                       */
    double * left,                                /* pointer to the left operand     */
    int operation,                                /* integer code for the operation  */
    double * right)                               /* pointer to the right operand    */
    {
        switch (operation)
        {
            case AND2:
                *left = ((*left == 0.0) or (*right == 0.0)) ? 0.0 : 1.0;
                break;
            case EXCLUSIVE_OR:
                *left = (((*left == 0.0) and (*right != 0.0)) or
                    ((*left != 0.0) and (*right == 0.0))) ? 1.0 : 0.0;
                break;
            case MINUS:
                *left = (*left - *right);
                break;
            case NON_EXCLUSIVE_OR:
                *left = ((*left != 0.0) or (*right != 0.0)) ? 1.0 : 0.0;
                break;
            case PLUS:
                *left = (*left + *right);
                break;
            default:
                throw error(NCE_BUG_UNKNOWN_OPERATION);
        }
    }

   /****************************************************************************/

   /* execute_block

   Returned Value: int
   If convert_stop returns RS274NGC_EXIT, this returns RS274NGC_EXIT.
   If any of the following functions is called and returns an error code,
   this returns that code.
   convert_comment
   convert_feed_mode
   convert_feed_rate
   convert_g
   convert_m
   convert_speed
   convert_stop
   convert_tool_select
   Otherwise, if the probe_flag in the settings is ON, this returns
   RS274NGC_EXECUTE_FINISH.
   Otherwise, it returns RS274NGC_OK.

   Side effects:
   One block of RS274/NGC instructions is executed.

   Called by:
   rs274ngc_execute

   This converts a block to zero to many actions. The order of execution
   of items in a block is critical to safe and effective machine operation,
   but is not specified clearly in the RS274/NGC documentation.

   Actions are executed in the following order:
   1. any comment.
   2. a feed mode setting (g93, g94)
   3. a feed rate (f) setting if in units_per_minute feed mode.
   4. a spindle speed (s) setting.
   5. a tool selection (t).
   6. "m" commands as described in convert_m (includes tool change).
   7. any g_codes (except g93, g94) as described in convert_g.
   8. stopping commands (m0, m1, m2, m30, or m60).

   In inverse time feed mode, the explicit and implicit g code executions
   include feed rate setting with g1, g2, and g3. Also in inverse time
   feed mode, attempting a canned cycle cycle (g81 to g89) or setting a
   feed rate with g0 is illegal and will be detected and result in an
   error message.

   */

    int rs274ngc::execute_block(                     /* ARGUMENTS                                    */
    block_t& block,                          /* pointer to a block of RS274/NGC instructions */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        int status;

        if (block.comment[0] != 0)
        {
            convert_comment(block.comment);
        }
        if (block.g_modes[5] != -1)
        {
            convert_feed_mode(block.g_modes[5], settings);
        }
        if (block.f_number > -1.0)
        {
   /* handle elsewhere */
            if (settings.feed_mode == INVERSE_TIME);
            else
            {
                convert_feed_rate(block, settings);
            }
        }
        if (block.s_number > -1.0)
        {
            convert_speed(block, settings);
        }
        if (block.t_number != -1)
        {
            convert_tool_select(block, settings);
        }
        convert_m(block, settings);
        convert_g(block, settings);
        if (block.m_modes[4] != -1)            /* converts m0, m1, m2, m30, or m60 */
        {
            status = convert_stop(block, settings);
            if (status == RS274NGC_EXIT)
                return RS274NGC_EXIT;
            else if (status != RS274NGC_OK)
                throw error(status);
        }
        return ((settings.probe_flag == ON) ? RS274NGC_EXECUTE_FINISH: RS274NGC_OK);
    }

   /****************************************************************************/

   /* execute_unary

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. the operation is unknown: NCE_BUG_UNKNOWN_OPERATION
   2. the argument to acos is not between minus and plus one:
   NCE_ARGUMENT_TO_ACOS_OUT_RANGE
   3. the argument to asin is not between minus and plus one:
   NCE_ARGUMENT_TO_ASIN_OUT_RANGE
   4. the argument to the natural logarithm is not positive:
   NCE_ZERO_OR_NEGATIVE_ARGUMENT_TO_LN
   5. the argument to square root is negative:
   NCE_NEGATIVE_ARGUMENT_TO_SQRT

   Side effects:
   The result from performing the operation on the value in double_ptr
   is put into what double_ptr points at.

   Called by: read_unary.

   This executes the operations: ABS, ACOS, ASIN, COS, EXP, FIX, FUP, LN
   ROUND, SIN, SQRT, TAN

   All angle measures in the input or output are in degrees.

   */

    void rs274ngc::execute_unary(                     /* ARGUMENTS                       */
    double * double_ptr,                          /* pointer to the operand          */
    int operation)                                /* integer code for the operation  */
    {
        switch (operation)
        {
            case ABS:
                if (*double_ptr < 0.0)
                    *double_ptr = (-1.0 * *double_ptr);
                break;
            case ACOS:
                error_if(((*double_ptr < -1.0) or (*double_ptr > 1.0)), NCE_ARGUMENT_TO_ACOS_OUT_OF_RANGE);
                *double_ptr = acos(*double_ptr);
                *double_ptr = ((*double_ptr * 180.0)/ PI);
                break;
            case ASIN:
                error_if(((*double_ptr < -1.0) or (*double_ptr > 1.0)), NCE_ARGUMENT_TO_ASIN_OUT_OF_RANGE);
                *double_ptr = asin(*double_ptr);
                *double_ptr = ((*double_ptr * 180.0)/ PI);
                break;
            case COS:
                *double_ptr = cos((*double_ptr * PI)/180.0);
                break;
            case EXP:
                *double_ptr = exp(*double_ptr);
                break;
            case FIX:
                *double_ptr = floor(*double_ptr);
                break;
            case FUP:
                *double_ptr = ceil(*double_ptr);
                break;
            case LN:
                error_if((*double_ptr <= 0.0), NCE_ZERO_OR_NEGATIVE_ARGUMENT_TO_LN);
                *double_ptr = log(*double_ptr);
                break;
            case ROUND:
                *double_ptr = (double)
                    ((int) (*double_ptr + ((*double_ptr < 0.0) ? -0.5 : 0.5)));
                break;
            case SIN:
                *double_ptr = sin((*double_ptr * PI)/180.0);
                break;
            case SQRT:
                error_if((*double_ptr < 0.0), NCE_NEGATIVE_ARGUMENT_TO_SQRT);
                *double_ptr = sqrt(*double_ptr);
                break;
            case TAN:
                *double_ptr = tan((*double_ptr * PI)/180.0);
                break;
            default:
                throw error(NCE_BUG_UNKNOWN_OPERATION);
        }
    }

   /****************************************************************************/

   /* find_arc_length

   Returned Value: double (length of path between start and end points)

   Side effects: none

   Called by:
   inverse_time_rate_arc
   inverse_time_rate_arc2
   inverse_time_rate_as

   This calculates the length of the path that will be made relative to
   the XYZ axes for a motion in which the X,Y,Z, motion is a circular or
   helical arc with its axis parallel to the Z-axis. If tool length
   compensation is on, this is the path of the tool tip; if off, the
   length of the path of the spindle tip. Any rotary axis motion is
   ignored.

   If the arc is helical, it is coincident with the hypotenuse of a right
   triangle wrapped around a cylinder. If the triangle is unwrapped, its
   base is [the radius of the cylinder times the number of radians in the
   helix] and its height is [z2 - z1], and the path length can be found
   by the Pythagorean theorem.

   This is written as though it is only for arcs whose axis is parallel to
   the Z-axis, but it will serve also for arcs whose axis is parallel
   to the X-axis or Y-axis, with suitable permutation of the arguments.

   This works correctly when turn is zero (find_turn returns 0 in that
   case).

   */

    double rs274ngc::find_arc_length(                /* ARGUMENTS                          */
    double x1,                                    /* X-coordinate of start point        */
    double y1,                                    /* Y-coordinate of start point        */
    double z1,                                    /* Z-coordinate of start point        */
    double center_x,                              /* X-coordinate of arc center         */
    double center_y,                              /* Y-coordinate of arc center         */
    int turn,                                     /* no. of full or partial circles CCW */
    double x2,                                    /* X-coordinate of end point          */
    double y2,                                    /* Y-coordinate of end point          */
    double z2)                                    /* Z-coordinate of end point          */
    {
        double radius;
        double theta;                             /* amount of turn of arc in radians */

        radius = hypot((center_x - x1), (center_y - y1));
        theta = find_turn(x1, y1, center_x, center_y, turn, x2, y2);
        if (z2 == z1)
            return (radius * fabs(theta));
        else
            return hypot((radius * theta), (z2 - z1));
    }

   /****************************************************************************/

   /* find_ends

   Returned Value: int (RS274NGC_OK)

   Side effects:
   The values of px, py, pz, aa_p, bb_p, and cc_p are set

   Called by:
   convert_arc
   convert_home
   convert_probe
   convert_straight

   This finds the coordinates of a point, "end", in the currently
   active coordinate system, and sets the values of the pointers to the
   coordinates (which are the arguments to the function).

   In all cases, if no value for the coodinate is given in the block, the
   current value for the coordinate is used. When cutter radius
   compensation is on, this function is called before compensation
   calculations are performed, so the current value of the programmed
   point is used, not the current value of the actual current_point.

   There are three cases for when the coordinate is included in the block:

   1. G_53 is active. This means to interpret the coordinates as machine
   coordinates. That is accomplished by adding the two offsets to the
   coordinate given in the block.

   2. Absolute coordinate mode is in effect. The coordinate in the block
   is used.

   3. Incremental coordinate mode is in effect. The coordinate in the
   block plus either (i) the programmed current position - when cutter
   radius compensation is in progress, or (2) the actual current position.

   */

    void rs274ngc::find_ends(                         /* ARGUMENTS                                    */
    block_t& block,                          /* pointer to a block of RS274/NGC instructions */
    setup_t& settings,                       /* pointer to machine settings                  */
    double * px,                                  /* pointer to end_x                             */
    double * py,                                  /* pointer to end_y                             */
    double * pz                                   /* pointer to end_z                             */
    , double * AA_p                               /* pointer to end_a                       *//*AA*/
    , double * BB_p                               /* pointer to end_b                       *//*BB*/
    , double * CC_p                               /* pointer to end_c                       *//*CC*/
    )
    {
        int mode;
        int middle;
        int comp;

        mode = settings.distance_mode;
        middle = (settings.program_x != UNKNOWN);
        comp = (settings.cutter_comp_side != OFF);

        if (block.g_modes[0] == G_53)            /* distance mode is absolute in this case */
        {
#ifdef DEBUG_EMC
            COMMENT("interpreter: offsets temporarily suspended");
#endif
            *px = (block.x_flag == ON) ? (block.x_number -
                (settings.origin_offset.x + settings.axis_offset.x)) :
            settings.current.x;
            *py = (block.y_flag == ON) ? (block.y_number -
                (settings.origin_offset.y + settings.axis_offset.y)) :
            settings.current.y;
            *pz = (block.z_flag == ON) ? (block.z_number -
                (settings.tool_length_offset + settings.origin_offset.z
                + settings.axis_offset.z)) : settings.current.z;
            *AA_p = (block.a_flag == ON) ? (block.a_number -
                (settings.origin_offset.a + settings.axis_offset.a)) :
            settings.current.a;                 /*AA*/
            *BB_p = (block.b_flag == ON) ? (block.b_number -
                (settings.origin_offset.b + settings.axis_offset.b)) :
            settings.current.b;                 /*BB*/
            *CC_p = (block.c_flag == ON) ? (block.c_number -
                (settings.tool_length_offset + settings.origin_offset.c
                + settings.axis_offset.c)) : settings.current.c;
        }
        else if (mode == MODE_ABSOLUTE)
        {
            *px = (block.x_flag == ON) ? block.x_number     :
            (comp and middle)     ? settings.program_x :
            settings.current.x ;

            *py = (block.y_flag == ON) ? block.y_number     :
            (comp and middle)     ? settings.program_y :
            settings.current.y ;

            *pz = (block.z_flag == ON) ? block.z_number     :
            settings.current.z ;
            *AA_p = (block.a_flag == ON) ? block.a_number     :
            settings.current.a ;                /*AA*/
            *BB_p = (block.b_flag == ON) ? block.b_number     :
            settings.current.b ;                /*BB*/
            *CC_p = (block.c_flag == ON) ? block.c_number     :
            settings.current.c ;                /*CC*/
        }
        else                                      /* mode is MODE_INCREMENTAL */
        {
            *px = (block.x_flag == ON)
                ? ((comp and middle) ? (block.x_number + settings.program_x)
                : (block.x_number + settings.current.x))
                : ((comp and middle) ? settings.program_x
                : settings.current.x);

            *py = (block.y_flag == ON)
                ? ((comp and middle) ? (block.y_number + settings.program_y)
                : (block.y_number + settings.current.y))
                : ((comp and middle) ? settings.program_y
                : settings.current.y);

            *pz = (block.z_flag == ON) ?
                (settings.current.z + block.z_number) : settings.current.z;
            *AA_p = (block.a_flag == ON) ?  /*AA*/
                (settings.current.a + block.a_number) : settings.current.a;
            *BB_p = (block.b_flag == ON) ?  /*BB*/
                (settings.current.b + block.b_number) : settings.current.b;
            *CC_p = (block.c_flag == ON) ?  /*CC*/
                (settings.current.c + block.c_number) : settings.current.c;
        }
    }

   /****************************************************************************/

   /* find_relative

   Returned Value: int (RS274NGC_OK)

   Side effects:
   The values of x2, y2, z2, aa_2, bb_2, and cc_2 are set.
   (NOTE: aa_2 etc. are written with lower case letters in this
   documentation because upper case would confuse the pre-preprocessor.)

   Called by:
   convert_home

   This finds the coordinates in the current system, under the current
   tool length offset, of a point (x1, y1, z1, aa_1, bb_1, cc_1) whose absolute
   coordinates are known.

   Don't confuse this with the inverse operation.

   */

    void rs274ngc::find_relative(                     /* ARGUMENTS                   */
    double x1,                                    /* absolute x position         */
    double y1,                                    /* absolute y position         */
    double z1,                                    /* absolute z position         */
    double AA_1,             /* absolute a position         */ /*AA*/
    double BB_1,             /* absolute b position         */ /*BB*/
    double CC_1,             /* absolute c position         */ /*CC*/
    double * x2,                                  /* pointer to relative x       */
    double * y2,                                  /* pointer to relative y       */
    double * z2,                                  /* pointer to relative z       */
    double * AA_2,           /* pointer to relative a       */ /*AA*/
    double * BB_2,           /* pointer to relative b       */ /*BB*/
    double * CC_2,           /* pointer to relative c       */ /*CC*/
    setup_t& settings)                       /* pointer to machine settings */
    {
        *x2 = (x1 - (settings.origin_offset.x + settings.axis_offset.x));
        *y2 = (y1 - (settings.origin_offset.y + settings.axis_offset.y));
        *z2 = (z1 - (settings.tool_length_offset +
            settings.origin_offset.z + settings.axis_offset.z));
        *AA_2 = (AA_1 - (settings.origin_offset.a +
            settings.axis_offset.a));           /*AA*/
        *BB_2 = (BB_1 - (settings.origin_offset.b +
            settings.axis_offset.b));           /*BB*/
        *CC_2 = (CC_1 - (settings.origin_offset.c +
            settings.axis_offset.c));           /*CC*/
    }

   /****************************************************************************/

   /* find_straight_length

   Returned Value: double (length of path between start and end points)

   Side effects: none

   Called by:
   inverse_time_rate_straight
   inverse_time_rate_as

   This calculates a number to use in feed rate calculations when inverse
   time feed mode is used, for a motion in which X,Y,Z,A,B, and C each change
   linearly or not at all from their initial value to their end value.

   This is used when the feed_reference mode is CANON_XYZ, which is
   always in rs274NGC.

   If any of the X, Y, or Z axes move or the A-axis, B-axis, and C-axis
   do not move, this is the length of the path relative to the XYZ axes
   from the first point to the second, and any rotary axis motion is
   ignored. The length is the simple Euclidean distance.

   The formula for the Euclidean distance "length" of a move involving
   only the A, B and C axes is based on a conversation with Jim Frohardt at
   Boeing, who says that the Fanuc controller on their 5-axis machine
   interprets the feed rate this way. Note that if only one rotary axis
   moves, this formula returns the absolute value of that axis move,
   which is what is desired.

   */

    double rs274ngc::find_straight_length(           /* ARGUMENTS   */
    double x2,                                    /* X-coordinate of end point    */
    double y2,                                    /* Y-coordinate of end point    */
    double z2,                                    /* Z-coordinate of end point    */
    double AA_2,      /* A-coordinate of end point    */ /*AA*/
    double BB_2,      /* B-coordinate of end point    */ /*BB*/
    double CC_2,      /* C-coordinate of end point    */ /*CC*/
    double x1,                                    /* X-coordinate of start point  */
    double y1,                                    /* Y-coordinate of start point  */
    double z1                                     /* Z-coordinate of start point  */
    , double AA_1     /* A-coordinate of start point  */ /*AA*/
    , double BB_1     /* B-coordinate of start point  */ /*BB*/
    , double CC_1     /* C-coordinate of start point  */ /*CC*/
    )
    {
        if ((x1 != x2) or (y1 != y2) or (z1 != z2) or
            (1
            and (AA_2 == AA_1)                    /*AA*/
            and (BB_2 == BB_1)                    /*BB*/
            and (CC_2 == CC_1)                    /*CC*/
            ))                                    /* straight line */
            return sqrt(pow((x2 - x1),2) + pow((y2 - y1),2) + pow((z2 - z1),2));
        else
            return sqrt(0 +
                pow((AA_2 - AA_1), 2) +           /*AA*/
                pow((BB_2 - BB_1), 2) +           /*BB*/
                pow((CC_2 - CC_1), 2) +           /*CC*/
                0);
    }

   /****************************************************************************/

   /* find_turn

   Returned Value: double (angle in radians between two radii of a circle)

   Side effects: none

   Called by: find_arc_length

   All angles are in radians.

   */

    double rs274ngc::find_turn(                      /* ARGUMENTS                          */
    double x1,                                    /* X-coordinate of start point        */
    double y1,                                    /* Y-coordinate of start point        */
    double center_x,                              /* X-coordinate of arc center         */
    double center_y,                              /* Y-coordinate of arc center         */
    int turn,                                     /* no. of full or partial circles CCW */
    double x2,                                    /* X-coordinate of end point          */
    double y2)                                    /* Y-coordinate of end point          */
    {
        double alpha;                             /* angle of first radius                      */
        double beta;                              /* angle of second radius                     */
        double theta;                             /* amount of turn of arc CCW - negative if CW */

        if (turn == 0)
            return 0.0;
        alpha = atan2((y1 - center_y), (x1 - center_x));
        beta = atan2((y2 - center_y), (x2 - center_x));
        if (turn > 0)
        {
            if (beta <= alpha)
                beta = (beta + TWO_PI);
            theta = ((beta - alpha) + ((turn - 1) * TWO_PI));
        }
        else                                      /* turn < 0 */
        {
            if (alpha <= beta)
                alpha = (alpha + TWO_PI);
            theta = ((beta - alpha) + ((turn + 1) * TWO_PI));
        }
        return (theta);
    }

   /****************************************************************************/

   /* init_block

   Returned Value: int (RS274NGC_OK)

   Side effects:
   Values in the block are reset as described below.

   Called by: parse_line

   This system reuses the same block over and over, rather than building
   a new one for each line of NC code. The block is re-initialized before
   each new line of NC code is read.

   The block contains many slots for values which may or may not be present
   on a line of NC code. For some of these slots, there is a flag which
   is turned on (at the time time value of the slot is read) if the item
   is present.  For slots whose values are to be read which do not have a
   flag, there is always some excluded range of values. Setting the
   initial value of these slot to some number in the excluded range
   serves to show that a value for that slot has not been read.

   The rules for the indicators for slots whose values may be read are:
   1. If the value may be an arbitrary real number (which is always stored
   internally as a double), a flag is needed to indicate if a value has
   been read. All such flags are initialized to OFF.
   Note that the value itself is not initialized; there is no point in it.
   2. If the value must be a non-negative real number (which is always stored
   internally as a double), a value of -1.0 indicates the item is not present.
   3. If the value must be an unsigned integer (which is always stored
   internally as an int), a value of -1 indicates the item is not present.
   (RS274/NGC does not use any negative integers.)
   4. If the value is a character string (only the comment slot is one), the
   first character is set to 0 (NULL).

   */

    void rs274ngc::init_block(                        /* ARGUMENTS                                     */
    block_t& block)                          /* pointer to a block to be initialized or reset */
    {
        block.a_flag = OFF;                 /*AA*/
        block.b_flag = OFF;                 /*BB*/
        block.c_flag = OFF;                 /*CC*/
        block.comment[0] = 0;
        block.d_number = -1;
        block.f_number = -1.0;
        for (int n = 0; n < 14; n++)
        {
            block.g_modes[n] = -1;
        }
        block.h_number = -1;
        block.i_flag = OFF;
        block.j_flag = OFF;
        block.k_flag = OFF;
        block.l_number = -1;
        block.line_number = -1;
        block.motion_to_be = -1;
        block.m_count = 0;
        for (int n = 0; n < 10; n++)
        {
            block.m_modes[n] = -1;
        }
        block.p_number = -1.0;
        block.q_number = -1.0;
        block.r_flag = OFF;
        block.s_number = -1.0;
        block.t_number = -1;
        block.x_flag = OFF;
        block.y_flag = OFF;
        block.z_flag = OFF;
        block.parameter_occurrence = 0;     /* initialize parameter buffer */
    }

   /****************************************************************************/

   /* inverse_time_rate_arc

   Returned Value: int (RS274NGC_OK)

   Side effects: a call is made to SET_FEED_RATE and _setup.feed_rate is set.

   Called by:
   convert_arc2
   convert_arc_comp1
   convert_arc_comp2

   This finds the feed rate needed by an inverse time move. The move
   consists of an a single arc. Most of the work here is in finding the
   length of the arc.

   */

    void rs274ngc::inverse_time_rate_arc(             /* ARGUMENTS                       */
    double x1,                                    /* x coord of start point of arc            */
    double y1,                                    /* y coord of start point of arc            */
    double z1,                                    /* z coord of start point of arc            */
    double cx,                                    /* x coord of center of arc                 */
    double cy,                                    /* y coord of center of arc                 */
    int turn,                                     /* turn of arc                              */
    double x2,                                    /* x coord of end point of arc              */
    double y2,                                    /* y coord of end point of arc              */
    double z2,                                    /* z coord of end point of arc              */
    block_t& block,                          /* pointer to a block of RS274 instructions */
    setup_t& settings)                       /* pointer to machine settings              */
    {
        double length;
        double rate;

        length = find_arc_length (x1, y1, z1, cx, cy, turn, x2, y2, z2);
        rate = std::max(0.1, (length * block.f_number));
        SET_FEED_RATE (rate);
        settings.feed_rate = rate;
    }

   /****************************************************************************/

   /* inverse_time_rate_arc2

   Returned Value: int (RS274NGC_OK)

   Side effects: a call is made to SET_FEED_RATE and _setup.feed_rate is set.

   Called by: convert_arc_comp2

   This finds the feed rate needed by an inverse time move in
   convert_arc_comp2. The move consists of an extra arc and a main
   arc. Most of the work here is in finding the lengths of the two arcs.

   All rotary motion is assumed to occur on the extra arc, as done by
   convert_arc_comp2.

   All z motion is assumed to occur on the main arc, as done by
   convert_arc_comp2.

   */

    void rs274ngc::inverse_time_rate_arc2(            /* ARGUMENTS */
    double start_x,                               /* x coord of last program point, extra arc center x */
    double start_y,                               /* y coord of last program point, extra arc center y */
    int turn1,                                    /* turn of extra arc                                 */
    double mid_x,                                 /* x coord of end point of extra arc                 */
    double mid_y,                                 /* y coord of end point of extra arc                 */
    double cx,                                    /* x coord of center of main arc                     */
    double cy,                                    /* y coord of center of main arc                     */
    int turn2,                                    /* turn of main arc                                  */
    double end_x,                                 /* x coord of end point of main arc                  */
    double end_y,                                 /* y coord of end point of main arc                  */
    double end_z,                                 /* z coord of end point of main arc                  */
    block_t& block,                          /* pointer to a block of RS274 instructions          */
    setup_t& settings)                       /* pointer to machine settings                       */
    {
        double length;
        double rate;

        length = (find_arc_length (settings.current.x, settings.current.y,
            settings.current.z, start_x, start_y,
            turn1, mid_x, mid_y, settings.current.z) +
            find_arc_length(mid_x, mid_y, settings.current.z,
            cx, cy, turn2, end_x, end_y, end_z));
        rate = std::max(0.1, (length * block.f_number));
        SET_FEED_RATE (rate);
        settings.feed_rate = rate;
    }

   /****************************************************************************/

   /* inverse_time_rate_as

   Returned Value: int (RS274NGC_OK)

   Side effects: a call is made to SET_FEED_RATE and _setup.feed_rate is set.

   Called by: convert_straight_comp2

   This finds the feed rate needed by an inverse time move in
   convert_straight_comp2. The move consists of an extra arc and a straight
   line. Most of the work here is in finding the lengths of the arc and
   the line.

   All rotary motion is assumed to occur on the arc, as done by
   convert_straight_comp2.

   All z motion is assumed to occur on the line, as done by
   convert_straight_comp2.

   */

    void rs274ngc::inverse_time_rate_as(              /* ARGUMENTS */
    double start_x,                               /* x coord of last program point, extra arc center x */
    double start_y,                               /* y coord of last program point, extra arc center y */
    int turn,                                     /* turn of extra arc                                 */
    double mid_x,                                 /* x coord of end point of extra arc                 */
    double mid_y,                                 /* y coord of end point of extra arc                 */
    double end_x,                                 /* x coord of end point of straight line             */
    double end_y,                                 /* y coord of end point of straight line             */
    double end_z,                                 /* z coord of end point of straight line             */
    double AA_end,                                /* A coord of end point of straight line       *//*AA*/
    double BB_end,                                /* B coord of end point of straight line       *//*BB*/
    double CC_end,                                /* C coord of end point of straight line       *//*CC*/
    block_t& block,                          /* pointer to a block of RS274 instructions          */
    setup_t& settings)                       /* pointer to machine settings                       */
    {
        double length;
        double rate;

        length = (find_arc_length (settings.current.x, settings.current.y,
            settings.current.z, start_x, start_y,
            turn, mid_x, mid_y, settings.current.z) +
            find_straight_length(end_x, end_y,
            end_z
            , AA_end
            , BB_end
            , CC_end
            , mid_x, mid_y,
            settings.current.z
            , AA_end
            , BB_end
            , CC_end
            ));
        rate = std::max(0.1, (length * block.f_number));
        SET_FEED_RATE (rate);
        settings.feed_rate = rate;
    }

   /****************************************************************************/

   /* inverse_time_rate_straight

   Returned Value: int (RS274NGC_OK)

   Side effects: a call is made to SET_FEED_RATE and _setup.feed_rate is set.

   Called by:
   convert_straight
   convert_straight_comp1
   convert_straight_comp2

   This finds the feed rate needed by an inverse time straight move. Most
   of the work here is in finding the length of the line.

   */

    void rs274ngc::inverse_time_rate_straight(        /* ARGUMENTS                    */
    double end_x,                                 /* x coordinate of end point of straight line */
    double end_y,                                 /* y coordinate of end point of straight line */
    double end_z,                                 /* z coordinate of end point of straight line */
    double AA_end,                                /* A coordinate of end point of straight line *//*AA*/
    double BB_end,                                /* B coordinate of end point of straight line *//*BB*/
    double CC_end,                                /* C coordinate of end point of straight line *//*CC*/
    block_t& block,                          /* pointer to a block of RS274 instructions   */
    setup_t& settings)                       /* pointer to machine settings                */
    {
        double length;
        double rate;

        length = find_straight_length
            (end_x, end_y, end_z
            , AA_end
            , BB_end
            , CC_end
            , settings.current.x,
            settings.current.y, settings.current.z

            , settings.current.a
            , settings.current.b
            , settings.current.c
            );
        rate = std::max(0.1, (length * block.f_number));
        SET_FEED_RATE (rate);
        settings.feed_rate = rate;
    }

   /****************************************************************************/

   /* parse_line

   Returned Value: int
   If any of the following functions returns an error code,
   this returns that code.
   init_block
   read_items
   enhance_block
   check_items
   Otherwise, it returns RS274NGC_OK.

   Side effects:
   One RS274 line is read into a block and the block is checked for
   errors. System parameters may be reset.

   Called by:  rs274ngc_read

   */

    void rs274ngc::parse_line(                        /* ARGUMENTS                            */
    const char * line,                                  /* array holding a line of RS274 code   */
    block_t& block,                          /* pointer to a block to be filled      */
    setup_t& settings)                       /* pointer to machine settings          */
    {
        init_block (block);
        read_items(block, line, settings.parameters);
        enhance_block(block, settings);
        check_items (block, settings);
    }

   /****************************************************************************/

   /* precedence

   Returned Value: int
   This returns an integer representing the precedence level of an_operator

   Side Effects: None

   Called by: read_real_expression

   To add additional levels of operator precedence, edit this function.

   */

    int rs274ngc::precedence(                        /* ARGUMENTS  */
    int an_operator)
    {
        if (an_operator == RIGHT_BRACKET)
            return 1;
        else if (an_operator == POWER)
            return 4;
        else if (an_operator >= AND2)
            return 2;
        else
            return 3;
    }

   /****************************************************************************/

   /* read_a

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not a:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. An a_coordinate has already been inserted in the block:
   NCE_MULTIPLE_A_WORDS_ON_ONE_LINE.
   3. A values are not allowed: NCE_CANNOT_USE_A_WORD.

   Side effects:
   counter is reset.
   The a_flag in the block is turned on.
   An a_number is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'a', indicating an a_coordinate
   setting. The function reads characters which tell how to set the
   coordinate, up to the start of the next item or the end of the line.
   The counter is then set to point to the character following.

   The value may be a real number or something that evaluates to a
   real number, so read_real_value is used to read it. Parameters
   may be involved.

   If the AA compiler flag is defined, the a_flag in the block is turned
   on and the a_number in the block is set to the value read. If the
   AA flag is not defined, (i) if the AXIS_ERROR flag is defined, that means
   A values are not allowed, and an error value is returned, (ii) if the
   AXIS_ERROR flag is not defined, nothing is done.

   */

    void rs274ngc::read_a(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        double value;

        error_if((line[*counter] != 'a'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.a_flag != OFF), NCE_MULTIPLE_A_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.a_flag = ON;
        block.a_number = value;
    }

   /****************************************************************************/

   /* read_atan

   Returned Value: int
   If read_real_expression returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character to read is not a slash:
   NCE_SLASH_MISSING_AFTER_FIRST_ATAN_ARGUMENT
   2. The second character to read is not a left bracket:
   NCE_LEFT_BRACKET_MISSING_AFTER_SLASH_WITH_ATAN

   Side effects:
   The computed value is put into what double_ptr points at.
   The counter is reset to point to the first character after the
   characters which make up the value.

   Called by:
   read_unary

   When this function is called, the characters "atan" and the first
   argument have already been read, and the value of the first argument
   is stored in double_ptr.  This function attempts to read a slash and
   the second argument to the atan function, starting at the index given
   by the counter and then to compute the value of the atan operation
   applied to the two arguments.  The computed value is inserted into
   what double_ptr points at.

   The computed value is in the range from -180 degrees to +180 degrees.
   The range is not specified in the RS274/NGC manual [NCMS, page 51],
   although using degrees (not radians) is specified.

   */

    void rs274ngc::read_atan(                         /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on line      */
    double * double_ptr,                          /* pointer to double to be read                   */
    double * parameters) const                          /* array of system parameters                     */
    {
        double argument2;

        error_if((line [*counter] != '/'), NCE_SLASH_MISSING_AFTER_FIRST_ATAN_ARGUMENT);
        *counter = (*counter + 1);
        error_if((line[*counter] != '['), NCE_LEFT_BRACKET_MISSING_AFTER_SLASH_WITH_ATAN);
        read_real_expression (line, counter, &argument2, parameters);
   /* value in radians */
        *double_ptr = atan2(*double_ptr, argument2);
   /* convert to degrees */
        *double_ptr = ((*double_ptr * 180.0)/PI);
    }

   /****************************************************************************/

   /* read_b

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not b:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. A b_coordinate has already been inserted in the block:
   NCE_MULTIPLE_B_WORDS_ON_ONE_LINE.
   3. B values are not allowed: NCE_CANNOT_USE_B_WORD

   Side effects:
   counter is reset.
   The b_flag in the block is turned on.
   A b_number is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'b', indicating a b_coordinate
   setting. The function reads characters which tell how to set the
   coordinate, up to the start of the next item or the end of the line.
   The counter is then set to point to the character following.

   The value may be a real number or something that evaluates to a
   real number, so read_real_value is used to read it. Parameters
   may be involved.

   If the BB compiler flag is defined, the b_flag in the block is turned
   on and the b_number in the block is set to the value read. If the
   BB flag is not defined, (i) if the AXIS_ERROR flag is defined, that means
   B values are not allowed, and an error value is returned, (ii) if the
   AXIS_ERROR flag is not defined, nothing is done.

   */

    void rs274ngc::read_b(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        double value;

        error_if((line[*counter] != 'b'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.b_flag != OFF), NCE_MULTIPLE_B_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.b_flag = ON;
        block.b_number = value;
    }

   /****************************************************************************/

   /* read_c

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not c:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. An c_coordinate has already been inserted in the block:
   NCE_MULTIPLE_C_WORDS_ON_ONE_LINE
   3. C values are not allowed: NCE_CANNOT_USE_C_WORD

   Side effects:
   counter is reset.
   The c_flag in the block is turned on.
   A c_number is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'c', indicating an c_coordinate
   setting. The function reads characters which tell how to set the
   coordinate, up to the start of the next item or the end of the line.
   The counter is then set to point to the character following.

   The value may be a real number or something that evaluates to a
   real number, so read_real_value is used to read it. Parameters
   may be involved.

   If the CC compiler flag is defined, the c_flag in the block is turned
   on and the c_number in the block is set to the value read. If the
   CC flag is not defined, (i) if the AXIS_ERROR flag is defined, that means
   C values are not allowed, and an error value is returned, (ii) if the
   AXIS_ERROR flag is not defined, nothing is done.

   */

    void rs274ngc::read_c(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        double value;

        error_if((line[*counter] != 'c'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.c_flag != OFF), NCE_MULTIPLE_C_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.c_flag = ON;
        block.c_number = value;
    }

   /****************************************************************************/

   /* read_comment

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not '(' ,
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED

   Side effects:
   The counter is reset to point to the character following the comment.
   The comment string, without parentheses, is copied into the comment
   area of the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character '(', indicating a comment is
   beginning. The function reads characters of the comment, up to and
   including the comment closer ')'.

   It is expected that the format of a comment will have been checked (by
   read_text or read_keyboard_line) and bad format comments will
   have prevented the system from getting this far, so that this function
   can assume a close parenthesis will be found when an open parenthesis
   has been found, and that comments are not nested.

   The "parameters" argument is not used in this function. That argument is
   present only so that this will have the same argument list as the other
   "read_XXX" functions called using a function pointer by read_one_item.

   */

    void rs274ngc::read_comment(                      /* ARGUMENTS                                     */
    const char * line,                                  /* string: line of RS274 code being processed    */
    int * counter,                                /* pointer to a counter for position on the line */
    block_t& block,                          /* pointer to a block being filled from the line */
    double * /*parameters*/) const                          /* array of system parameters                    */
    {
        int n;

        error_if((line[*counter] != '('), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        (*counter)++;
        for (n = 0; line[*counter] != ')' ; (*counter)++, n++)
        {
            block.comment[n] = line[*counter];
        }
        block.comment[n] = 0;
        (*counter)++;
    }

   /****************************************************************************/

   /* read_d

   Returned Value: int
   If read_integer_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not d:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. A d_number has already been inserted in the block:
   NCE_MULTIPLE_D_WORDS_ON_ONE_LINE
   3. The d_number is negative: NCE_NEGATIVE_D_WORD_TOOL_RADIUS_INDEX_USED
   4. The d_number is more than _setup.tool_max: NCE_TOOL_RADIUS_INDEX_TOO_BIG

   Side effects:
   counter is reset to the character following the tool number.
   A d_number is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'd', indicating an index into a
   table of tool diameters.  The function reads characters which give the
   (integer) value of the index. The value may not be more than
   _setup.tool_max and may not be negative, but it may be zero. The range
   is checked here.

   read_integer_value allows a minus sign, so a check for a negative value
   is made here, and the parameters argument is also needed.

   */

    void rs274ngc::read_d(                            /* ARGUMENTS                                     */
    const char * line,                                  /* string: line of RS274 code being processed    */
    int * counter,                                /* pointer to a counter for position on the line */
    block_t& block,                          /* pointer to a block being filled from the line */
    double * parameters) const                          /* array of system parameters                    */
    {
        int value;

        error_if((line[*counter] != 'd'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.d_number > -1), NCE_MULTIPLE_D_WORDS_ON_ONE_LINE);
        read_integer_value(line, counter, &value, parameters);
        error_if((value < 0), NCE_NEGATIVE_D_WORD_TOOL_RADIUS_INDEX_USED);
        error_if((value > _setup.tool_max), NCE_TOOL_RADIUS_INDEX_TOO_BIG);
        block.d_number = value;
    }

   /****************************************************************************/

   /* read_f

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not f:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. An f_number has already been inserted in the block:
   NCE_MULTIPLE_F_WORDS_ON_ONE_LINE
   3. The f_number is negative: NCE_NEGATIVE_F_WORD_USED

   Side effects:
   counter is reset to point to the first character following the f_number.
   The f_number is inserted in block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'f'. The function reads characters
   which tell how to set the f_number, up to the start of the next item
   or the end of the line. This information is inserted in the block.

   The value may be a real number or something that evaluates to a real
   number, so read_real_value is used to read it. Parameters may be
   involved, so the parameters argument is required. The value is always
   a feed rate.

   */

    void rs274ngc::read_f(                            /* ARGUMENTS                                     */
    const char * line,                                  /* string: line of RS274 code being processed    */
    int * counter,                                /* pointer to a counter for position on the line */
    block_t& block,                          /* pointer to a block being filled from the line */
    double * parameters) const                          /* array of system parameters                    */
    {
        double value;

        error_if((line[*counter] != 'f'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.f_number > -1.0), NCE_MULTIPLE_F_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        error_if((value < 0.0), NCE_NEGATIVE_F_WORD_USED);
        block.f_number = value;
    }

   /****************************************************************************/

   /* read_g

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not g:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. The value is negative: NCE_NEGATIVE_G_CODE_USED
   3. The value differs from a number ending in an even tenth by more
   than 0.0001: NCE_G_CODE_OUT_OF_RANGE
   4. The value is greater than 99.9: NCE_G_CODE_OUT_OF_RANGE
   5. The value is not the number of a valid g code: NCE_UNKNOWN_G_CODE_USED
   6. Another g code from the same modal group has already been
   inserted in the block: NCE_TWO_G_CODES_USED_FROM_SAME_MODAL_GROUP

   Side effects:
   counter is reset to the character following the end of the g_code.
   A g code is inserted as the value of the appropriate mode in the
   g_modes array in the block.
   The g code counter in the block is increased by 1.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'g', indicating a g_code.  The
   function reads characters which tell how to set the g_code.

   The RS274/NGC manual [NCMS, page 51] allows g_codes to be represented
   by expressions and provide [NCMS, 71 - 73] that a g_code must evaluate
   to to a number of the form XX.X (59.1, for example). The manual does not
   say how close an expression must come to one of the allowed values for
   it to be legitimate. Is 59.099999 allowed to mean 59.1, for example?
   In the interpreter, we adopt the convention that the evaluated number
   for the g_code must be within 0.0001 of a value of the form XX.X

   To simplify the handling of g_codes, we convert them to integers by
   multiplying by 10 and rounding down or up if within 0.001 of an
   integer. Other functions that deal with g_codes handle them
   symbolically, however. The symbols are defined in rs274NGC.hh
   where G_1 is 10, G_83 is 830, etc.

   This allows any number of g_codes on one line, provided that no two
   are in the same modal group.

   */

    void rs274ngc::read_g(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        double value_read;
        int value;
        int mode;

        error_if((line[*counter] != 'g'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        read_real_value(line, counter, &value_read, parameters);
        value_read = (10.0 * value_read);
        value = (int)floor(value_read);

        if ((value_read - value) > 0.999)
            value = (int)ceil(value_read);
        else if ((value_read - value) > 0.001)
            throw error(NCE_G_CODE_OUT_OF_RANGE);

        error_if((value > 999), NCE_G_CODE_OUT_OF_RANGE);
        error_if((value < 0), NCE_NEGATIVE_G_CODE_USED);
        mode = _gees[value];
        error_if((mode == -1), NCE_UNKNOWN_G_CODE_USED);
        error_if((block.g_modes[mode] != -1), NCE_TWO_G_CODES_USED_FROM_SAME_MODAL_GROUP);
        block.g_modes[mode] = value;
    }

   /****************************************************************************/

   /* read_h

   Returned Value: int
   If read_integer_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not h:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. An h_number has already been inserted in the block:
   NCE_MULTIPLE_H_WORDS_ON_ONE_LINE
   3. The value is negative: NCE_NEGATIVE_H_WORD_TOOL_LENGTH_OFFSET_INDEX_USED
   4. The value is greater than _setup.tool_max:
   NCE_TOOL_LENGTH_OFFSET_INDEX_TOO_BIG

   Side effects:
   counter is reset to the character following the h_number.
   An h_number is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'h', indicating a tool length
   offset index.  The function reads characters which give the (integer)
   value of the tool length offset index (not the actual distance of the
   offset).

   */

    void rs274ngc::read_h(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        int value;

        error_if((line[*counter] != 'h'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.h_number > -1), NCE_MULTIPLE_H_WORDS_ON_ONE_LINE);
        read_integer_value(line, counter, &value, parameters);
        error_if((value < 0), NCE_NEGATIVE_H_WORD_TOOL_LENGTH_OFFSET_INDEX_USED);
        error_if((value > _setup.tool_max), NCE_TOOL_LENGTH_OFFSET_INDEX_TOO_BIG);
        block.h_number = value;
    }

   /****************************************************************************/

   /* read_i

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not i:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. An i_coordinate has already been inserted in the block:
   NCE_MULTIPLE_I_WORDS_ON_ONE_LINE

   Side effects:
   counter is reset.
   The i_flag in the block is turned on.
   An i_coordinate setting is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'i', indicating a i_coordinate
   setting. The function reads characters which tell how to set the
   coordinate, up to the start of the next item or the end of the line.
   This information is inserted in the block. The counter is then set to
   point to the character following.

   The value may be a real number or something that evaluates to a
   real number, so read_real_value is used to read it. Parameters
   may be involved.

   */

    void rs274ngc::read_i(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274 code being processed     */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        double value;

        error_if((line[*counter] != 'i'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.i_flag != OFF), NCE_MULTIPLE_I_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.i_flag = ON;
        block.i_number = value;
    }

   /****************************************************************************/

   /* read_integer_unsigned

   Returned Value: int
   If any of the following errors occur, this returns the error shown.
   Otherwise, RS274NGC_OK is returned.
   1. The first character is not a digit: NCE_BAD_FORMAT_UNSIGNED_INTEGER
   2. sscanf fails: NCE_SSCANF_FAILED

   Side effects:
   The number read from the line is put into what integer_ptr points at.

   Called by: read_line_number

   This reads an explicit unsigned (positive) integer from a string,
   starting from the position given by *counter. It expects to find one
   or more digits. Any character other than a digit terminates reading
   the integer. Note that if the first character is a sign (+ or -),
   an error will be reported (since a sign is not a digit).

   */

    void rs274ngc::read_integer_unsigned(             /* ARGUMENTS                       */
    const char * line,                                  /* string: line of RS274 code being processed    */
    int * counter,                                /* pointer to a counter for position on the line */
    int * integer_ptr) const                            /* pointer to the value being read               */
    {
        int n;
        char c;

        for (n = *counter; ; n++)
        {
            c = line[n];
            if ((c < 48) or (c > 57))
                break;
        }
        error_if((n == *counter), NCE_BAD_FORMAT_UNSIGNED_INTEGER);
        if (sscanf(line + *counter, "%d", integer_ptr) == 0)
            throw error(NCE_SSCANF_FAILED);
        *counter = n;
    }

   /****************************************************************************/

   /* read_integer_value

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The returned value is not close to an integer:
   NCE_NON_INTEGER_VALUE_FOR_INTEGER

   Side effects:
   The number read from the line is put into what integer_ptr points at.

   Called by:
   read_d
   read_l
   read_h
   read_m
   read_parameter
   read_parameter_setting
   read_t

   This reads an integer (positive, negative or zero) from a string,
   starting from the position given by *counter. The value being
   read may be written with a decimal point or it may be an expression
   involving non-integers, as long as the result comes out within 0.0001
   of an integer.

   This proceeds by calling read_real_value and checking that it is
   close to an integer, then returning the integer it is close to.

   */

    void rs274ngc::read_integer_value(                /* ARGUMENTS                                 */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    int * integer_ptr,                            /* pointer to the value being read                */
    double * parameters) const                          /* array of system parameters                     */
    {
        double float_value;

        read_real_value(line, counter, &float_value, parameters);
        *integer_ptr = (int)floor(float_value);
        if ((float_value - *integer_ptr) > 0.9999)
        {
            *integer_ptr = (int)ceil(float_value);
        }
        else if ((float_value - *integer_ptr) > 0.0001)
            throw error(NCE_NON_INTEGER_VALUE_FOR_INTEGER);
    }

   /****************************************************************************/

   /* read_items

   Returned Value: int
   If read_line_number or read_one_item returns an error code,
   this returns that code.
   Otherwise, it returns RS274NGC_OK.

   Side effects:
   One line of RS274 code is read and data inserted into a block.
   The counter which is passed around among the readers is initialized.
   System parameters may be reset.

   Called by: parse_line

   */

    void rs274ngc::read_items(                        /* ARGUMENTS                                      */
    block_t& block,                          /* pointer to a block being filled from the line  */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    double * parameters) const                          /* array of system parameters                     */
    {
        int counter;
        int length;

        length = strlen(line);
        counter = 0;

        if (line[counter] == '/')                 /* skip the slash character if first */
            counter++;
        if (line[counter] == 'n')
        {
            read_line_number(line, &counter, block);
        }
        for ( ; counter < length; )
        {
            read_one_item (line, &counter, block, parameters);
        }
    }

   /****************************************************************************/

   /* read_j

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not j:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. A j_coordinate has already been inserted in the block.
   NCE_MULTIPLE_J_WORDS_ON_ONE_LINE

   Side effects:
   counter is reset.
   The j_flag in the block is turned on.
   A j_coordinate setting is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'j', indicating a j_coordinate
   setting. The function reads characters which tell how to set the
   coordinate, up to the start of the next item or the end of the line.
   This information is inserted in the block. The counter is then set to
   point to the character following.

   The value may be a real number or something that evaluates to a real
   number, so read_real_value is used to read it. Parameters may be
   involved.

   */

    void rs274ngc::read_j(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274 code being processed     */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        double value;

        error_if((line[*counter] != 'j'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.j_flag != OFF), NCE_MULTIPLE_J_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.j_flag = ON;
        block.j_number = value;
    }

   /****************************************************************************/

   /* read_k

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not k:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. A k_coordinate has already been inserted in the block:
   NCE_MULTIPLE_K_WORDS_ON_ONE_LINE

   Side effects:
   counter is reset.
   The k_flag in the block is turned on.
   A k_coordinate setting is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'k', indicating a k_coordinate
   setting. The function reads characters which tell how to set the
   coordinate, up to the start of the next item or the end of the line.
   This information is inserted in the block. The counter is then set to
   point to the character following.

   The value may be a real number or something that evaluates to a real
   number, so read_real_value is used to read it. Parameters may be
   involved.

   */

    void rs274ngc::read_k(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274 code being processed     */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        double value;

        error_if((line[*counter] != 'k'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.k_flag != OFF), NCE_MULTIPLE_K_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.k_flag = ON;
        block.k_number = value;
    }

   /****************************************************************************/

   /* read_l

   Returned Value: int
   If read_integer_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not l:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. An l_number has already been inserted in the block:
   NCE_MULTIPLE_L_WORDS_ON_ONE_LINE
   3. the l_number is negative: NCE_NEGATIVE_L_WORD_USED

   Side effects:
   counter is reset to the character following the l number.
   An l code is inserted in the block as the value of l.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'l', indicating an L code.
   The function reads characters which give the (integer) value of the
   L code.

   L codes are used for:
   1. the number of times a canned cycle should be repeated.
   2. a key with G10.

   */

    void rs274ngc::read_l(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        int value;

        error_if((line[*counter] != 'l'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.l_number > -1), NCE_MULTIPLE_L_WORDS_ON_ONE_LINE);
        read_integer_value(line, counter, &value, parameters);
        error_if((value < 0), NCE_NEGATIVE_L_WORD_USED);
        block.l_number = value;
    }

   /****************************************************************************/

   /* read_line_number

   Returned Value: int
   If read_integer_unsigned returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not n:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. The line number is too large (more than 99999):
   NCE_LINE_NUMBER_GREATER_THAN_99999

   Side effects:
   counter is reset to the character following the line number.
   A line number is inserted in the block.

   Called by: read_items

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'n', indicating a line number.
   The function reads characters which give the (integer) value of the
   line number.

   Note that extra initial zeros in a line number will not cause the
   line number to be too large.

   */

    void rs274ngc::read_line_number(                  /* ARGUMENTS                               */
    const char * line,                                  /* string: line of RS274    code being processed  */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block) const                          /* pointer to a block being filled from the line  */
    {
        int value;

        error_if((line[*counter] != 'n'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        read_integer_unsigned(line, counter, &value);
        error_if((value > 99999), NCE_LINE_NUMBER_GREATER_THAN_99999);
        block.line_number = value;
    }

   /****************************************************************************/

   /* read_m

   Returned Value:
   If read_integer_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not m:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. The value is negative: NCE_NEGATIVE_M_CODE_USED
   3. The value is greater than 99: NCE_M_CODE_GREATER_THAN_99
   4. The m code is not known to the system: NCE_UNKNOWN_M_CODE_USED
   5. Another m code in the same modal group has already been read:
   NCE_TWO_M_CODES_USED_FROM_SAME_MODAL_GROUP

   Side effects:
   counter is reset to the character following the m number.
   An m code is inserted as the value of the appropriate mode in the
   m_modes array in the block.
   The m code counter in the block is increased by 1.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'm', indicating an m code.
   The function reads characters which give the (integer) value of the
   m code.

   read_integer_value allows a minus sign, so a check for a negative value
   is needed here, and the parameters argument is also needed.

   */

    void rs274ngc::read_m(                            /* ARGUMENTS                                     */
    const char * line,                                  /* string: line of RS274 code being processed    */
    int * counter,                                /* pointer to a counter for position on the line */
    block_t& block,                          /* pointer to a block being filled from the line */
    double * parameters) const                          /* array of system parameters                    */
    {
        int value;
        int mode;

        error_if((line[*counter] != 'm'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        read_integer_value(line, counter, &value, parameters);
        error_if((value < 0), NCE_NEGATIVE_M_CODE_USED);
        error_if((value > 99), NCE_M_CODE_GREATER_THAN_99);
        mode = _ems[value];
        error_if((mode == -1), NCE_UNKNOWN_M_CODE_USED);
        error_if((block.m_modes[mode] != -1), NCE_TWO_M_CODES_USED_FROM_SAME_MODAL_GROUP);
        block.m_modes[mode] = value;
        block.m_count++;
    }

   /****************************************************************************/

   /* read_one_item

   Returned Value: int
   If a reader function which is called returns an error code, that
   error code is returned.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. the first character read is not a known character for starting a
   word: NCE_BAD_CHARACTER_USED

   Side effects:
   This function reads one item from a line of RS274/NGC code and inserts
   the information in a block. System parameters may be reset.

   Called by: read_items.

   When this function is called, the counter is set so that the position
   being considered is the first position of a word. The character at
   that position must be one known to the system.  In this version those
   characters are: a,b,c,d,f,g,h,i,j,k,l,m,n,p,q,r,s,t,x,y,z,(,#.
   However, read_items calls read_line_number directly if the first word
   begins with n, so no read function is included in the "_readers" array
   for the letter n. Thus, if an n word is encountered in the middle of
   a line, this function reports NCE_BAD_CHARACTER_USED.

   The function looks for a letter or special character and calls a
   selected function according to what the letter or character is.  The
   selected function will be responsible to consider all the characters
   that comprise the remainder of the item, and reset the pointer so that
   it points to the next character after the end of the item (which may be
   the end of the line or the first character of another item).

   After an item is read, the counter is set at the index of the
   next unread character. The item data is stored in the block.

   It is expected that the format of a comment will have been checked;
   this is being done by close_and_downcase. Bad format comments will
   have prevented the system from getting this far, so that this function
   can assume a close parenthesis will be found when an open parenthesis
   has been found, and that comments are not nested.

   */

    void rs274ngc::read_one_item(                     /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        read_function_pointer function_pointer;
        char letter;

        letter = line[*counter];             /* check if in array range */
        error_if(((letter < 0) or (letter > 'z')), NCE_BAD_CHARACTER_USED);
        function_pointer = _readers[static_cast<unsigned int>(letter)];
        error_if((function_pointer == 0), NCE_BAD_CHARACTER_USED);
        (this->*function_pointer)(line, counter, block, parameters);
    }

   /****************************************************************************/

   /* read_operation

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The operation is unknown:
   NCE_UNKNOWN_OPERATION_NAME_STARTING_WITH_A
   NCE_UNKNOWN_OPERATION_NAME_STARTING_WITH_M
   NCE_UNKNOWN_OPERATION_NAME_STARTING_WITH_O
   NCE_UNKNOWN_OPERATION_NAME_STARTING_WITH_X
   NCE_UNKNOWN_OPERATION
   2. The line ends without closing the expression: NCE_UNCLOSED_EXPRESSION

   Side effects:
   An integer representing the operation is put into what operation points
   at.  The counter is reset to point to the first character after the
   operation.

   Called by: read_real_expression

   This expects to be reading a binary operation (+, -, /, *, **, and,
   mod, or, xor) or a right bracket (]). If one of these is found, the
   value of operation is set to the symbolic value for that operation.
   If not, an error is reported as described above.

   */

    void rs274ngc::read_operation(                    /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    int * operation) const                              /* pointer to operation to be read                */
    {
        char c;

        c = line[*counter];
        *counter = (*counter + 1);
        switch(c)
        {
            case '+':
                *operation = PLUS;
                break;
            case '-':
                *operation = MINUS;
                break;
            case '/':
                *operation = DIVIDED_BY;
                break;
            case '*':
                if(line[*counter] == '*')
                {
                    *operation = POWER;
                    *counter = (*counter + 1);
                }
                else
                    *operation = TIMES;
                break;
            case ']':
                *operation = RIGHT_BRACKET;
                break;
            case 'a':
                if((line[*counter] == 'n') and (line[(*counter)+1] == 'd'))
                {
                    *operation = AND2;
                    *counter = (*counter + 2);
                }
                else
                    throw error(NCE_UNKNOWN_OPERATION_NAME_STARTING_WITH_A);
                break;
            case 'm':
                if((line[*counter] == 'o') and (line[(*counter)+1] == 'd'))
                {
                    *operation = MODULO;
                    *counter = (*counter + 2);
                }
                else
                    throw error(NCE_UNKNOWN_OPERATION_NAME_STARTING_WITH_M);
                break;
            case 'o':
                if(line[*counter] == 'r')
                {
                    *operation = NON_EXCLUSIVE_OR;
                    *counter = (*counter + 1);
                }
                else
                    throw error(NCE_UNKNOWN_OPERATION_NAME_STARTING_WITH_O);
                break;
            case 'x':
                if((line[*counter] == 'o') and (line[(*counter)+1] == 'r'))
                {
                    *operation = EXCLUSIVE_OR;
                    *counter = (*counter + 2);
                }
                else
                    throw error(NCE_UNKNOWN_OPERATION_NAME_STARTING_WITH_X);
                break;
            case 0:
                throw error(NCE_UNCLOSED_EXPRESSION);
            default:
                throw error(NCE_UNKNOWN_OPERATION);
        }
    }

   /****************************************************************************/

   /* read_operation_unary

   Returned Value: int
   If the operation is not a known unary operation, this returns one of
   the following error codes:
   NCE_UNKNOWN_WORD_STARTING_WITH_A
   NCE_UNKNOWN_WORD_STARTING_WITH_C
   NCE_UNKNOWN_WORD_STARTING_WITH_E
   NCE_UNKNOWN_WORD_STARTING_WITH_F
   NCE_UNKNOWN_WORD_STARTING_WITH_L
   NCE_UNKNOWN_WORD_STARTING_WITH_R
   NCE_UNKNOWN_WORD_STARTING_WITH_S
   NCE_UNKNOWN_WORD_STARTING_WITH_T
   NCE_UNKNOWN_WORD_WHERE_UNARY_OPERATION_COULD_BE
   Otherwise, this returns RS274NGC_OK.

   Side effects:
   An integer code for the name of the operation read from the
   line is put into what operation points at.
   The counter is reset to point to the first character after the
   characters which make up the operation name.

   Called by:
   read_unary

   This attempts to read the name of a unary operation out of the line,
   starting at the index given by the counter. Known operations are:
   abs, acos, asin, atan, cos, exp, fix, fup, ln, round, sin, sqrt, tan.

   */

    void rs274ngc::read_operation_unary(              /* ARGUMENTS                               */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    int * operation) const                              /* pointer to operation to be read                */
    {
        char c;

        c = line[*counter];
        *counter = (*counter + 1);
        switch (c)
        {
            case 'a':
                if((line[*counter] == 'b') and (line[(*counter)+1] == 's'))
                {
                    *operation = ABS;
                    *counter = (*counter + 2);
                }
                else if(strncmp((line + *counter), "cos", 3) == 0)
                {
                    *operation = ACOS;
                    *counter = (*counter + 3);
                }
                else if(strncmp((line + *counter), "sin", 3) == 0)
                {
                    *operation = ASIN;
                    *counter = (*counter + 3);
                }
                else if(strncmp((line + *counter), "tan", 3) == 0)
                {
                    *operation = ATAN;
                    *counter = (*counter + 3);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_A);
                break;
            case 'c':
                if((line[*counter] == 'o') and (line[(*counter)+1] == 's'))
                {
                    *operation = COS;
                    *counter = (*counter + 2);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_C);
                break;
            case 'e':
                if((line[*counter] == 'x') and (line[(*counter)+1] == 'p'))
                {
                    *operation = EXP;
                    *counter = (*counter + 2);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_E);
                break;
            case 'f':
                if((line[*counter] == 'i') and (line[(*counter)+1] == 'x'))
                {
                    *operation = FIX;
                    *counter = (*counter + 2);
                }
                else if((line[*counter] == 'u') and (line[(*counter)+1] == 'p'))
                {
                    *operation = FUP;
                    *counter = (*counter + 2);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_F);
                break;
            case 'l':
                if(line[*counter] == 'n')
                {
                    *operation = LN;
                    *counter = (*counter + 1);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_L);
                break;
            case 'r':
                if(strncmp((line + *counter), "ound", 4) == 0)
                {
                    *operation = ROUND;
                    *counter = (*counter + 4);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_R);
                break;
            case 's':
                if((line[*counter] == 'i') and (line[(*counter)+1] == 'n'))
                {
                    *operation = SIN;
                    *counter = (*counter + 2);
                }
                else if(strncmp((line + *counter), "qrt", 3) == 0)
                {
                    *operation = SQRT;
                    *counter = (*counter + 3);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_S);
                break;
            case 't':
                if((line[*counter] == 'a') and (line[(*counter)+1] == 'n'))
                {
                    *operation = TAN;
                    *counter = (*counter + 2);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_T);
                break;
            default:
                throw error(NCE_UNKNOWN_WORD_WHERE_UNARY_OPERATION_COULD_BE);
        }
    }

   /****************************************************************************/

   /* read_p

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not p:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. A p value has already been inserted in the block:
   NCE_MULTIPLE_P_WORDS_ON_ONE_LINE
   3. The p value is negative: NCE_NEGATIVE_P_WORD_USED

   Side effects:
   counter is reset to point to the first character following the p value.
   The p value setting is inserted in block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'p', indicating a p value
   setting. The function reads characters which tell how to set the p
   value, up to the start of the next item or the end of the line. This
   information is inserted in the block.

   P codes are used for:
   1. Dwell time in canned cycles g82, G86, G88, G89 [NCMS pages 98 - 100].
   2. A key with G10 [NCMS, pages 9, 10].

   */

    void rs274ngc::read_p(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        double value;

        error_if((line[*counter] != 'p'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.p_number > -1.0), NCE_MULTIPLE_P_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        error_if((value < 0.0), NCE_NEGATIVE_P_WORD_USED);
        block.p_number = value;
    }

   /****************************************************************************/

   /* read_parameter

   Returned Value: int
   If read_integer_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, this returns RS274NGC_OK.
   1. The first character read is not # :
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. The parameter number is out of bounds:
   NCE_PARAMETER_NUMBER_OUT_OF_RANGE

   Side effects:
   The value of the given parameter is put into what double_ptr points at.
   The counter is reset to point to the first character after the
   characters which make up the value.

   Called by:  read_real_value

   This attempts to read the value of a parameter out of the line,
   starting at the index given by the counter.

   According to the RS274/NGC manual [NCMS, p. 62], the characters following
   # may be any "parameter expression". Thus, the following are legal
   and mean the same thing (the value of the parameter whose number is
   stored in parameter 2):
   ##2
   #[#2]

   Parameter setting is done in parallel, not sequentially. For example
   if #1 is 5 before the line "#1=10 #2=#1" is read, then after the line
   is is executed, #1 is 10 and #2 is 5. If parameter setting were done
   sequentially, the value of #2 would be 10 after the line was executed.

   */

    void rs274ngc::read_parameter(                    /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    double * double_ptr,                          /* pointer to double to be read                   */
    double * parameters) const                          /* array of system parameters                     */
    {
        int index;

        error_if((line[*counter] != '#'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        read_integer_value(line, counter, &index, parameters);
        error_if(((index < 1) or (index >= RS274NGC_MAX_PARAMETERS)), NCE_PARAMETER_NUMBER_OUT_OF_RANGE);
        *double_ptr = parameters[index];
    }

   /****************************************************************************/

   /* read_parameter_setting

   Returned Value: int
   If read_real_value or read_integer_value returns an error code,
   this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not # :
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. The parameter index is out of range: PARAMETER_NUMBER_OUT_OF_RANGE
   3. An equal sign does not follow the parameter expression:
   NCE_EQUAL_SIGN_MISSING_IN_PARAMETER_SETTING

   Side effects:
   counter is reset to the character following the end of the parameter
   setting. The parameter whose index follows "#" is set to the
   real value following "=".

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character '#', indicating a parameter
   setting when found by read_one_item.  The function reads characters
   which tell how to set the parameter.

   Any number of parameters may be set on a line. If parameters set early
   on the line are used in expressions farther down the line, the
   parameters have their old values, not their new values. This is
   usually called setting parameters in parallel.

   Parameter setting is not clearly described in [NCMS, pp. 51 - 62]: it is
   not clear if more than one parameter setting per line is allowed (any
   number is OK in this implementation). The characters immediately following
   the "#" must constitute a "parameter expression", but it is not clear
   what that is. Here we allow any expression as long as it evaluates to
   an integer.

   Parameters are handled in the interpreter by having a parameter table
   and a parameter buffer as part of the machine settings. The parameter
   table is passed to the reading functions which need it. The parameter
   buffer is used directly by functions that need it. Reading functions
   may set parameter values in the parameter buffer. Reading functions
   may obtain parameter values; these come from parameter table.

   The parameter buffer has three parts: (i) a counter for how many
   parameters have been set while reading the current line (ii) an array
   of the indexes of parameters that have been set while reading the
   current line, and (iii) an array of the values for the parameters that
   have been set while reading the current line; the nth value
   corresponds to the nth index. Any given index will appear once in the
   index number array for each time the parameter with that index is set
   on a line. There is no point in setting the same parameter more than
   one on a line because only the last setting of that parameter will
   take effect.

   The syntax recognized by this this function is # followed by an
   integer expression (explicit integer or expression evaluating to an
   integer) followed by = followed by a real value (number or
   expression).

   Note that # also starts a bunch of characters which represent a parameter
   to be evaluated. That situation is handled by read_parameter.

   */

    void rs274ngc::read_parameter_setting(            /* ARGUMENTS                        */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        int index;
        double value;

        error_if((line[*counter] != '#'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        read_integer_value(line, counter, &index, parameters);
        error_if(((index < 1) or (index >= RS274NGC_MAX_PARAMETERS)), NCE_PARAMETER_NUMBER_OUT_OF_RANGE);
        error_if((line[*counter] != '='), NCE_EQUAL_SIGN_MISSING_IN_PARAMETER_SETTING);
        *counter = (*counter + 1);
        read_real_value(line, counter, &value, parameters);
        block.parameter_numbers[block.parameter_occurrence] = index;
        block.parameter_values[block.parameter_occurrence] = value;
        block.parameter_occurrence++;
    }

   /****************************************************************************/

   /* read_q

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not q:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. A q value has already been inserted in the block:
   NCE_MULTIPLE_Q_WORDS_ON_ONE_LINE
   3. The q value is negative or zero: NCE_NEGATIVE_OR_ZERO_Q_VALUE_USED

   Side effects:
   counter is reset to point to the first character following the q value.
   The q value setting is inserted in block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'q', indicating a q value
   setting. The function reads characters which tell how to set the q
   value, up to the start of the next item or the end of the line. This
   information is inserted in the block.

   Q is used only in the G87 canned cycle [NCMS, page 98], where it must
   be positive.

   */

    void rs274ngc::read_q(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        double value;

        error_if((line[*counter] != 'q'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.q_number > -1.0), NCE_MULTIPLE_Q_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        error_if((value <= 0.0), NCE_NEGATIVE_OR_ZERO_Q_VALUE_USED);
        block.q_number = value;
    }

   /****************************************************************************/

   /* read_r

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not r:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. An r_number has already been inserted in the block:
   NCE_MULTIPLE_R_WORDS_ON_ONE_LINE

   Side effects:
   counter is reset.
   The r_flag in the block is turned on.
   The r_number is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'r'. The function reads characters
   which tell how to set the coordinate, up to the start of the next item
   or the end of the line. This information is inserted in the block. The
   counter is then set to point to the character following.

   An r number indicates the clearance plane in canned cycles.
   An r number may also be the radius of an arc.

   The value may be a real number or something that evaluates to a
   real number, so read_real_value is used to read it. Parameters
   may be involved.

   */

    void rs274ngc::read_r(                            /* ARGUMENTS                                     */
    const char * line,                                  /* string: line of RS274 code being processed    */
    int * counter,                                /* pointer to a counter for position on the line */
    block_t& block,                          /* pointer to a block being filled from the line */
    double * parameters) const                          /* array of system parameters                    */
    {
        double value;

        error_if((line[*counter] != 'r'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.r_flag != OFF), NCE_MULTIPLE_R_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.r_flag = ON;
        block.r_number = value;
    }

   /****************************************************************************/

   /* read_real_expression

   Returned Value: int
   If any of the following functions returns an error code,
   this returns that code.
   read_real_value
   read_operation
   execute_binary
   If any of the following errors occur, this returns the error shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character is not [ :
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED

   Side effects:
   The number read from the line is put into what value_ptr points at.
   The counter is reset to point to the first character after the real
   expression.

   Called by:
   read_atan
   read_real_value
   read_unary

   Example 1: [2 - 3 * 4 / 5] means [2 - [[3 * 4] / 5]] and equals -0.4.

   Segmenting Expressions -

   The RS274/NGC manual, section 3.5.1.1 [NCMS, page 50], provides for
   using square brackets to segment expressions.

   Binary Operations -

   The RS274/NGC manual, section 3.5.1.1, discusses expression evaluation.
   The manual provides for eight binary operations: the four basic
   mathematical operations (addition, subtraction, multiplication,
   division), three logical operations (non-exclusive OR, exclusive OR,
   and AND2) and the modulus operation. The manual does not explicitly call
   these "binary" operations, but implicitly recognizes that they are
   binary. We have added the "power" operation of raising the number
   on the left of the operation to the power on the right; this is
   needed for many basic machining calculations.

   There are two groups of binary operations given in the manual. If
   operations are strung together as shown in Example 1, operations in
   the first group are to be performed before operations in the second
   group. If an expression contains more than one operation from the same
   group (such as * and / in Example 1), the operation on the left is
   performed first. The first group is: multiplication (*), division (/),
   and modulus (MOD). The second group is: addition(+), subtraction (-),
   logical non-exclusive or (OR), logical exclusive or (XOR), and logical
   and (AND2). We have added a third group with higher precedence than
   the first group. The third group contains only the power (**)
   operation.

   The logical operations and modulus are apparently to be performed on
   any real numbers, not just on integers or on some other data type.

   Unary Operations -

   The RS274/NGC manual, section 3.5.1.2, provides for fifteen unary
   mathematical operations. Two of these, BIN and BCD, are apparently for
   converting between decimal and hexadecimal number representation,
   although the text is not clear. These have not been implemented, since
   we are not using any hexadecimal numbers. The other thirteen unary
   operations have been implemented: absolute_value, arc_cosine, arc_sine,
   arc_tangent, cosine, e_raised_to, fix_down, fix_up, natural_log_of,
   round, sine, square_root, tangent.

   The manual section 3.5.1.2 [NCMS, page 51] requires the argument to
   all unary operations (except atan) to be in square brackets.  Thus,
   for example "sin[90]" is allowed in the interpreter, but "sin 90" is
   not. The atan operation must be in the format "atan[..]/[..]".

   Production Rule Definitions in Terms of Tokens -

   The following is a production rule definition of what this RS274NGC
   interpreter recognizes as valid combinations of symbols which form a
   recognized real_value (the top of this production hierarchy).

   The notion of "integer_value" is used in the interpreter. Below it is
   defined as a synonym for real_value, but in fact a constraint is added
   which cannot be readily written in a production language.  An
   integer_value is a real_value which is very close to an integer.
   Integer_values are needed for array and table indices and (when
   divided by 10) for the values of M codes and G codes. All numbers
   (including integers) are read as real numbers and stored as doubles.
   If an integer_value is required in some situation, a test for being
   close to an integer is applied to the number after it is read.

   arc_tangent_combo = arc_tangent expression divided_by expression .

   binary_operation1 = divided_by | modulo | power | times .

   binary_operation2 = and | exclusive_or | minus |  non_exclusive_or | plus .

   combo1 = real_value { binary_operation1 real_value } .

   digit = zero | one | two | three | four | five | six | seven |eight | nine .

   expression =
   left_bracket
   (unary_combo | (combo1 { binary_operation2 combo1 }))
   right_bracket .

   integer_value = real_value .

   ordinary_unary_combo =  ordinary_unary_operation expression .

   ordinary_unary_operation =
   absolute_value | arc_cosine | arc_sine | cosine | e_raised_to |
   fix_down | fix_up | natural_log_of | round | sine | square_root | tangent .

   parameter_index = integer_value .

   parameter_value = parameter_sign  parameter_index .

   real_number =
   [ plus | minus ]
   (( digit { digit } decimal_point {digit}) | ( decimal_point digit {digit})).

   real_value =
   real_number | expression | parameter_value | unary_combo.

   unary_combo = ordinary_unary_combo | arc_tangent_combo .

   Production Tokens in Terms of Characters -

   absolute_value   = 'abs'
   and              = 'and'
   arc_cosine       = 'acos'
   arc_sine         = 'asin'
   arc_tangent      = 'atan'
   cosine           = 'cos'
   decimal_point    = '.'
   divided_by       = '/'
   eight            = '8'
   exclusive_or     = 'xor'
   e_raised_to      = 'exp'
   five             = '5'
   fix_down         = 'fix'
   fix_up           = 'fup'
   four             = '4'
   left_bracket     = '['
   minus            = '-'
   modulo           = 'mod'
   natural_log_of   = 'ln'
   nine             = '9'
   non_exclusive_or = 'or'
   one              = '1'
   parameter_sign   = '#'
   plus             = '+'
   power            = '**'
   right_bracket    = ']'
   round            = 'round'
   seven            = '7'
   sine             = 'sin'
   six              = '6'
   square_root      = 'sqrt'
   tangent          = 'tan'
   three            = '3'
   times            = '*'
   two              = '2'
   zero             = '0'

   When this function is called, the counter should be set at a left
   bracket. The function reads up to and including the right bracket
   which closes the expression.

   The basic form of an expression is: [v1 bop v2 bop ... vn], where the
   vi are real_values and the bops are binary operations. The vi may be
   numbers, parameters, expressions, or unary functions. Because some
   bops are to be evaluated before others, for understanding the order of
   evaluation, it is useful to rewrite the general form collecting any
   subsequences of bops of the same precedence. For example, suppose the
   expression is: [9+8*7/6+5-4*3**2+1]. It may be rewritten as:
   [9+[8*7/6]+5-[4*[3**2]]+1] to show how it should be evaluated.

   The manual provides that operations of the same precedence should be
   processed left to right.

   The first version of this function is commented out. It is suitable
   for when there are only two precendence levels. It is an improvement
   over the version used in interpreters before 2000, but not as general
   as the second version given here.

   The first version of this function reads the first value and the first
   operation in the expression. Then it calls either read_rest_bop1 or
   read_rest_bop2 according to whether the first operation is a bop1 or a
   bop2.  Read_rest_bop1 resets the next_operation to either a right
   bracket or a bop2. If it is reset to a bop2, read_rest_bop2 is called
   when read_rest_bop1 returns.

   */

#ifdef UNDEFINED
    static void read_real_expression(              /* ARGUMENTS                               */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    double * value,                               /* pointer to double to be read                   */
    double * parameters) const                          /* array of system parameters                     */
    {
        int next_operation;
        int status;

        error_if((line[*counter] != '['), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        read_real_value(line, counter, value, parameters);
        read_operation(line, counter, &next_operation);
        if (next_operation == RIGHT_BRACKET);     /* nothing to do */
        else if (next_operation < AND2)           /* next operation is a bop1, times-like */
        {
            read_rest_bop1(line, counter, value, &next_operation, parameters);
            if (next_operation == RIGHT_BRACKET); /* next_operation has been reset */
            else                                  /* next_operation is now a bop2, plus-like */
                read_rest_bop2(line, counter, value, next_operation, parameters);
        }
        else                                      /* next operation is a bop2, plus-like */
            read_rest_bop2(line, counter, value, next_operation, parameters);
    }
#endif

   /****************************************************************************/

   /*

   The following version is stack-based and fully general. It is the
   classical stack-based version with left-to-right evaluation of
   operations of the same precedence. Separate stacks are used for
   operations and values, and the stacks are made with arrays
   rather than lists, but those are implementation details. Pushing
   and popping are implemented by increasing or decreasing the
   stack index.

   Additional levels of precedence may be defined easily by changing the
   precedence function. The size of MAX_STACK should always be at least
   as large as the number of precedence levels used. We are currently
   using four precedence levels (for right-bracket, plus-like operations,
   times-like operations, and power).

   */

#define MAX_STACK 5

    void rs274ngc::read_real_expression(              /* ARGUMENTS                               */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    double * value,                               /* pointer to double to be computed               */
    double * parameters) const                          /* array of system parameters                     */
    {
        double values[MAX_STACK];
        int operators[MAX_STACK];
        int stack_index;

        error_if((line[*counter] != '['), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        read_real_value(line, counter, values, parameters);
        read_operation(line, counter, operators);
        stack_index = 1;
        for(; operators[0] != RIGHT_BRACKET ;)
        {
            read_real_value(line, counter, values+stack_index, parameters);
            read_operation(line, counter, operators+stack_index);
            if (precedence(operators[stack_index]) >
                precedence(operators[stack_index - 1]))
                stack_index++;
            else                                  /* precedence of latest operator is <= previous precedence */
            {
                for (;precedence(operators[stack_index]) <=
                    precedence(operators[stack_index - 1]); )
                {
                    execute_binary((values + stack_index - 1), operators[stack_index -1], (values + stack_index));
                    operators[stack_index - 1] = operators[stack_index];
                    if ((stack_index > 1) and
                        (precedence(operators[stack_index - 1]) <=
                        precedence(operators[stack_index - 2])))
                        stack_index--;
                    else
                        break;
                }
            }
        }
        *value = values[0];
    }

   /****************************************************************************/

   /* read_real_number

   Returned Value: int
   If any of the following errors occur, this returns the error shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character is not "+", "-", "." or a digit:
   NCE_BAD_NUMBER_FORMAT
   2. No digits are found after the first character and before the
   end of the line or the next character that cannot be part of a real:
   NCE_NO_DIGITS_FOUND_WHERE_REAL_NUMBER_SHOULD_BE
   3. sscanf fails: NCE_SSCANF_FAILED

   Side effects:
   The number read from the line is put into what double_ptr points at.
   The counter is reset to point to the first character after the real.

   Called by:  read_real_value

   This attempts to read a number out of the line, starting at the index
   given by the counter. It stops when the first character that cannot
   be part of the number is found.

   The first character may be a digit, "+", "-", or "."
   Every following character must be a digit or "." up to anything
   that is not a digit or "." (a second "." terminates reading).

   This function is not called if the first character is NULL, so it is
   not necessary to check that.

   The temporary insertion of a NULL character on the line is to avoid
   making a format string like "%3lf" which the LynxOS compiler cannot
   handle.

   */

    void rs274ngc::read_real_number(                  /* ARGUMENTS                               */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    double * double_ptr) const                          /* pointer to double to be read                   */
    {
        char c;                                   /* for character being processed    */
        int flag_digit;                           /* set to ON if digit found         */
        int flag_point;                           /* set to ON if decimal point found */
        int n;                                    /* for indexing line                */

        n = *counter;
        flag_point = OFF;
        flag_digit = OFF;

   /* check first character */
        c = line[n];
        if (c == '+')
        {
            *counter = (*counter + 1);       /* skip plus sign */
            n++;
        }
        else if (c == '-')
        {
            n++;
        }
        else if ((c != '.') and ((c < 48) or (c > 57)))
            throw error(NCE_BAD_NUMBER_FORMAT);

   /* check out rest of characters (must be digit or decimal point) */
        for (; (c = line[n]) != (char) NULL; n++)
        {
            if (( 47 < c) and ( c < 58))
            {
                flag_digit = ON;
            }
            else if (c == '.')
            {
                if (flag_point == OFF)
                {
                    flag_point = ON;
                }
                else
                    break;                        /* two decimal points, error appears on reading next item */
            }
            else
                break;
        }

        error_if(flag_digit == OFF, NCE_NO_DIGITS_FOUND_WHERE_REAL_NUMBER_SHOULD_BE);
        
        char dbl_buf[RS274NGC_TEXT_SIZE];
        strncpy(dbl_buf, line + *counter, n-*counter);
        dbl_buf[n-*counter] = '\0';
        
        if (sscanf(dbl_buf, "%lf", double_ptr) == 0)
        {
            throw error(NCE_SSCANF_FAILED);
        }
        else
        {
            *counter = n;
        }
    }

   /****************************************************************************/

   /* read_real_value

   Returned Value: int
   If one of the following functions returns an error code,
   this returns that code.
   read_real_expression
   read_parameter
   read_unary
   read_real_number
   If no characters are found before the end of the line this
   returns NCE_NO_CHARACTERS_FOUND_IN_READING_REAL_VALUE.
   Otherwise, this returns RS274NGC_OK.

   Side effects:
   The value read from the line is put into what double_ptr points at.
   The counter is reset to point to the first character after the
   characters which make up the value.

   Called by:
   read_a
   read_b
   read_c
   read_f
   read_g
   read_i
   read_integer_value
   read_j
   read_k
   read_p
   read_parameter_setting
   read_q
   read_r
   read_real_expression
   read_s
   read_x
   read_y
   read_z

   This attempts to read a real value out of the line, starting at the
   index given by the counter. The value may be a number, a parameter
   value, a unary function, or an expression. It calls one of four
   other readers, depending upon the first character.

   */

    void rs274ngc::read_real_value(                   /* ARGUMENTS                               */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    double * double_ptr,                          /* pointer to double to be read                   */
    double * parameters) const                          /* array of system parameters                     */
    {
        char c;

        c = line[*counter];
        error_if((c == 0), NCE_NO_CHARACTERS_FOUND_IN_READING_REAL_VALUE);
        if (c == '[')
            read_real_expression (line, counter, double_ptr, parameters);
        else if (c == '#')
            read_parameter(line, counter, double_ptr, parameters);
        else if ((c >= 'a') and (c <= 'z'))
            read_unary(line, counter, double_ptr, parameters);
        else
            read_real_number(line, counter, double_ptr);
    }

   /****************************************************************************/

   /* read_rest_bop1

   Returned Value: int
   If any of the following functions returns an error code,
   this returns that code.
   execute_binary1
   read_real_value
   read_operation
   Otherwise, it returns RS274NGC_OK.

   Side effects:
   The value argument is set to the value of the expression.
   The counter is reset to point to the first character after the real
   expression.

   Called by:
   read_real_expression
   read_rest_bop2

   The value argument has a value in it when this is called. This repeatedly
   gets the next_value and the next_operation, performs the last_operation
   on the value and the next_value and resets the last_operation to the
   next_operation. Observe that both the value and the last_operation
   are passed back to the caller.

   This is commented out since it is not used in the uncommented version
   of read_real_expression. It has been tested.

   */

#ifdef UNDEFINED
    static void read_rest_bop1(                    /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    double * value,                               /* pointer to double to be calculated             */
    int * last_operation,                         /* last operation read, reset to next operation   */
    double * parameters) const                          /* array of system parameters                     */
    {
        double next_value;
        int next_operation;
        int status;

        for(; ; )
        {
            read_real_value(line, counter, &next_value, parameters);
            read_operation(line, counter, &next_operation);
            execute_binary1(value, *last_operation, &next_value);
            *last_operation = next_operation;
            if (next_operation >= AND2)           /* next op is a bop2 or right bracket */
                break;
        }
    }
#endif

   /****************************************************************************/

   /* read_rest_bop2

   Returned Value: int
   If any of the following functions returns an error code,
   this returns that code.
   execute_binary2
   read_real_value
   read_operation
   read_rest_bop1
   Otherwise, it returns RS274NGC_OK.

   Side effects:
   The value argument is set to the value of the expression.
   The counter is reset to point to the first character after the real
   expression.

   Called by:  read_real_expression

   The value argument has a value in it when this is called. This repeatedly
   gets the next_value and the next_operation, performs the last_operation
   on the value and the next_value and resets the last_operation to the
   next_operation. If the next_operation is ever a bop1 read_rest_bop1 is
   called to set the next_value.

   This is commented out since it is not used in the uncommented version
   of read_real_expression. It has been tested.

   */

#ifdef UNDEFINED
    static void read_rest_bop2(                    /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    double * value,                               /* pointer to double to be calculated             */
    int last_operation,                           /* last operation read                            */
    double * parameters) const                          /* array of system parameters                     */
    {
        double next_value;
        int next_operation;
        int status;

        for(; ; last_operation = next_operation)
        {
            read_real_value(line, counter, &next_value, parameters);
            read_operation(line, counter, &next_operation);
            if (next_operation < AND2)            /* next operation is a bop1 */
            {
                read_rest_bop1(line, counter, &next_value, &next_operation, parameters);
            }
            execute_binary2(value, last_operation, &next_value);
            if (next_operation == RIGHT_BRACKET)
                break;
        }
    }
#endif

   /****************************************************************************/

   /* read_s

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not s:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. A spindle speed has already been inserted in the block:
   NCE_MULTIPLE_S_WORDS_ON_ONE_LINE
   3. The spindle speed is negative: NCE_NEGATIVE_SPINDLE_SPEED_USED

   Side effects:
   counter is reset to the character following the spindle speed.
   A spindle speed setting is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 's', indicating a spindle speed
   setting. The function reads characters which tell how to set the spindle
   speed.

   The value may be a real number or something that evaluates to a
   real number, so read_real_value is used to read it. Parameters
   may be involved.

   */

    void rs274ngc::read_s(                            /* ARGUMENTS                                     */
    const char * line,                                  /* string: line of RS274NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line */
    block_t& block,                          /* pointer to a block being filled from the line */
    double * parameters) const                          /* array of system parameters                    */
    {
        double value;

        error_if((line[*counter] != 's'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.s_number > -1.0), NCE_MULTIPLE_S_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        error_if((value < 0.0), NCE_NEGATIVE_SPINDLE_SPEED_USED);
        block.s_number = value;
    }

   /****************************************************************************/

   /* read_t

   Returned Value: int
   If read_integer_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not t:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. A t_number has already been inserted in the block:
   NCE_MULTIPLE_T_WORDS_ON_ONE_LINE
   3. The t_number is negative: NCE_NEGATIVE_TOOL_ID_USED

   Side effects:
   counter is reset to the character following the t_number.
   A t_number is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 't', indicating a tool.
   The function reads characters which give the (integer) value of the
   tool code.

   The value must be an integer or something that evaluates to a
   real number, so read_integer_value is used to read it. Parameters
   may be involved.

   */

    void rs274ngc::read_t(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        int value;

        error_if((line[*counter] != 't'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.t_number > -1), NCE_MULTIPLE_T_WORDS_ON_ONE_LINE);
        read_integer_value(line, counter, &value, parameters);
        error_if((value < 0), NCE_NEGATIVE_TOOL_ID_USED);
        block.t_number = value;
    }

   /****************************************************************************/

   /* read_text

   Returned Value: int
   If close_and_downcase returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, this returns:
   a. RS274NGC_ENDFILE if the percent_flag is ON and the only
   non-white character on the line is %,
   b. RS274NGC_EXECUTE_FINISH if the first character of the
   close_and_downcased line is a slash, and
   c. RS274NGC_OK otherwise.
   1. The end of the file is found and the percent_flag is ON:
   NCE_FILE_ENDED_WITH_NO_PERCENT_SIGN
   2. The end of the file is found and the percent_flag is OFF:
   NCE_FILE_ENDED_WITH_NO_PERCENT_SIGN_OR_PROGRAM_END
   3. The command argument is not null and is too long or the command
   argument is null and the line read from the file is too long:
   NCE_COMMAND_TOO_LONG

   Side effects: See below

   Called by:  rs274ngc_read

   This reads a line of RS274 code from a command string or a file into
   the line array. If the command string is not null, the file is ignored.

   If the end of file is reached, an error is returned as described
   above. The end of the file should not be reached because (a) if the
   file started with a percent line, it must end with a percent line, and
   no more reading of the file should occur after that, and (b) if the
   file did not start with a percent line, it must have a program ending
   command (M2 or M30) in it, and no more reading of the file should
   occur after that.

   All blank space at the end of a line read from a file is removed and
   replaced here with NULL characters.

   This then calls close_and_downcase to downcase and remove tabs and
   spaces from everything on the line that is not part of a comment. Any
   comment is left as is.

   The length is set to zero if any of the following occur:
   1. The line now starts with a slash, but the second character is NULL.
   2. The first character is NULL.
   Otherwise, length is set to the length of the line.

   An input line is blank if the first character is NULL or it consists
   entirely of tabs and spaces and, possibly, a newline before the first
   NULL.

   Block delete is discussed in [NCMS, page 3] but the discussion makes
   no sense. Block delete is handled by having this function return
   RS274NGC_EXECUTE_FINISH if the first character of the
   close_and_downcased line is a slash. When the caller sees this,
   the caller is expected not to call rs274ngc_execute if the switch
   is on, but rather call rs274ngc_read again to overwrite and ignore
   what is read here.

   The value of the length argument is set to the number of characters on
   the reduced line.

   */

    int rs274ngc::read_text(                         /* ARGUMENTS                                   */
    const char * command,                         /* a string which has input text */
    char * raw_line,                              /* array to write raw input line into          */
    char * line,                                  /* array for input line to be processed in     */
    int * length)                                 /* a pointer to an integer to be set           */
    {
        error_if((strlen(command) >= RS274NGC_TEXT_SIZE), NCE_COMMAND_TOO_LONG);
        strcpy(raw_line, command);
        strcpy(line, command);
        close_and_downcase(line);
        
        _setup.sequence_number++;
        if ((line[0] == 0) or ((line[0] == '/') and (line[1] == 0)))
            *length = 0;
        else
            *length = strlen(line);

        return ((line[0] == '/')? RS274NGC_EXECUTE_FINISH : RS274NGC_OK);
    }

   /****************************************************************************/

   /* read_unary

   Returned Value: int
   If any of the following functions returns an error code,
   this returns that code.
   execute_unary
   read_atan
   read_operation_unary
   read_real_expression
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. the name of the unary operation is not followed by a left bracket:
   NCE_LEFT_BRACKET_MISSING_AFTER_UNARY_OPERATION_NAME

   Side effects:
   The value read from the line is put into what double_ptr points at.
   The counter is reset to point to the first character after the
   characters which make up the value.

   Called by:  read_real_value

   This attempts to read the value of a unary operation out of the line,
   starting at the index given by the counter. The atan operation is
   handled specially because it is followed by two arguments.

   */

    void rs274ngc::read_unary(                        /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274/NGC code being processed */
    int * counter,                                /* pointer to a counter for position on the line  */
    double * double_ptr,                          /* pointer to double to be read                   */
    double * parameters) const                          /* array of system parameters                     */
    {
        int operation;

        read_operation_unary (line, counter, &operation);
        error_if((line[*counter] != '['), NCE_LEFT_BRACKET_MISSING_AFTER_UNARY_OPERATION_NAME);
        read_real_expression (line, counter, double_ptr, parameters);

        if (operation == ATAN)
            read_atan(line, counter, double_ptr, parameters);
        else
            execute_unary(double_ptr, operation);
    }

   /****************************************************************************/

   /* read_x

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not x:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. A x_coordinate has already been inserted in the block:
   NCE_MULTIPLE_X_WORDS_ON_ONE_LINE

   Side effects:
   counter is reset.
   The x_flag in the block is turned on.
   An x_number is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'x', indicating a x_coordinate
   setting. The function reads characters which tell how to set the
   coordinate, up to the start of the next item or the end of the line.
   This information is inserted in the block. The counter is then set to
   point to the character following.

   The value may be a real number or something that evaluates to a
   real number, so read_real_value is used to read it. Parameters
   may be involved.

   */

    void rs274ngc::read_x(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274 code being processed     */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        double value;

        error_if((line[*counter] != 'x'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.x_flag != OFF), NCE_MULTIPLE_X_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.x_flag = ON;
        block.x_number = value;
    }

   /****************************************************************************/

   /* read_y

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not y:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. A y_coordinate has already been inserted in the block:
   NCE_MULTIPLE_Y_WORDS_ON_ONE_LINE

   Side effects:
   counter is reset.
   The y_flag in the block is turned on.
   A y_number is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'y', indicating a y_coordinate
   setting. The function reads characters which tell how to set the
   coordinate, up to the start of the next item or the end of the line.
   This information is inserted in the block. The counter is then set to
   point to the character following.

   The value may be a real number or something that evaluates to a
   real number, so read_real_value is used to read it. Parameters
   may be involved.

   */

    void rs274ngc::read_y(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274 code being processed     */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        double value;

        error_if((line[*counter] != 'y'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.y_flag != OFF), NCE_MULTIPLE_Y_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.y_flag = ON;
        block.y_number = value;
    }

   /****************************************************************************/

   /* read_z

   Returned Value: int
   If read_real_value returns an error code, this returns that code.
   If any of the following errors occur, this returns the error code shown.
   Otherwise, it returns RS274NGC_OK.
   1. The first character read is not z:
   NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED
   2. A z_coordinate has already been inserted in the block:
   NCE_MULTIPLE_Z_WORDS_ON_ONE_LINE

   Side effects:
   counter is reset.
   The z_flag in the block is turned on.
   A z_number is inserted in the block.

   Called by: read_one_item

   When this function is called, counter is pointing at an item on the
   line that starts with the character 'z', indicating a z_coordinate
   setting. The function reads characters which tell how to set the
   coordinate, up to the start of the next item or the end of the line.
   This information is inserted in the block. The counter is then set to
   point to the character following.

   The value may be a real number or something that evaluates to a
   real number, so read_real_value is used to read it. Parameters
   may be involved.

   */

    void rs274ngc::read_z(                            /* ARGUMENTS                                      */
    const char * line,                                  /* string: line of RS274 code being processed     */
    int * counter,                                /* pointer to a counter for position on the line  */
    block_t& block,                          /* pointer to a block being filled from the line  */
    double * parameters) const                          /* array of system parameters                     */
    {
        double value;

        error_if((line[*counter] != 'z'), NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if((block.z_flag != OFF), NCE_MULTIPLE_Z_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.z_flag = ON;
        block.z_number = value;
    }

   /****************************************************************************/

   /* set_probe_data

   Returned Value: int (RS274NGC_OK)

   Side effects:
   The current position is set.
   System parameters for probe position are set.

   Called by:  rs274ngc_read

   */

    void rs274ngc::set_probe_data(                    /* ARGUMENTS                   */
    setup_t& settings)                       /* pointer to machine settings */
    {
        settings.current.x = GET_EXTERNAL_POSITION_X();
        settings.current.y = GET_EXTERNAL_POSITION_Y();
        settings.current.z = GET_EXTERNAL_POSITION_Z();
        settings.current.a = GET_EXTERNAL_POSITION_A();
        settings.current.b = GET_EXTERNAL_POSITION_B();
        settings.current.c = GET_EXTERNAL_POSITION_C();
        settings.parameters[5061] = GET_EXTERNAL_PROBE_POSITION_X();
        settings.parameters[5062] = GET_EXTERNAL_PROBE_POSITION_Y();
        settings.parameters[5063] = GET_EXTERNAL_PROBE_POSITION_Z();
        settings.parameters[5064] = GET_EXTERNAL_PROBE_POSITION_A();
        settings.parameters[5065] = GET_EXTERNAL_PROBE_POSITION_B();
        settings.parameters[5066] = GET_EXTERNAL_PROBE_POSITION_C();
        settings.parameters[5067] = GET_EXTERNAL_PROBE_VALUE();
    }

   /****************************************************************************/
   /* write_g_codes

   Returned Value: int (RS274NGC_OK)

   Side effects:
   The active_g_codes in the settings are updated.

   Called by:
   rs274ngc_execute
   rs274ngc_init

   The block may be NULL.

   This writes active g_codes into the settings.active_g_codes array by
   examining the interpreter settings. The array of actives is composed
   of ints, so (to handle codes like 59.1) all g_codes are reported as
   ints ten times the actual value. For example, 59.1 is reported as 591.

   The correspondence between modal groups and array indexes is as follows
   (no apparent logic to it).

   The group 0 entry is taken from the block (if there is one), since its
   codes are not modal.

   group 0  - gez[2]  g4, g10, g28, g30, g53, g92 g92.1, g92.2, g92.3 - misc
   group 1  - gez[1]  g0, g1, g2, g3, g38.2, g80, g81, g82, g83, g84, g85,
   g86, g87, g88, g89 - motion
   group 2  - gez[3]  g17, g18, g19 - plane selection
   group 3  - gez[6]  g90, g91 - distance mode
   group 4  - no such group
   group 5  - gez[7]  g93, g94 - feed rate mode
   group 6  - gez[5]  g20, g21 - units
   group 7  - gez[4]  g40, g41, g42 - cutter radius compensation
   group 8  - gez[9]  g43, g49 - tool length offse
   group 9  - no such group
   group 10 - gez[10] g98, g99 - return mode in canned cycles
   group 11 - no such group
   group 12 - gez[8]  g54, g55, g56, g57, g58, g59, g59.1, g59.2, g59.3
   - coordinate system
   group 13 - gez[11] g61, g61.1, g64 - control mode

   */

    void rs274ngc::write_g_codes(                     /* ARGUMENTS                                    */
    const block_t* block,                          /* pointer to a block of RS274/NGC instructions */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        int * gez;

        gez = settings.active_g_codes;
        gez[0] = settings.sequence_number;
        gez[1] = settings.motion_mode;
        gez[2] = ((block == NULL) ? -1 : block->g_modes[0]);
        gez[3] =
            (settings.plane == CANON_PLANE_XY) ? G_17 :
        (settings.plane == CANON_PLANE_XZ) ? G_18 : G_19;
        gez[4] =
            (settings.cutter_comp_side == RIGHT) ? G_42 :
        (settings.cutter_comp_side == LEFT) ? G_41 : G_40;
        gez[5] =
            (settings.length_units == CANON_UNITS_INCHES) ? G_20 : G_21;
        gez[6] =
            (settings.distance_mode == MODE_ABSOLUTE) ? G_90 : G_91;
        gez[7] =
            (settings.feed_mode == INVERSE_TIME) ? G_93 : G_94;
        gez[8] =
            (settings.origin_index < 7) ? (530 + (10 * settings.origin_index)) :
        (584 + settings.origin_index);
        gez[9] =
            (settings.tool_length_offset == 0.0) ? G_49 : G_43;
        gez[10] =
            (settings.retract_mode == OLD_Z) ? G_98 : G_99;
        gez[11] =
            (settings.control_mode == CANON_CONTINUOUS) ? G_64 :
        (settings.control_mode == CANON_EXACT_PATH) ? G_61 : G_61_1;
    }

   /****************************************************************************/

   /* write_m_codes

   Returned Value: int (RS274NGC_OK)

   Side effects:
   The settings.active_m_codes are updated.

   Called by:
   rs274ngc_execute
   rs274ngc_init

   This is testing only the feed override to see if overrides is on.
   Might add check of speed override.

   */

    void rs274ngc::write_m_codes(                     /* ARGUMENTS                                    */
    const block_t* block,                          /* pointer to a block of RS274/NGC instructions */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        int * emz;

        emz = settings.active_m_codes;
        emz[0] = settings.sequence_number;  /* 0 seq number  */
        emz[1] =
            (block == NULL) ? -1 : block->m_modes[4];/* 1 stopping    */
        emz[2] =
   /* 2 spindle     */
            (settings.spindle_turning == CANON_STOPPED) ? 5 :
        (settings.spindle_turning == CANON_CLOCKWISE) ? 3 : 4;
        emz[3] =                             /* 3 tool change */
            (block == NULL) ? -1 : block->m_modes[6];
        emz[4] =                             /* 4 mist        */
            (settings.mist == ON) ? 7 :
        (settings.flood == ON) ? -1 : 9;
        emz[5] =                             /* 5 flood       */
            (settings.flood == ON) ? 8 : -1;
        emz[6] =                             /* 6 overrides   */
            (settings.feed_override == ON) ? 48 : 49;
    }

   /****************************************************************************/

   /* write_settings

   Returned Value: int (RS274NGC_OK)

   Side effects:
   The settings.active_settings array of doubles is updated with the
   sequence number, feed, and speed settings.

   Called by:
   rs274ngc_execute
   rs274ngc_init

   */

    void rs274ngc::write_settings(                    /* ARGUMENTS                   */
    setup_t& settings)                       /* pointer to machine settings */
    {
        double * vals;

        vals = settings.active_settings;
        vals[0] = settings.sequence_number; /* 0 sequence number */
        vals[1] = settings.feed_rate;       /* 1 feed rate       */
        vals[2] = settings.speed;           /* 2 spindle speed   */
    }

   /****************************************************************************/
   /****************************************************************************/

   /*

   The functions in this section of this file are functions for
   external programs to call to tell the rs274ngc interpreter
   what to do. They are declared in rs274ngc.hh.

   */

   /***********************************************************************/

   /* rs274ngc_execute

   Returned Value: int)
   If execute_block returns RS274NGC_EXIT, this returns that.
   If execute_block returns RS274NGC_EXECUTE_FINISH, this returns that.
   If execute_block returns an error code, this returns that code.
   Otherwise, this returns RS274NGC_OK.

   Side Effects:
   Calls to canonical machining commands are made.
   The interpreter variables are changed.
   At the end of the program, the file is closed.
   If using a file, the active G codes and M codes are updated.

   Called By: external programs

   This executes a previously parsed block.

   */

    int rs274ngc::execute()                        /* NO ARGUMENTS */
    {
        int status;

        if (_setup.line_length != 0)            /* line not blank */
        {
		    for (size_t n = 0; n < _setup.block1.parameter_occurrence; ++n)
		    {                                         // copy parameter settings from parameter buffer into parameter table
		        _setup.parameters[_setup.block1.parameter_numbers[n]] = _setup.block1.parameter_values[n];
		    }
            status = execute_block (_setup.block1, _setup);
            write_g_codes(&_setup.block1, _setup);
            write_m_codes(&_setup.block1, _setup);
            write_settings(_setup);
            if ((status != RS274NGC_OK) and
                (status != RS274NGC_EXECUTE_FINISH) and
                (status != RS274NGC_EXIT))
                throw error(status);
        }
        else                                      /* blank line is OK */
            status = RS274NGC_OK;
        return status;
    }

   /***********************************************************************/

   /* rs274ngc_exit

   Returned Value: int (RS274NGC_OK)

   Side Effects: See below

   Called By: external programs

   The system parameters are saved to a file and some parts of the world
   model are reset. If GET_EXTERNAL_PARAMETER_FILE_NAME provides a
   non-empty file name, that name is used for the file that is
   written. Otherwise, the default parameter file name is used.

   */

    void rs274ngc::exit()                           /* NO ARGUMENTS */
    {
        char file_name[RS274NGC_TEXT_SIZE];

        GET_EXTERNAL_PARAMETER_FILE_NAME(file_name, (RS274NGC_TEXT_SIZE - 1));
        save_parameters
            (((file_name[0] == 0) ? RS274NGC_PARAMETER_FILE_NAME_DEFAULT : file_name),
            _setup.parameters);
        reset();
    }

   /***********************************************************************/

   /* rs274_ngc_init

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, this returns RS274NGC_OK.
   1. rs274ngc_restore_parameters returns an error code.
   2. Parameter 5220, the work coordinate system index, is not in the range
   1 to 9: NCE_COORDINATE_SYSTEM_INDEX_PARAMETER_5220_OUT_OF_RANGE

   Side Effects:
   Many values in the _setup structure are reset.
   A USE_LENGTH_UNITS canonical command call is made.
   A SET_FEED_REFERENCE canonical command call is made.
   A SET_ORIGIN_OFFSETS canonical command call is made.
   An INIT_CANON call is made.

   Called By: external programs

   Currently we are running only in CANON_XYZ feed_reference mode.  There
   is no command regarding feed_reference in the rs274 language (we
   should try to get one added). The initialization routine, therefore,
   always calls SET_FEED_REFERENCE(CANON_XYZ).

   */

    void rs274ngc::init()                           /* NO ARGUMENTS */
    {
        int k;                                    // starting index in parameters of origin offsets
        char filename[RS274NGC_TEXT_SIZE];
        double * pars;                            // short name for _setup.parameters

        INIT_CANON();
        _setup.length_units = GET_EXTERNAL_LENGTH_UNIT_TYPE();
        USE_LENGTH_UNITS(_setup.length_units);
        GET_EXTERNAL_PARAMETER_FILE_NAME(filename, RS274NGC_TEXT_SIZE);
        if (filename[0] == 0)
            strcpy(filename, RS274NGC_PARAMETER_FILE_NAME_DEFAULT);
        restore_parameters(filename);
        pars = _setup.parameters;
        _setup.origin_index = (int)(pars[5220] + 0.0001);
        error_if(((_setup.origin_index < 1) or (_setup.origin_index > 9)),
            NCE_COORDINATE_SYSTEM_INDEX_PARAMETER_5220_OUT_OF_RANGE);
        k = (5200 + (_setup.origin_index * 20));
        SET_ORIGIN_OFFSETS((pars[k + 1] + pars[5211]),
            (pars[k + 2] + pars[5212]),
            (pars[k + 3] + pars[5213])
            ,            (pars[k + 4] + pars[5214])
            ,            (pars[k + 5] + pars[5215])
            ,            (pars[k + 6] + pars[5216])
            );
        SET_FEED_REFERENCE(CANON_XYZ);
        _setup.axis_offset.a = pars[5214];  /*AA*/
   //_setup.current.a set in rs274ngc_synch
        _setup.origin_offset.a = pars[k + 4];
   //_setup.active_g_codes initialized below
   //_setup.active_m_codes initialized below
   //_setup.active_settings initialized below
        _setup.axis_offset.x = pars[5211];
        _setup.axis_offset.y = pars[5212];
        _setup.axis_offset.z = pars[5213];
        _setup.axis_offset.b = pars[5215];  /*BB*/
   //_setup.current.b set in rs274ngc_synch
        _setup.origin_offset.b = pars[k + 5];
   //_setup.block1 does not need initialization
        _setup.blocktext[0] = 0;
        _setup.axis_offset.c = pars[5216];  /*CC*/
   //_setup.current.c set in rs274ngc_synch
        _setup.origin_offset.c = pars[k + 6];
   //_setup.current_slot set in rs274ngc_synch
   //_setup.current_x set in rs274ngc_synch
   //_setup.current_y set in rs274ngc_synch
   //_setup.current_z set in rs274ngc_synch
        _setup.cutter_comp_side = OFF;
   //_setup.cycle values do not need initialization
        _setup.distance_mode = MODE_ABSOLUTE;
        _setup.feed_mode = UNITS_PER_MINUTE;
        _setup.feed_override = ON;
   //_setup.feed_rate set in rs274ngc_synch
   //_setup.flood set in rs274ngc_synch
        _setup.length_offset_index = 1;
   //_setup.length_units set in rs274ngc_synch
        _setup.line_length = 0;
        _setup.linetext[0] = 0;
   //_setup.mist set in rs274ngc_synch
        _setup.motion_mode = G_80;
   //_setup.origin_index set above
        _setup.origin_offset.x = pars[k + 1];
        _setup.origin_offset.y = pars[k + 2];
        _setup.origin_offset.z = pars[k + 3];
   //_setup.parameters set above
   //_setup.parameter_occurrence does not need initialization
   //_setup.parameter_numbers does not need initialization
   //_setup.parameter_values does not need initialization
   //_setup.percent_flag does not need initialization
   //_setup.plane set in rs274ngc_synch
        _setup.probe_flag = OFF;
        _setup.program_x = UNKNOWN;          /* for cutter comp */
        _setup.program_y = UNKNOWN;          /* for cutter comp */
   //_setup.retract_mode does not need initialization
   //_setup.selected_tool_slot set in rs274ngc_synch
        _setup.sequence_number = 0;          /*DOES THIS NEED TO BE AT TOP? */
   //_setup.speed set in rs274ngc_synch
        _setup.speed_feed_mode = CANON_INDEPENDENT;
        _setup.speed_override = ON;
   //_setup.spindle_turning set in rs274ngc_synch
   //_setup.stack does not need initialization
   //_setup.stack_index does not need initialization
        _setup.tool_length_offset = 0.0;
   //_setup.tool_max set in rs274ngc_synch
   //_setup.tool_table set in rs274ngc_synch
        _setup.tool_table_index = 1;
   //_setup.traverse_rate set in rs274ngc_synch

        write_g_codes(nullptr, _setup);
        write_m_codes(nullptr, _setup);
        write_settings(_setup);

   // Synch rest of settings to external world
        synch();
    }

   /***********************************************************************/

   /* rs274ngc_load_tool_table

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, this returns RS274NGC_OK.
   1. _setup.tool_max is larger than CANON_TOOL_MAX: NCE_TOOL_MAX_TOO_LARGE

   Side Effects:
   _setup.tool_table[] is modified.

   Called By:
   rs274ngc_synch
   external programs

   This function calls the canonical interface function GET_EXTERNAL_TOOL_TABLE
   to load the whole tool table into the _setup.

   The CANON_TOOL_MAX is an upper limit for this software. The
   _setup.tool_max is intended to be set for a particular machine.

   */

    void rs274ngc::load_tool_table()                /* NO ARGUMENTS */
    {
        int n;

        error_if((_setup.tool_max > CANON_TOOL_MAX), NCE_TOOL_MAX_TOO_LARGE);
        for (n = 0; n <= _setup.tool_max; n++)
        {
            _setup.tool_table[n] = GET_EXTERNAL_TOOL_TABLE(n);
        }
        for(; n <= CANON_TOOL_MAX; n++)
        {
            _setup.tool_table[n].id = 0;
            _setup.tool_table[n].length = 0;
            _setup.tool_table[n].diameter = 0;
        }
    }

   /***********************************************************************/

   /* rs274ngc_read

   Returned Value: int
   If any of the following errors occur, this returns the error code shown.
   Otherwise, this returns:
   a. RS274NGC_ENDFILE if the only non-white character on the line is %,
   b. RS274NGC_EXECUTE_FINISH if the first character of the
   close_and_downcased line is a slash, and
   c. RS274NGC_OK otherwise.
   1. The command and_setup.file_pointer are both NULL: NCE_FILE_NOT_OPEN
   2. The probe_flag is ON but the HME command queue is not empty:
   NCE_QUEUE_IS_NOT_EMPTY_AFTER_PROBING
   3. If read_text (which gets a line of NC code from file) or parse_line
   (which parses the line) returns an error code, this returns that code.

   Side Effects:
   _setup.sequence_number is incremented.
   The _setup.block1 is filled with data.

   Called By: external programs

   This reads a line of NC-code from the command string or, (if the
   command string is NULL) from the currently open file. The
   _setup.line_length will be set by read_text. This will be zero if the
   line is blank or consists of nothing but a slash. If the length is not
   zero, this parses the line into the _setup.block1.

   */

    int rs274ngc::read(                            /* ARGUMENTS                       */
    const char * command)                         /* a string to read */
    {
        int read_status;

        if (_setup.probe_flag == ON)
        {
            error_if((GET_EXTERNAL_QUEUE_EMPTY() == 0), NCE_QUEUE_IS_NOT_EMPTY_AFTER_PROBING);
            set_probe_data(_setup);
            _setup.probe_flag = OFF;
        }
        error_if(((command == NULL)), NCE_FILE_NOT_OPEN);
        read_status = read_text(command, _setup.linetext, _setup.blocktext, &_setup.line_length);
        if ((read_status == RS274NGC_EXECUTE_FINISH) or
            (read_status == RS274NGC_OK))
        {
            if (_setup.line_length != 0)
            {
                parse_line(_setup.blocktext, _setup.block1, _setup);
            }
        }
        else if (read_status == RS274NGC_ENDFILE);
        else
            throw error(read_status);
        return read_status;
    }

   /***********************************************************************/

   /* rs274ngc_reset

   Returned Value: int (RS274NGC_OK)

   Side Effects: See below

   Called By:
   external programs
   rs274ngc_close
   rs274ngc_exit
   rs274ngc_open

   This function resets the parts of the _setup model having to do with
   reading and interpreting one line. It does not affect the parts of the
   model dealing with a file being open; rs274ngc_open and rs274ngc_close
   do that.

   There is a hierarchy of resetting the interpreter. Each of the following
   calls does everything the ones above it do.

   rs274ngc_reset()
   rs274ngc_close()
   rs274ngc_init()

   In addition, rs274ngc_synch and rs274ngc_restore_parameters (both of
   which are called by rs274ngc_init) change the model.

   */

    void rs274ngc::reset()
    {
        _setup.linetext[0] = 0;
        _setup.blocktext[0] = 0;
        _setup.line_length = 0;
    }

   /***********************************************************************/

   /* rs274ngc_restore_parameters

   Returned Value:
   If any of the following errors occur, this returns the error code shown.
   Otherwise it returns RS274NGC_OK.
   1. The parameter file cannot be opened for reading: NCE_UNABLE_TO_OPEN_FILE
   2. A parameter index is out of range: NCE_PARAMETER_NUMBER_OUT_OF_RANGE
   3. A required parameter is missing from the file:
   NCE_REQUIRED_PARAMETER_MISSING
   4. The parameter file is not in increasing order:
   NCE_PARAMETER_FILE_OUT_OF_ORDER

   Side Effects: See below

   Called By:
   external programs
   rs274ngc_init

   This function restores the parameters from a file, modifying the
   parameters array. Usually parameters is _setup.parameters. The file
   contains lines of the form:

   <variable number> <value>

   e.g.,

   5161 10.456

   The variable numbers must be in increasing order, and certain
   parameters must be included, as given in the _required_parameters
   array. These are the axis offsets, the origin index (5220), and nine
   sets of origin offsets. Any parameter not given a value in the file
   has its value set to zero.

   */
    void rs274ngc::restore_parameters(              /* ARGUMENTS                        */
    const char * filename)                        /* name of parameter file to read   */
    {
        FILE * infile;
        char line[256];
        int variable;
        double value;
        int required;                             // number of next required parameter
        int index;                                // index into _required_parameters
        double * pars;                            // short name for _setup.parameters
        int k;

   // open original for reading
        infile = fopen(filename, "r");
        error_if((infile == NULL), NCE_UNABLE_TO_OPEN_FILE);

        pars = _setup.parameters;
        k = 0;
        index = 0;
        required = _required_parameters[index++];
        while (feof(infile) == 0)
        {
            if (fgets(line, 256, infile) == NULL)
            {
                break;
            }

   // try for a variable-value match in the file
            if (sscanf(line, "%d %lf", &variable, &value) == 2)
            {
                error_if(((variable <= 0) or (variable >= RS274NGC_MAX_PARAMETERS)), NCE_PARAMETER_NUMBER_OUT_OF_RANGE);
                for (; k < RS274NGC_MAX_PARAMETERS; k++)
                {
                    if (k > variable)
                        throw error(NCE_PARAMETER_FILE_OUT_OF_ORDER);
                    else if (k == variable)
                    {
                        pars[k] = value;
                        if (k == required)
                            required = _required_parameters[index++];
                        k++;
                        break;
                    }
                    else                          // if (k < variable)
                    {
                        if (k == required)
                            throw error(NCE_REQUIRED_PARAMETER_MISSING);
                        else
                            pars[k] = 0;
                    }
                }
            }
        }
        fclose(infile);
        error_if((required != RS274NGC_MAX_PARAMETERS), NCE_REQUIRED_PARAMETER_MISSING);
        for (; k < RS274NGC_MAX_PARAMETERS; k++)
        {
            pars[k] = 0;
        }
    }

   /***********************************************************************/

   /* rs274ngc_save_parameters

   Returned Value:
   If any of the following errors occur, this returns the error code shown.
   Otherwise it returns RS274NGC_OK.
   1. The existing file cannot be renamed:  NCE_CANNOT_CREATE_BACKUP_FILE
   2. The renamed file cannot be opened to read: NCE_CANNOT_OPEN_BACKUP_FILE
   3. The new file cannot be opened to write: NCE_CANNOT_OPEN_VARIABLE_FILE
   4. A parameter index is out of range: NCE_PARAMETER_NUMBER_OUT_OF_RANGE
   5. The renamed file is out of order: NCE_PARAMETER_FILE_OUT_OF_ORDER

   Side Effects: See below

   Called By:
   external programs
   rs274ngc_exit

   A file containing variable-value assignments is updated. The old
   version of the file is saved under a different name.  For each
   variable-value pair in the old file, a line is written in the new file
   giving the current value of the variable.  File lines have the form:

   <variable number> <value>

   e.g.,

   5161 10.456

   If a required parameter is missing from the input file, this does not
   complain, but does write it in the output file.

   */
    void rs274ngc::save_parameters(                 /* ARGUMENTS             */
    const char * filename,                        /* name of file to write */
    const double parameters[])                    /* parameters to save    */
    {
        FILE * infile;
        FILE * outfile;
        char line[256];
        int variable;
        double value;
        int required;                             // number of next required parameter
        int index;                                // index into _required_parameters
        int k;

   // rename as .bak
        strcpy(line, filename);
        strcat(line, RS274NGC_PARAMETER_FILE_BACKUP_SUFFIX);
        error_if((rename(filename, line) != 0), NCE_CANNOT_CREATE_BACKUP_FILE);

   // open backup for reading
        infile = fopen(line, "r");
        error_if((infile == NULL), NCE_CANNOT_OPEN_BACKUP_FILE);

   // open original for writing
        outfile = fopen(filename, "w");
        error_if((outfile == NULL), NCE_CANNOT_OPEN_VARIABLE_FILE);

        k = 0;
        index = 0;
        required = _required_parameters[index++];
        while (feof(infile) == 0)
        {
            if (fgets(line, 256, infile) == NULL)
            {
                break;
            }
   // try for a variable-value match
            if (sscanf(line, "%d %lf", &variable, &value) == 2)
            {
                error_if(((variable <= 0) or (variable >= RS274NGC_MAX_PARAMETERS)), NCE_PARAMETER_NUMBER_OUT_OF_RANGE);
                for (; k < RS274NGC_MAX_PARAMETERS; k++)
                {
                    if (k > variable)
                        throw error(NCE_PARAMETER_FILE_OUT_OF_ORDER);
                    else if (k == variable)
                    {
                        sprintf(line, "%d\t%f\n", k, parameters[k]);
                        fputs(line, outfile);
                        if (k == required)
                            required = _required_parameters[index++];
                        k++;
                        break;
                    }
                    else if (k == required)       // know (k < variable)
                    {
                        sprintf(line, "%d\t%f\n", k, parameters[k]);
                        fputs(line, outfile);
                        required = _required_parameters[index++];
                    }
                }
            }
        }
        fclose(infile);
        for (; k < RS274NGC_MAX_PARAMETERS; k++)
        {
            if (k == required)
            {
                sprintf(line, "%d\t%f\n", k, parameters[k]);
                fputs(line, outfile);
                required = _required_parameters[index++];
            }
        }
        fclose(outfile);
    }

   /***********************************************************************/

   /* rs274ngc_synch

   Returned Value: int (RS274NGC_OK)

   Side Effects:
   sets the value of many attribute of _setup by calling various
   GET_EXTERNAL_xxx functions.

   Called By:
   rs274ngc_init
   external programs

   This function gets the _setup world model in synch with the rest of
   the controller.

   */

    void rs274ngc::synch()                          /* NO ARGUMENTS */
    {
        _setup.control_mode = GET_EXTERNAL_MOTION_CONTROL_MODE();
        _setup.current.a = GET_EXTERNAL_POSITION_A();
        _setup.current.b = GET_EXTERNAL_POSITION_B();
        _setup.current.c = GET_EXTERNAL_POSITION_C();
        _setup.current_slot = GET_EXTERNAL_TOOL_SLOT();
        _setup.current.x = GET_EXTERNAL_POSITION_X();
        _setup.current.y = GET_EXTERNAL_POSITION_Y();
        _setup.current.z = GET_EXTERNAL_POSITION_Z();
        _setup.feed_rate = GET_EXTERNAL_FEED_RATE();
        _setup.flood = (GET_EXTERNAL_FLOOD() != 0) ? ON : OFF;
        _setup.length_units = GET_EXTERNAL_LENGTH_UNIT_TYPE();
        _setup.mist = (GET_EXTERNAL_MIST() != 0) ? ON : OFF;
        _setup.plane = GET_EXTERNAL_PLANE();
        _setup.selected_tool_slot = GET_EXTERNAL_TOOL_SLOT();
        _setup.speed = GET_EXTERNAL_SPEED();
        _setup.spindle_turning = GET_EXTERNAL_SPINDLE();
        _setup.tool_max = GET_EXTERNAL_TOOL_MAX();
        _setup.traverse_rate = GET_EXTERNAL_TRAVERSE_RATE();

        load_tool_table();               /*  must set  _setup.tool_max first */
    }

   /***********************************************************************/
   /***********************************************************************/

   /*

   The functions in this section are to extract information from the
   interpreter.

   */

   /***********************************************************************/

   /* rs274ngc_active_g_codes

   Returned Value: none

   Side Effects: copies active G codes into the codes array

   Called By: external programs

   See documentation of write_g_codes.

   */

    void rs274ngc::active_g_codes(                 /* ARGUMENTS                   */
    int * codes)                                  /* array of codes to copy into */
    {
        int n;

        for (n = 0; n < RS274NGC_ACTIVE_G_CODES; n++)
        {
            codes[n] = _setup.active_g_codes[n];
        }
    }

   /***********************************************************************/

   /* rs274ngc_active_m_codes

   Returned Value: none

   Side Effects: copies active M codes into the codes array

   Called By: external programs

   See documentation of write_m_codes.

   */

    void rs274ngc::active_m_codes(                 /* ARGUMENTS                   */
    int * codes)                                  /* array of codes to copy into */
    {
        int n;

        for (n = 0; n < RS274NGC_ACTIVE_M_CODES; n++)
        {
            codes[n] = _setup.active_m_codes[n];
        }
    }

   /***********************************************************************/

   /* rs274ngc_active_settings

   Returned Value: none

   Side Effects: copies active F, S settings into array

   Called By: external programs

   See documentation of write_settings.

   */

    void rs274ngc::active_settings(                /* ARGUMENTS                      */
    double * settings)                            /* array of settings to copy into */
    {
        int n;

        for (n = 0; n < RS274NGC_ACTIVE_SETTINGS; n++)
        {
            settings[n] = _setup.active_settings[n];
        }
    }

   /***********************************************************************/

   /* rs274ngc_error_text

   Returned Value: none

   Side Effects: see below

   Called By: external programs

   This copies the error string whose index in the _rs274ngc_errors array
   is error_code into the error_text array -- unless the error_code is
   an out-of-bounds index or the length of the error string is not less
   than max_size, in which case an empty string is put into the
   error_text. The length of the error_text array should be at least
   max_size.

   */

    void rs274ngc::error_text(                     /* ARGUMENTS                            */
    int error_code,                               /* code number of error                 */
    char * error_text,                            /* char array to copy error text into   */
    size_t max_size)                                 /* maximum number of characters to copy */
    {
        if (((error_code >= RS274NGC_MIN_ERROR) and
            (error_code <= RS274NGC_MAX_ERROR)) and
            (strlen(_rs274ngc_errors[error_code]) < max_size))
        {
            strcpy(error_text, _rs274ngc_errors[error_code]);
        }
        else
            error_text[0] = 0;
    }

   /***********************************************************************/

   /* rs274ngc_line_length

   Returned Value: the length of the most recently read line

   Side Effects: none

   Called By: external programs

   */

    int rs274ngc::line_length()
    {
        return _setup.line_length;
    }

   /***********************************************************************/

   /* rs274ngc_line_text

   Returned Value: none

   Side Effects: See below

   Called By: external programs

   This copies at most (max_size - 1) non-null characters of the most
   recently read line into the line_text string and puts a NULL after the
   last non-null character.

   */

    void rs274ngc::line_text(                      /* ARGUMENTS                            */
    char * line_text,                             /* string: to copy line into            */
    size_t max_size)                                 /* maximum number of characters to copy */
    {
        size_t n;
        char * the_text;

        the_text = _setup.linetext;
        for (n = 0; n < (max_size - 1); n++)
        {
            if (the_text[n] != 0)
                line_text[n] = the_text[n];
            else
                break;
        }
        line_text[n] = 0;
    }

   /***********************************************************************/

   /* rs274ngc_sequence_number

   Returned Value: the current interpreter sequence number (how many
   lines read since the last time the sequence number was reset to zero,
   which happens only when rs274ngc_init or rs274ngc_open is called).

   Side Effects: none

   Called By: external programs

   */

    int rs274ngc::sequence_number()
    {
        return _setup.sequence_number;
    }

   /***********************************************************************/
   /***********************************************************************/
   /* end of file */
