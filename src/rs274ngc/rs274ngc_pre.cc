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

#include <cstring>
#include <cctype>
#include <algorithm>
#include "rs274ngc.hh"
#include "rs274ngc_return.hh"
#include "codes.h"
#include "error.h"
#include "arc.h"

   // name of parameter file for saving/restoring interpreter variables
#define RS274NGC_PARAMETER_FILE_NAME_DEFAULT "rs274ngc.var"
#define RS274NGC_PARAMETER_FILE_BACKUP_SUFFIX ".bak"

//#define DEBUG_EMC

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

    static const unsigned int _required_parameters[] =
    {
        5161, 5162, 5163,                     /* G28 home */
        5164, 5165, 5166,
        
        5181, 5182, 5183,                     /* G30 home */
        5184, 5185, 5186,
        
        5211, 5212, 5213,                     /* G92 offsets */
        5214, 5215, 5216,
        
        5220,                                 /* selected coordinate */
        
        5221, 5222, 5223,                     /* coordinate system 1 */
        5224, 5225, 5226,
        
        5241, 5242, 5243,                     /* coordinate system 2 */
        5244, 5245, 5246,
        
        5261, 5262, 5263,                     /* coordinate system 3 */
        5264, 5265, 5266,
        
        5281, 5282, 5283,                     /* coordinate system 4 */
        5284, 5285, 5286,
        
        5301, 5302, 5303,                     /* coordinate system 5 */
        5304, 5305, 5306,
        
        5321, 5322, 5323,                     /* coordinate system 6 */
        5324, 5325, 5326,
        
        5341, 5342, 5343,                     /* coordinate system 7 */
        5344, 5345, 5346,
        
        5361, 5362, 5363,                     /* coordinate system 8 */
        5364, 5365, 5366,
        
        5381, 5382, 5383,                     /* coordinate system 9 */
        5384, 5385, 5386,
        
        RS274NGC_MAX_PARAMETERS
    };

rs274ngc::rs274ngc()
{
}

   /****************************************************************************/
   /****************************************************************************/

   /*

   The functions in this section are the interpreter kernel functions

   */

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
        for (n = 0, m = 0; (item = line[m]) != 0; m++)
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
        Position end;

        ijk_flag = (block.i or block.j or block.k) ? ON : OFF;
        first = (settings.program_x == UNKNOWN);

        error_if(!block.r and (ijk_flag != ON), NCE_R_I_J_K_WORDS_ALL_MISSING_FOR_ARC);
        error_if(block.r and (ijk_flag == ON), NCE_MIXED_RADIUS_IJK_FORMAT_FOR_ARC);
        if (settings.feed_mode == FeedMode::UnitsPerMinute)
        {
            error_if(settings.feed_rate == 0.0, NCE_CANNOT_MAKE_ARC_WITH_ZERO_FEED_RATE);
        }
        else if (settings.feed_mode == FeedMode::InverseTime)
        {
            error_if(!block.f, NCE_F_WORD_MISSING_WITH_INVERSE_TIME_ARC_MOVE);
        }
        if (ijk_flag)
        {
            if (settings.plane == Plane::XY)
            {
                error_if(!!block.k, NCE_K_WORD_GIVEN_FOR_ARC_IN_XY_PLANE);
                if (!block.i)         /* i or j flag on to get here */
                    block.i = 0.0;
                else if (!block.j)
                    block.j = 0.0;
            }
            else if (settings.plane == Plane::YZ)
            {
                error_if(!!block.i, NCE_I_WORD_GIVEN_FOR_ARC_IN_YZ_PLANE);
                if (!block.j)         /* j or k flag on to get here */
                    block.j = 0.0;
                else if (!block.k)
                    block.k = 0.0;
            }
            else if (settings.plane == Plane::XZ)
            {
                error_if(!!block.j, NCE_J_WORD_GIVEN_FOR_ARC_IN_XZ_PLANE);
                if (!block.i)         /* i or k flag on to get here */
                    block.i = 0.0;
                else if (!block.k)
                    block.k = 0.0;
            }
            else
                throw error(NCE_BUG_PLANE_NOT_XY_YZ_OR_XZ);
        }
        else
        {
             /* r format arc; no other checks needed specific to this format */
        }

        if (settings.plane == Plane::XY)    /* checks for both formats */
        {
            error_if(!block.x and !block.y, NCE_X_AND_Y_WORDS_MISSING_FOR_ARC_IN_XY_PLANE);
        }
        else if (settings.plane == Plane::YZ)
        {
            error_if(!block.y and !block.z, NCE_Y_AND_Z_WORDS_MISSING_FOR_ARC_IN_YZ_PLANE);
        }
        else if (settings.plane == Plane::XZ)
        {
            error_if(!block.x and !block.z, NCE_X_AND_Z_WORDS_MISSING_FOR_ARC_IN_XZ_PLANE);
        }

        find_ends(block, settings, &end.x, &end.y, &end.z, &end.a, &end.b, &end.c);
        settings.motion_mode = move;

        if (settings.plane == Plane::XY)
        {
            if ((settings.cutter_comp_side == Side::Off) or (settings.cutter_comp_radius == 0.0))
            {
                    convert_arc2(move, block, settings,
                    &(settings.current.x), &(settings.current.y), &(settings.current.z), 
                    end.x, end.y, end.z, 
                    end.a, end.b, end.c, 
                    *block.i, *block.j);
            }
            else if (first)
            {
                convert_arc_comp1(move, block, settings, end);
            }
            else
            {
                convert_arc_comp2(move, block, settings, end);
            }
        }
        else if (settings.plane == Plane::XZ)
        {
            convert_arc2 (move, block, settings,
	            &(settings.current.z), &(settings.current.x), &(settings.current.y), 
	            end.z, end.x, end.y, 
	            end.a, end.b, end.c, 
	            *block.k, *block.i);
        }
        else if (settings.plane == Plane::YZ)
        {
            convert_arc2 (move, block, settings,
	            &(settings.current.y), &(settings.current.z), &(settings.current.x), 
	            end.y, end.z, end.x, 
	            end.a, end.b, end.c, 
	            *block.j, *block.k);
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
    double AA_end,                                /* a-value at end of arc                    */
    double BB_end,                                /* b-value at end of arc                    */
    double CC_end,                                /* c-value at end of arc                    */
    double offset1,                               /* offset of center from current1           */
    double offset2)                               /* offset of center from current2           */
    {
        double center1;
        double center2;
        double tolerance;                         /* tolerance for difference of radii          */
        int turn;                                 /* number of full or partial turns CCW in arc */

        tolerance = (settings.length_units == Units::Imperial) ? TOLERANCE_INCH : TOLERANCE_MM;

        if (block.r)
        {
            arc_data_r(move, *current1, *current2, end1, end2, *block.r, &center1, &center2, &turn);
        }
        else
        {
            arc_data_ijk(move, *current1, *current2, end1, end2, offset1, offset2, &center1, &center2, &turn, tolerance);
        }

        if (settings.feed_mode == FeedMode::InverseTime)
            inverse_time_rate_arc(*current1, *current2, *current3, center1, center2, turn, end1, end2, end3, block, settings);
        arc(end1, end2, center1, center2, turn, end3, AA_end, BB_end, CC_end);
        *current1 = end1;
        *current2 = end2;
        *current3 = end3;
        settings.current.a = AA_end;
        settings.current.b = BB_end;
        settings.current.c = CC_end;
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
    Position end
    )
    {
        double center_x;
        double center_y;
        double gamma;                             /* direction of perpendicular to arc at end */
        Side side;                                 /* offset side - right or left              */
        double tolerance;                         /* tolerance for difference of radii        */
        double tool_radius;
        int turn;                                 /* 1 for counterclockwise, -1 for clockwise */

        side = settings.cutter_comp_side;
   /* always is positive */
        tool_radius = settings.cutter_comp_radius;
        tolerance = (settings.length_units == Units::Imperial) ? TOLERANCE_INCH : TOLERANCE_MM;

        error_if((hypot((end.x - settings.current.x), (end.y - settings.current.y)) <= tool_radius), NCE_CUTTER_GOUGING_WITH_CUTTER_RADIUS_COMP);

        if (block.r)
        {
            arc_data_comp_r(move, side, tool_radius, settings.current.x, settings.current.y, end.x, end.y, *block.r, &center_x, &center_y, &turn);
        }
        else
        {
            arc_data_comp_ijk(move, side, tool_radius, settings.current.x, settings.current.y, end.x, end.y, *block.i, *block.j, &center_x, &center_y, &turn, tolerance);
        }

        gamma =
            (((side == Side::Left) and (move == G_3)) or
            ((side == Side::Right) and (move == G_2))) ?
            atan2 ((center_y - end.y), (center_x - end.x)) :
        atan2 ((end.y - center_y), (end.x - center_x));

        settings.program_x = end.x;
        settings.program_y = end.y;
   /* end.x reset actual */
        end.x = (end.x + (tool_radius * cos(gamma)));
   /* end.y reset actual */
        end.y = (end.y + (tool_radius * sin(gamma)));

        if (settings.feed_mode == FeedMode::InverseTime)
            inverse_time_rate_arc(settings.current.x, settings.current.y,
                settings.current.z, center_x, center_y, turn,
                end.x, end.y, end.z, block, settings);
        arc(end.x, end.y, center_x, center_y, turn, end.z, end.a, end.b, end.c);
        settings.current = end;
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
    Position end
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
        Side side;
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
        tolerance = (settings.length_units == Units::Imperial) ? TOLERANCE_INCH : TOLERANCE_MM;

        if (block.r)
        {
            arc_data_r(move, start_x, start_y, end.x, end.y, *block.r, &center_x, &center_y, &turn);
        }
        else
        {
            arc_data_ijk(move, start_x, start_y, end.x, end.y, *block.i, *block.j, &center_x, &center_y, &turn, tolerance);
        }

   /* compute other data */
        side = settings.cutter_comp_side;
   /* always is positive */
        tool_radius = settings.cutter_comp_radius;
        arc_radius = hypot((center_x - end.x), (center_y - end.y));
        theta = atan2(settings.current.y - start_y, settings.current.x - start_x);
        theta = (side == Side::Left) ? (theta - PI2) : (theta + PI2);
        delta = atan2(center_y - start_y, center_x - start_x);
        alpha = (move == G_3) ? (delta - PI2) : (delta + PI2);
        beta = (side == Side::Left) ? (theta - alpha) : (alpha - theta);
        beta = (beta > (1.5 * PI))  ? (beta - TWO_PI) :
        (beta < -PI2) ? (beta + TWO_PI) : beta;

        if (((side == Side::Left)  and (move == G_3)) or ((side == Side::Right) and (move == G_2)))
        {
            gamma = atan2 ((center_y - end.y), (center_x - end.x));
            error_if(arc_radius <= tool_radius, NCE_TOOL_RADIUS_NOT_LESS_THAN_ARC_RADIUS_WITH_COMP);
        }
        else
        {
            gamma = atan2 ((end.y - center_y), (end.x - center_x));
            delta = (delta + PI);
        }

        settings.program_x = end.x;
        settings.program_y = end.y;
   /* end.x reset actual */
        end.x = (end.x + (tool_radius * cos(gamma)));
   /* end.y reset actual */
        end.y = (end.y + (tool_radius * sin(gamma)));

   /* check if extra arc needed and insert if so */

        error_if(((beta < -small) or (beta > (PI + small))), NCE_CONCAVE_CORNER_WITH_CUTTER_RADIUS_COMP);
        if (beta > small)                         /* two arcs needed */
        {
            mid_x = (start_x + (tool_radius * cos(delta)));
            mid_y = (start_y + (tool_radius * sin(delta)));
            if (settings.feed_mode == FeedMode::InverseTime)
                inverse_time_rate_arc2(start_x, start_y, (side == Side::Left) ? -1 : 1,
                mid_x, mid_y, center_x, center_y, turn,
                end.x, end.y, end.z, block, settings);
            arc(mid_x, mid_y, start_x, start_y, ((side == Side::Left) ? -1 : 1), settings.current.z, end.a, end.b, end.c);
            arc(end.x, end.y, center_x, center_y, turn, end.z, end.a, end.b, end.c);
        }
        else                                      /* one arc needed */
        {
            if (settings.feed_mode == FeedMode::InverseTime)
                inverse_time_rate_arc(settings.current.x, settings.current.y,
                    settings.current.z, center_x, center_y, turn,
                    end.x, end.y, end.z, block, settings);
            arc(end.x, end.y, center_x, center_y, turn, end.z, end.a, end.b, end.c);
        }

        settings.current = end;
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

        error_if(settings.cutter_comp_side != Side::Off, NCE_CANNOT_CHANGE_AXIS_OFFSETS_WITH_CUTTER_RADIUS_COMP);
        pars = settings.parameters;
        if (g_code == G_92)
        {
            if (block.x)
            {
                settings.axis_offset.x = (settings.current.x + settings.axis_offset.x - *block.x);
                settings.current.x = *block.x;
            }

            if (block.y)
            {
                settings.axis_offset.y = (settings.current.y + settings.axis_offset.y - *block.y);
                settings.current.y = *block.y;
            }

            if (block.z)
            {
                settings.axis_offset.z = (settings.current.z + settings.axis_offset.z - *block.z);
                settings.current.z = *block.z;
            }

            if (block.a)
            {
                settings.axis_offset.a = (settings.current.a + settings.axis_offset.a - *block.a);
                settings.current.a = *block.a;
            }

            if (block.b)
            {
                settings.axis_offset.b = (settings.current.b + settings.axis_offset.b - *block.b);
                settings.current.b = *block.b;
            }

            if (block.c)
            {
                settings.axis_offset.c = (settings.current.c + settings.axis_offset.c - *block.c);
                settings.current.c = *block.c;
            }

            offset_origin(settings.origin_offset + settings.axis_offset);
            pars[5211] = settings.axis_offset.x;
            pars[5212] = settings.axis_offset.y;
            pars[5213] = settings.axis_offset.z;
            pars[5214] = settings.axis_offset.a;
            pars[5215] = settings.axis_offset.b;
            pars[5216] = settings.axis_offset.c;
        }
        else if ((g_code == G_92_1) or (g_code == G_92_2))
        {
	        settings.current = settings.current + settings.axis_offset;
            offset_origin(settings.origin_offset);
            settings.axis_offset = Position{};
            if (g_code == G_92_1)
            {
                pars[5211] = 0.0;
                pars[5212] = 0.0;
                pars[5213] = 0.0;
                pars[5214] = 0.0;
                pars[5215] = 0.0;
                pars[5216] = 0.0;
            }
        }
        else if (g_code == G_92_3)
        {
        	Position param_pos {pars[5211], pars[5212], pars[5213], pars[5214], pars[5215], pars[5216]};
            settings.current = settings.current + settings.axis_offset - param_pos;
            settings.axis_offset = param_pos;
            offset_origin(settings.origin_offset + settings.axis_offset);
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
            this->comment(comment);
            return;
        }
        for (m++; ((item = comment[m]) == ' ') or (item == '\t') ; m++);
        if ((item != 'S') and (item != 's'))
        {
            this->comment(comment);
            return;
        }
        for (m++; ((item = comment[m]) == ' ') or (item == '\t') ; m++);
        if ((item != 'G') and (item != 'g'))
        {
            this->comment(comment);
            return;
        }
        for (m++; ((item = comment[m]) == ' ') or (item == '\t') ; m++);
        if (item != ',')
        {
            this->comment(comment);
            return;
        }
        message(comment + m + 1);
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
   control mode (Motion::Exact_Stop, Motion::Exact_Path or CANON_CONTINUOUS).

   A call is made to motion_mode(CANON_XXX), where CANON_XXX is
   Motion::Exact_Path if g_code is G_61, Motion::Exact_Stop if g_code is G_61_1,
   and CANON_CONTINUOUS if g_code is G_64.

   Setting the control mode to Motion::Exact_Stop on G_61 would correspond
   more closely to the meaning of G_61 as given in [NCMS, page 40], but
   Motion::Exact_Path has the advantage that the tool does not stop if it
   does not have to, and no evident disadvantage compared to
   Motion::Exact_Stop, so it is being used for G_61. G_61_1 is not defined
   in [NCMS], so it is available and is used here for setting the control
   mode to Motion::Exact_Stop.

   It is OK to call motion_mode(CANON_XXX) when CANON_XXX is
   already in force.

   */

    void rs274ngc::convert_control_mode(              /* ARGUMENTS                             */
    int g_code,                                   /* g_code being executed (G_61, G61_1, or G_64) */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        if (g_code == G_61)
        {
            motion_mode(Motion::Exact_Path);
            settings.control_mode = Motion::Exact_Path;
        }
        else if (g_code == G_61_1)
        {
            motion_mode(Motion::Exact_Stop);
            settings.control_mode = Motion::Exact_Stop;
        }
        else if (g_code == G_64)
        {
            motion_mode(Motion::Continuous);
            settings.control_mode = Motion::Continuous;
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
   coordinate system is used in a offset_origin function call and
   origin_index in the machine model is set to 1.

   If a g_code in the range g54 - g59.3 is encountered in an NC program,
   the data from the appropriate NGC coordinate system is copied into the
   origin offsets used by the interpreter, a offset_origin function
   call is made, and the current position is reset.

   If a g10 is encountered, the convert_setup function is called to reset
   the offsets of the program coordinate system indicated by the P number
   given in the same block.

   If a g53 is encountered, the axis values given in that block are used
   to calculate what the coordinates are of that point in the current
   coordinate system, and a rapid or linear function
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
        Position pos;
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
            comment("interpreter: continuing to use same coordinate system");
#endif
            return;
        }

        settings.origin_index = origin;
        parameters[5220] = (double)origin;

   /* axis offsets could be included in the two set of calculations for
      current_x, current_y, etc., but do not need to be because the results
      would be the same. They would be added in then subtracted out. */
      settings.current = settings.current + settings.origin_offset;

        pos.x = parameters[5201 + (origin * 20)];
        pos.y = parameters[5202 + (origin * 20)];
        pos.z = parameters[5203 + (origin * 20)];
        pos.a = parameters[5204 + (origin * 20)];
        pos.b = parameters[5205 + (origin * 20)];
        pos.c = parameters[5206 + (origin * 20)];

        settings.origin_offset = pos;

        settings.current = settings.current - pos;

        offset_origin(pos + settings.axis_offset);
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
            convert_cutter_compensation_on(Side::Left, block, settings);
        }
        else if (g_code == G_42)
        {
            convert_cutter_compensation_on(Side::Right, block, settings);
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
        comment("interpreter: cutter radius compensation off");
#endif
        settings.cutter_comp_side = Side::Off;
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
    Side side,                                     /* side of path cutter is on (LEFT or RIGHT) */
    block_t& block,                          /* pointer to a block of RS274 instructions  */
    setup_t& settings)                       /* pointer to machine settings               */
    {
        double radius;
        int index;

        error_if(settings.plane != Plane::XY, NCE_CANNOT_TURN_CUTTER_RADIUS_COMP_ON_OUT_OF_XY_PLANE);
        error_if(settings.cutter_comp_side != Side::Off, NCE_CANNOT_TURN_CUTTER_RADIUS_COMP_ON_WHEN_ON);
        index = block.d ? *block.d : settings.current_slot;
        radius = ((settings.tool_table[index].diameter)/2.0);

        if (radius < 0.0)                         /* switch side & make radius positive if radius negative */
        {
            radius = -radius;
            if (side == Side::Right)
                side = Side::Left;
            else
                side = Side::Right;
        }

#ifdef DEBUG_EMC
        if (side == RIGHT)
            comment("interpreter: cutter radius compensation on right");
        else
            comment("interpreter: cutter radius compensation on left");
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
        Plane plane;

        plane = settings.plane;
        if (!block.r)
        {
            if (settings.motion_mode == motion)
                block.r = settings.cycle.r;
            else
                throw error(NCE_R_CLEARANCE_PLANE_UNSPECIFIED_IN_CYCLE);
        }

        if(!block.l)
            block.l = 1;
        error_if(*block.l == 0, NCE_CANNOT_DO_ZERO_REPEATS_OF_CYCLE);

        if (plane == Plane::XY)
        {
            convert_cycle_xy(motion, block, settings);
        }
        else if (plane == Plane::YZ)
        {
            convert_cycle_yz(motion, block, settings);
        }
        else if (plane == Plane::XZ)
        {
            convert_cycle_zx(motion, block, settings);
        }
        else
            throw error(NCE_BUG_PLANE_NOT_XY_YZ_OR_XZ);

        settings.cycle.l = *block.l;
        settings.cycle.r = *block.r;
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
    Plane plane,                            /* selected plane                   */
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
    Plane plane,                            /* selected plane                   */
    double x,                                     /* x-value where cycle is executed  */
    double y,                                     /* y-value where cycle is executed  */
    double clear_z,                               /* z-value of clearance plane       */
    double bottom_z,                              /* value of z at bottom of cycle    */
    double dwell)                                 /* dwell time                       */
    {
        cycle_feed(plane, x, y, bottom_z);
        this->dwell(dwell);
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
    Plane plane,                            /* selected plane                   */
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
        if (_setup.length_units == Units::Metric)
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
    Plane plane,                            /* selected plane                      */
    double x,                                     /* x-value where cycle is executed     */
    double y,                                     /* y-value where cycle is executed     */
    double clear_z,                               /* z-value of clearance plane          */
    double bottom_z,                              /* value of z at bottom of cycle       */
    Direction direction,                    /* direction spindle turning at outset */
    SpeedFeedMode mode)                   /* the speed-feed mode at outset       */
    {
        error_if(direction != Direction::Clockwise, NCE_SPINDLE_NOT_TURNING_CLOCKWISE_IN_G84);
        speed_feed_sync_start();
        cycle_feed(plane, x, y, bottom_z);
        spindle_stop();
        spindle_start_counterclockwise();
        cycle_feed(plane, x, y, clear_z);
        if (mode != SpeedFeedMode::Synched)
            speed_feed_sync_stop();
        spindle_stop();
        spindle_start_clockwise();
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
    Plane plane,                            /* selected plane                   */
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
    Plane plane,                            /* selected plane                      */
    double x,                                     /* x-value where cycle is executed     */
    double y,                                     /* y-value where cycle is executed     */
    double clear_z,                               /* z-value of clearance plane          */
    double bottom_z,                              /* value of z at bottom of cycle       */
    double dwell,                                 /* dwell time                          */
    Direction direction)                    /* direction spindle turning at outset */
    {
        error_if((direction != Direction::Clockwise) and (direction != Direction::CounterClockwise), NCE_SPINDLE_NOT_TURNING_IN_G86);

        cycle_feed(plane, x, y, bottom_z);
        this->dwell(dwell);
        spindle_stop();
        cycle_traverse(plane, x, y, clear_z);
        if (direction == Direction::Clockwise)
            spindle_start_clockwise();
        else
            spindle_start_counterclockwise();
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
    Plane plane,                            /* selected plane                      */
    double x,                                     /* x-value where cycle is executed     */
    double offset_x,                              /* x-axis offset position              */
    double y,                                     /* y-value where cycle is executed     */
    double offset_y,                              /* y-axis offset position              */
    double r,                                     /* z_value of r_plane                  */
    double clear_z,                               /* z-value of clearance plane          */
    double middle_z,                              /* z-value of top of back bore         */
    double bottom_z,                              /* value of z at bottom of cycle       */
    Direction direction)                    /* direction spindle turning at outset */
    {
        error_if((direction != Direction::Clockwise) and (direction != Direction::CounterClockwise), NCE_SPINDLE_NOT_TURNING_IN_G87);

        cycle_traverse(plane, offset_x, offset_y, r);
        spindle_stop();
        spindle_orient(0.0, direction);
        cycle_traverse(plane, offset_x, offset_y, bottom_z);
        cycle_traverse(plane, x, y, bottom_z);
        if (direction == Direction::Clockwise)
            spindle_start_clockwise();
        else
            spindle_start_counterclockwise();
        cycle_feed(plane, x, y, middle_z);
        cycle_feed(plane, x, y, bottom_z);
        spindle_stop();
        spindle_orient(0.0, direction);
        cycle_traverse(plane, offset_x, offset_y, bottom_z);
        cycle_traverse(plane, offset_x, offset_y, clear_z);
        cycle_traverse(plane, x, y, clear_z);
        if (direction == Direction::Clockwise)
            spindle_start_clockwise();
        else
            spindle_start_counterclockwise();
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
    Plane plane,                            /* selected plane                      */
    double x,                                     /* x-value where cycle is executed     */
    double y,                                     /* y-value where cycle is executed     */
    double bottom_z,                              /* value of z at bottom of cycle       */
    double dwell,                                 /* dwell time                          */
    Direction direction)                    /* direction spindle turning at outset */
    {
        error_if((direction != Direction::Clockwise) and (direction != Direction::CounterClockwise), NCE_SPINDLE_NOT_TURNING_IN_G88);

        cycle_feed(plane, x, y, bottom_z);
        this->dwell(dwell);
        spindle_stop();
        program_stop();                           /* operator retracts the spindle here */
        if (direction == Direction::Clockwise)
            spindle_start_clockwise();
        else
            spindle_start_counterclockwise();
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
    Plane plane,                            /* selected plane                   */
    double x,                                     /* x-value where cycle is executed  */
    double y,                                     /* y-value where cycle is executed  */
    double clear_z,                               /* z-value of clearance plane       */
    double bottom_z,                              /* value of z at bottom of cycle    */
    double dwell)                                 /* dwell time                       */
    {
        cycle_feed(plane, x, y, bottom_z);
        this->dwell(dwell);
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

#define CYCLE_MACRO(call) for (int repeat = *block.l; repeat > 0; repeat--) \
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
        Plane plane;
        double r;
        Motion save_mode;

        plane = Plane::XY;
        if (settings.motion_mode != motion)
        {
            error_if(!block.z, NCE_Z_VALUE_UNSPECIFIED_IN_XY_PLANE_CANNED_CYCLE);
        }
        block.z = block.z ? *block.z : settings.cycle.cc;
        old_cc = settings.current.z;

        if (settings.distance_mode == DistanceMode::Absolute)
        {
            aa_increment = 0.0;
            bb_increment = 0.0;
            r = *block.r;
            cc = *block.z;
            aa = block.x ? *block.x : settings.current.x;
            bb = block.y ? *block.y : settings.current.y;
        }
        else if (settings.distance_mode == DistanceMode::Incremental)
        {
            aa_increment = *block.x;
            bb_increment = *block.y;
            r = (*block.r + old_cc);
            cc = (r + *block.z);      /* [NCMS, page 98] */
            aa = settings.current.x;
            bb = settings.current.y;
        }
        else
            throw error(NCE_BUG_DISTANCE_MODE_NOT_G90_OR_G91);
        error_if(r < cc, NCE_R_LESS_THAN_Z_IN_CYCLE_IN_XY_PLANE);

        if (old_cc < r)
        {
            rapid({settings.current.x, settings.current.y, r, settings.current.a, settings.current.b, settings.current.c});
            old_cc = r;
        }
        clear_cc = (settings.retract_mode == R_PLANE) ? r : old_cc;

        save_mode = motion_mode();
        if (save_mode != Motion::Exact_Path)
            motion_mode(Motion::Exact_Path);

        switch(motion)
        {
            case G_81:
                CYCLE_MACRO(convert_cycle_g81(Plane::XY, aa, bb, clear_cc, cc))
                break;
            case G_82:
                error_if((settings.motion_mode != G_82) and !block.p, NCE_DWELL_TIME_P_WORD_MISSING_WITH_G82);
                block.p = !block.p ? settings.cycle.p : *block.p;
                CYCLE_MACRO(convert_cycle_g82 (Plane::XY, aa, bb, clear_cc, cc, *block.p))
                settings.cycle.p = *block.p;
                break;
            case G_83:
                error_if((settings.motion_mode != G_83) and !block.q, NCE_Q_WORD_MISSING_WITH_G83);
                block.q = !block.q ? settings.cycle.q : *block.q;
                CYCLE_MACRO(convert_cycle_g83 (Plane::XY, aa, bb, r, clear_cc, cc, *block.q))
                settings.cycle.q = *block.q;
                break;
            case G_84:
                CYCLE_MACRO(convert_cycle_g84 (Plane::XY, aa, bb, clear_cc, cc, settings.spindle_turning, settings.speed_feed_mode))
                    break;
            case G_85:
                CYCLE_MACRO(convert_cycle_g85 (Plane::XY, aa, bb, clear_cc, cc))
                    break;
            case G_86:
                error_if((settings.motion_mode != G_86) and (!block.p), NCE_DWELL_TIME_P_WORD_MISSING_WITH_G86);
                block.p = !block.p ? settings.cycle.p : *block.p;
                CYCLE_MACRO(convert_cycle_g86 (Plane::XY, aa, bb, clear_cc, cc, *block.p, settings.spindle_turning))
                settings.cycle.p = *block.p;
                break;
            case G_87:
                if (settings.motion_mode != G_87)
                {
                    error_if(!block.i, NCE_I_WORD_MISSING_WITH_G87);
                    error_if(!block.j, NCE_J_WORD_MISSING_WITH_G87);
                    error_if(!block.k, NCE_K_WORD_MISSING_WITH_G87);
                }
                i = block.i ? *block.i : settings.cycle.i;
                j = block.j ? *block.j : settings.cycle.j;
                k = block.k ? *block.k : settings.cycle.k;
                settings.cycle.i = i;
                settings.cycle.j = j;
                settings.cycle.k = k;
                if (settings.distance_mode == DistanceMode::Incremental)
                {
                    k = (cc + k);            /* k always absolute in function call below */
                }
                CYCLE_MACRO(convert_cycle_g87 (Plane::XY, aa, (aa + i), bb, (bb + j), r, clear_cc, k, cc, settings.spindle_turning))
                    break;
            case G_88:
                error_if((settings.motion_mode != G_88) and !block.p, NCE_DWELL_TIME_P_WORD_MISSING_WITH_G88);
                block.p = !block.p ? settings.cycle.p : *block.p;
                CYCLE_MACRO(convert_cycle_g88 (Plane::XY, aa, bb, cc, *block.p, settings.spindle_turning))
                settings.cycle.p = *block.p;
                break;
            case G_89:
                error_if((settings.motion_mode != G_89) and !block.p, NCE_DWELL_TIME_P_WORD_MISSING_WITH_G89);
                block.p = !block.p ? settings.cycle.p : *block.p;
                CYCLE_MACRO(convert_cycle_g89 (Plane::XY, aa, bb, clear_cc, cc, *block.p))
                settings.cycle.p = *block.p;
                break;
            default:
                throw error(NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        }
        settings.current.x = aa;            /* CYCLE_MACRO updates aa and bb */
        settings.current.y = bb;
        settings.current.z = clear_cc;
        settings.cycle.cc = *block.z;

        if (save_mode != Motion::Exact_Path)
            motion_mode(save_mode);
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
        Plane plane;
        double r;
        Motion save_mode;

        plane = Plane::YZ;
        if (settings.motion_mode != motion)
        {
            error_if(!block.x, NCE_X_VALUE_UNSPECIFIED_IN_YZ_PLANE_CANNED_CYCLE);
        }
        block.x = block.x ? *block.x : settings.cycle.cc;
        old_cc = settings.current.x;

        if (settings.distance_mode == DistanceMode::Absolute)
        {
            aa_increment = 0.0;
            bb_increment = 0.0;
            r = *block.r;
            cc = *block.x;
            aa = block.y ? *block.y : settings.current.y;
            bb = block.z ? *block.z : settings.current.z;
        }
        else if (settings.distance_mode == DistanceMode::Incremental)
        {
            aa_increment = *block.y;
            bb_increment = *block.z;
            r = (*block.r + old_cc);
            cc = (r + *block.x);      /* [NCMS, page 98] */
            aa = settings.current.y;
            bb = settings.current.z;
        }
        else
            throw error(NCE_BUG_DISTANCE_MODE_NOT_G90_OR_G91);
        error_if(r < cc, NCE_R_LESS_THAN_X_IN_CYCLE_IN_YZ_PLANE);

        if (old_cc < r)
        {
            rapid({r, settings.current.y, settings.current.z, settings.current.a, settings.current.b, settings.current.c});
            old_cc = r;
        }
        clear_cc = (settings.retract_mode == R_PLANE) ? r : old_cc;

        save_mode = motion_mode();
        if (save_mode != Motion::Exact_Path)
            motion_mode(Motion::Exact_Path);

        switch(motion)
        {
            case G_81:
                CYCLE_MACRO(convert_cycle_g81(Plane::YZ, aa, bb, clear_cc, cc))
                break;
            case G_82:
                error_if((settings.motion_mode != G_82) and !block.p, NCE_DWELL_TIME_P_WORD_MISSING_WITH_G82);
                block.p = !block.p ? settings.cycle.p : *block.p;
                CYCLE_MACRO(convert_cycle_g82 (Plane::YZ, aa, bb, clear_cc, cc, *block.p))
                settings.cycle.p = *block.p;
                break;
            case G_83:
                error_if((settings.motion_mode != G_83) and !block.q, NCE_Q_WORD_MISSING_WITH_G83);
                block.q = !block.q ? settings.cycle.q : *block.q;
                CYCLE_MACRO(convert_cycle_g83 (Plane::YZ, aa, bb, r, clear_cc, cc, *block.q))
                settings.cycle.q = *block.q;
                break;
            case G_84:
                CYCLE_MACRO(convert_cycle_g84 (Plane::YZ, aa, bb, clear_cc, cc, settings.spindle_turning, settings.speed_feed_mode))
                break;
            case G_85:
                CYCLE_MACRO(convert_cycle_g85 (Plane::YZ, aa, bb, clear_cc, cc))
                break;
            case G_86:
                error_if((settings.motion_mode != G_86) and !block.p, NCE_DWELL_TIME_P_WORD_MISSING_WITH_G86);
                block.p = !block.p ? settings.cycle.p : *block.p;
                CYCLE_MACRO(convert_cycle_g86 (Plane::YZ, aa, bb, clear_cc, cc, *block.p, settings.spindle_turning))
                settings.cycle.p = *block.p;
                break;
            case G_87:
                if (settings.motion_mode != G_87)
                {
                    error_if(!block.i, NCE_I_WORD_MISSING_WITH_G87);
                    error_if(!block.j, NCE_J_WORD_MISSING_WITH_G87);
                    error_if(!block.k, NCE_K_WORD_MISSING_WITH_G87);
                }
                i = block.i ? *block.i : settings.cycle.i;
                j = block.j ? *block.j : settings.cycle.j;
                k = block.k ? *block.k : settings.cycle.k;
                settings.cycle.i = i;
                settings.cycle.j = j;
                settings.cycle.k = k;
                if (settings.distance_mode == DistanceMode::Incremental)
                {
                    i = (cc + i);            /* i always absolute in function call below */
                }
                CYCLE_MACRO(convert_cycle_g87 (Plane::YZ, aa, (aa + j), bb, (bb + k), r, clear_cc, i, cc, settings.spindle_turning))
                    break;
            case G_88:
                error_if((settings.motion_mode != G_88) and !block.p, NCE_DWELL_TIME_P_WORD_MISSING_WITH_G88);
                block.p = !block.p ? settings.cycle.p : *block.p;
                CYCLE_MACRO(convert_cycle_g88 (Plane::YZ, aa, bb, cc, *block.p, settings.spindle_turning))
                settings.cycle.p = *block.p;
                break;
            case G_89:
                error_if((settings.motion_mode != G_89) and !block.p, NCE_DWELL_TIME_P_WORD_MISSING_WITH_G89);
                block.p = !block.p ? settings.cycle.p : *block.p;
                CYCLE_MACRO(convert_cycle_g89 (Plane::YZ, aa, bb, clear_cc, cc, *block.p))
                settings.cycle.p = *block.p;
                break;
            default:
                throw error(NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        }
        settings.current.y = aa;            /* CYCLE_MACRO updates aa and bb */
        settings.current.z = bb;
        settings.current.x = clear_cc;
        settings.cycle.cc = *block.x;

        if (save_mode != Motion::Exact_Path)
            motion_mode(save_mode);
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
   Plane::XZ.

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
        Plane plane;
        double r;
        Motion save_mode;

        plane = Plane::XZ;
        if (settings.motion_mode != motion)
        {
            error_if(!block.y, NCE_Y_VALUE_UNSPECIFIED_IN_XZ_PLANE_CANNED_CYCLE);
        }
        block.y = block.y ? *block.y : settings.cycle.cc;
        old_cc = settings.current.y;

        if (settings.distance_mode == DistanceMode::Absolute)
        {
            aa_increment = 0.0;
            bb_increment = 0.0;
            r = *block.r;
            cc = *block.y;
            aa = block.z ? *block.z : settings.current.z;
            bb = block.x ? *block.x : settings.current.x;
        }
        else if (settings.distance_mode == DistanceMode::Incremental)
        {
            aa_increment = *block.z;
            bb_increment = *block.x;
            r = (*block.r + old_cc);
            cc = (r + *block.y);      /* [NCMS, page 98] */
            aa = settings.current.z;
            bb = settings.current.x;
        }
        else
            throw error(NCE_BUG_DISTANCE_MODE_NOT_G90_OR_G91);
        error_if(r < cc, NCE_R_LESS_THAN_Y_IN_CYCLE_IN_XZ_PLANE);

        if (old_cc < r)
        {
            rapid({settings.current.x, r, settings.current.z, settings.current.a, settings.current.b, settings.current.c});
            old_cc = r;
        }
        clear_cc = (settings.retract_mode == R_PLANE) ? r : old_cc;

        save_mode = motion_mode();
        if (save_mode != Motion::Exact_Path)
            motion_mode(Motion::Exact_Path);

        switch(motion)
        {
            case G_81:
                CYCLE_MACRO(convert_cycle_g81(Plane::XZ, aa, bb, clear_cc, cc))
                break;
            case G_82:
                error_if((settings.motion_mode != G_82) and !block.p, NCE_DWELL_TIME_P_WORD_MISSING_WITH_G82);
                block.p = !block.p ? settings.cycle.p : *block.p;
                CYCLE_MACRO(convert_cycle_g82 (Plane::XZ, aa, bb, clear_cc, cc, *block.p))
                settings.cycle.p = *block.p;
                break;
            case G_83:
                error_if((settings.motion_mode != G_83) and !block.q, NCE_Q_WORD_MISSING_WITH_G83);
                block.q = !block.q ? settings.cycle.q : *block.q;
                CYCLE_MACRO(convert_cycle_g83 (Plane::XZ, aa, bb, r, clear_cc, cc, *block.q))
                settings.cycle.q = *block.q;
                break;
            case G_84:
                CYCLE_MACRO(convert_cycle_g84 (Plane::XZ, aa, bb, clear_cc, cc, settings.spindle_turning, settings.speed_feed_mode))
                break;
            case G_85:
                CYCLE_MACRO(convert_cycle_g85 (Plane::XZ, aa, bb, clear_cc, cc))
                break;
            case G_86:
                error_if((settings.motion_mode != G_86) and !block.p, NCE_DWELL_TIME_P_WORD_MISSING_WITH_G86);
                block.p = !block.p ? settings.cycle.p : *block.p;
                CYCLE_MACRO(convert_cycle_g86 (Plane::XZ, aa, bb, clear_cc, cc, *block.p, settings.spindle_turning))
                settings.cycle.p = *block.p;
                break;
            case G_87:
                if (settings.motion_mode != G_87)
                {
                    error_if(!block.i, NCE_I_WORD_MISSING_WITH_G87);
                    error_if(!block.j, NCE_J_WORD_MISSING_WITH_G87);
                    error_if(!block.k, NCE_K_WORD_MISSING_WITH_G87);
                }
                i = block.i ? *block.i : settings.cycle.i;
                j = block.j ? *block.j : settings.cycle.j;
                k = block.k ? *block.k : settings.cycle.k;
                settings.cycle.i = i;
                settings.cycle.j = j;
                settings.cycle.k = k;
                if (settings.distance_mode == DistanceMode::Incremental)
                {
                    j = (cc + j);            /* j always absolute in function call below */
                }
                CYCLE_MACRO(convert_cycle_g87 (Plane::XZ, aa, (aa + k), bb, (bb + i), r, clear_cc, j, cc, settings.spindle_turning))
                break;
            case G_88:
                error_if((settings.motion_mode != G_88) and !block.p, NCE_DWELL_TIME_P_WORD_MISSING_WITH_G88);
                block.p = !block.p ? settings.cycle.p : *block.p;
                CYCLE_MACRO(convert_cycle_g88 (Plane::XZ, aa, bb, cc, *block.p, settings.spindle_turning))
                settings.cycle.p = *block.p;
                break;
            case G_89:
                error_if((settings.motion_mode != G_89) and !block.p, NCE_DWELL_TIME_P_WORD_MISSING_WITH_G89);
                block.p = !block.p ? settings.cycle.p : *block.p;
                CYCLE_MACRO(convert_cycle_g89 (Plane::XZ, aa, bb, clear_cc, cc, *block.p))
                settings.cycle.p = *block.p;
                break;
            default:
                throw error(NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        }
        settings.current.z = aa;            /* CYCLE_MACRO updates aa and bb */
        settings.current.x = bb;
        settings.current.y = clear_cc;
        settings.cycle.cc = *block.y;

        if (save_mode != Motion::Exact_Path)
            motion_mode(save_mode);
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
            if (settings.distance_mode != DistanceMode::Absolute)
            {
#ifdef DEBUG_EMC
                comment("interpreter: distance mode changed to absolute");
#endif
                settings.distance_mode = DistanceMode::Absolute;
            }
        }
        else if (g_code == G_91)
        {
            if (settings.distance_mode != DistanceMode::Incremental)
            {
#ifdef DEBUG_EMC
                comment("interpreter: distance mode changed to incremental");
#endif
                settings.distance_mode = DistanceMode::Incremental;
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
        dwell(time);
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
            comment("interpreter: feed mode set to inverse time");
#endif
            settings.feed_mode = FeedMode::InverseTime;
        }
        else if (g_code == G_94)
        {
#ifdef DEBUG_EMC
            comment("interpreter: feed mode set to units per minute");
#endif
            settings.feed_mode = FeedMode::UnitsPerMinute;
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
        feed_rate(*block.f);
        settings.feed_rate = *block.f;
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
            convert_dwell(*block.p);
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
    	Position end;
        
        double AA_end2;
        double BB_end2;
        double CC_end2;
        double * parameters;

        parameters = settings.parameters;
        find_ends(block, settings, &end.x, &end.y, &end.z, &end.a, &end.b, &end.c);

        error_if(settings.cutter_comp_side != Side::Off, NCE_CANNOT_USE_G28_OR_G30_WITH_CUTTER_RADIUS_COMP);
        rapid(end);
        if (move == G_28)
        {
            find_relative(
            	parameters[5161], parameters[5162], parameters[5163],
                parameters[5164], parameters[5165], parameters[5166],
                &end.x, &end.y, &end.z, &AA_end2, &BB_end2, &CC_end2, 
                settings);
        }
        else if (move == G_30)
        {
            find_relative(
            	parameters[5181], parameters[5182], parameters[5183],
                parameters[5184], parameters[5185], parameters[5186],
                &end.x, &end.y, &end.z, &AA_end2, &BB_end2, &CC_end2, 
                settings);
        }
        else
            throw error(NCE_BUG_CODE_NOT_G28_OR_G30);
        
        rapid(end);
        settings.current.x = end.x;
        settings.current.y = end.y;
        settings.current.z = end.z;
        settings.current.a = AA_end2;
        settings.current.b = BB_end2;
        settings.current.c = CC_end2;
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
        error_if(settings.cutter_comp_side != Side::Off, NCE_CANNOT_CHANGE_UNITS_WITH_CUTTER_RADIUS_COMP);
        if (g_code == G_20)
        {
            units(Units::Imperial);
            if (settings.length_units != Units::Imperial)
            {
                settings.length_units = Units::Imperial;
                settings.current.x = (settings.current.x * INCH_PER_MM);
                settings.current.y = (settings.current.y * INCH_PER_MM);
                settings.current.z = (settings.current.z * INCH_PER_MM);
                settings.axis_offset.x = (settings.axis_offset.x * INCH_PER_MM);
                settings.axis_offset.y = (settings.axis_offset.y * INCH_PER_MM);
                settings.axis_offset.z = (settings.axis_offset.z * INCH_PER_MM);
                settings.origin_offset.x = (settings.origin_offset.x * INCH_PER_MM);
                settings.origin_offset.y = (settings.origin_offset.y * INCH_PER_MM);
                settings.origin_offset.z = (settings.origin_offset.z * INCH_PER_MM);
            }
        }
        else if (g_code == G_21)
        {
            units(Units::Metric);
            if (settings.length_units != Units::Metric)
            {
                settings.length_units = Units::Metric;
                settings.current.x = (settings.current.x * MM_PER_INCH);
                settings.current.y = (settings.current.y * MM_PER_INCH);
                settings.current.z = (settings.current.z * MM_PER_INCH);
                settings.axis_offset.x = (settings.axis_offset.x * MM_PER_INCH);
                settings.axis_offset.y = (settings.axis_offset.y * MM_PER_INCH);
                settings.axis_offset.z = (settings.axis_offset.z * MM_PER_INCH);
                settings.origin_offset.x = (settings.origin_offset.x * MM_PER_INCH);
                settings.origin_offset.y = (settings.origin_offset.y * MM_PER_INCH);
                settings.origin_offset.z = (settings.origin_offset.z * MM_PER_INCH);
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
            spindle_start_clockwise();
            settings.spindle_turning = Direction::Clockwise;
        }
        else if (block.m_modes[7] == 4)
        {
            spindle_start_counterclockwise();
            settings.spindle_turning = Direction::CounterClockwise;
        }
        else if (block.m_modes[7] == 5)
        {
            spindle_stop();
            settings.spindle_turning = Direction::Stop;
        }

        if (block.m_modes[8] == 7)
        {
            coolant_mist_on();
            settings.coolant.mist = ON;
        }
        else if (block.m_modes[8] == 8)
        {
            coolant_flood_on();
            settings.coolant.flood = ON;
        }
        else if (block.m_modes[8] == 9)
        {
            coolant_mist_off();
            settings.coolant.mist = OFF;
            coolant_flood_off();
            settings.coolant.flood = OFF;
        }

   /* No axis clamps in this version
     if (block.m_modes[2] == 26)
       {
   #ifdef DEBUG_EMC
   comment("interpreter: automatic A-axis clamping turned on");
   #endif
   settings.a_axis_clamping = ON;
   }
   else if (block.m_modes[2] == 27)
   {
   #ifdef DEBUG_EMC
   comment("interpreter: automatic A-axis clamping turned off");
   #endif
   settings.a_axis_clamping = OFF;
   }
   */

        if (block.m_modes[9] == 48)
        {
            feed_override_enable();
            speed_override_enable();
            settings.feed_override = ON;
            settings.speed_override = ON;
        }
        else if (block.m_modes[9] == 49)
        {
            feed_override_disable();
            speed_override_disable();
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
        else if ((code == G_92)   or (code == G_92_1) or (code == G_92_2) or (code == G_92_3))
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
            comment("interpreter: motion mode set to none");
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
        Position end;

        error_if(!block.x and !block.y and !block.z, NCE_X_Y_AND_Z_WORDS_ALL_MISSING_WITH_G38_2);
        error_if(settings.feed_mode == FeedMode::InverseTime, NCE_CANNOT_PROBE_IN_INVERSE_TIME_FEED_MODE);
        error_if(settings.cutter_comp_side != Side::Off, NCE_CANNOT_PROBE_WITH_CUTTER_RADIUS_COMP_ON);
        error_if(settings.feed_rate == 0.0, NCE_CANNOT_PROBE_WITH_ZERO_FEED_RATE);
        find_ends(block, settings, &end.x, &end.y, &end.z, &end.a, &end.b, &end.c);
        if ((end.a != settings.current.a)
            or (end.b != settings.current.b)
            or (end.c != settings.current.c)
            )
            throw error(NCE_CANNOT_MOVE_ROTARY_AXES_DURING_PROBING);
        distance = sqrt(pow((settings.current.x - end.x), 2) +
            pow((settings.current.y - end.y), 2) +
            pow((settings.current.z - end.z), 2));
        error_if((distance < ((settings.length_units == Units::Metric) ? 0.254 : 0.01)), NCE_START_POINT_TOO_CLOSE_TO_PROBE_POINT);
        probe_on();
        probe(end);
        probe_off();
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
            comment("interpreter: retract mode set to old_z");
#endif
            settings.retract_mode = OLD_Z;
        }
        else if (g_code == G_99)
        {
#ifdef DEBUG_EMC
            comment("interpreter: retract mode set to r_plane");
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
    	Position pos;
        double * parameters;
        int p_int;

        parameters = settings.parameters;
        p_int = (int)(*block.p + 0.0001);

        if (block.x)
        {
            pos.x = *block.x;
            parameters[5201 + (p_int * 20)] = pos.x;
        }
        else
            pos.x = parameters[5201 + (p_int * 20)];

        if (block.y)
        {
            pos.y = *block.y;
            parameters[5202 + (p_int * 20)] = pos.y;
        }
        else
            pos.y = parameters[5202 + (p_int * 20)];
        if (block.z)
        {
            pos.z = *block.z;
            parameters[5203 + (p_int * 20)] = pos.z;
        }
        else
            pos.z = parameters[5203 + (p_int * 20)];

        if (block.a)
        {
            pos.a = *block.a;
            parameters[5204 + (p_int * 20)] = pos.a;
        }
        else
            pos.a = parameters[5204 + (p_int * 20)];

        if (block.b)
        {
            pos.b = *block.b;
            parameters[5205 + (p_int * 20)] = pos.b;
        }
        else
            pos.b = parameters[5205 + (p_int * 20)];

        if (block.c)
        {
            pos.c = *block.c;
            parameters[5206 + (p_int * 20)] = pos.c;
        }
        else
            pos.c = parameters[5206 + (p_int * 20)];

   /* axis offsets could be included in the two sets of calculations for
      current_x, current_y, etc., but do not need to be because the results
      would be the same. They would be added in then subtracted out. */
        if (p_int == settings.origin_index)      /* system is currently used */
        {
            settings.current = settings.current + settings.origin_offset;
            settings.origin_offset = pos;
            settings.current = settings.current - pos;
            offset_origin(pos + settings.axis_offset);
        }
#ifdef DEBUG_EMC
        else
            comment("interpreter: setting coordinate system origin");
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
            plane(Plane::XY);
            settings.plane = Plane::XY;
        }
        else if (g_code == G_18)
        {
            error_if(settings.cutter_comp_side != Side::Off, NCE_CANNOT_USE_XZ_PLANE_WITH_CUTTER_RADIUS_COMP);
            plane(Plane::XZ);
            settings.plane = Plane::XZ;
        }
        else if (g_code == G_19)
        {
            error_if(settings.cutter_comp_side != Side::Off, NCE_CANNOT_USE_YZ_PLANE_WITH_CUTTER_RADIUS_COMP);
            plane(Plane::YZ);
            settings.plane = Plane::YZ;
        }
        else
            throw error(NCE_BUG_CODE_NOT_G17_G18_OR_G19);
    }

   /****************************************************************************/

   /* convert_speed

   Returned Value: int (RS274NGC_OK)

   Side effects:
   The machine spindle speed is set to the value of s_number in the
   block by a call to spindle_speed.
   The machine model for spindle speed is set to that value.

   Called by: execute_block.

   */

    void rs274ngc::convert_speed(                     /* ARGUMENTS                                */
    block_t& block,                          /* pointer to a block of RS274 instructions */
    setup_t& settings)                       /* pointer to machine settings              */
    {
        spindle_speed(*block.s);
        settings.speed = *block.s;
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

   For m0, m1, and m60, this makes a function call to the program_stop
   canonical machining function (which stops program execution).
   In addition, m60 calls pallet_shuttle.

   For m2 and m30, this resets the machine and then calls program_end.
   In addition, m30 calls pallet_shuttle.

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

   1. Axis offsets are set to zero (like g92.2) and      - offset_origin
   origin offsets are set to the default (like G54)
   2. Selected plane is set to Plane::XY (like G17) - plane
   3. Distance mode is set to MODE_ABSOLUTE (like G90)   - no canonical call
   4. Feed mode is set to UNITS_PER_MINUTE (like G94)    - no canonical call
   5. Feed and speed overrides are set to ON (like M48)  - feed_override_enable
   - speed_override_enable
   6. Cutter compensation is turned off (like G40)       - no canonical call
   7. The spindle is stopped (like M5)                   - spindle_stop
   8. The motion mode is set to G_1 (like G1)            - no canonical call
   9. Coolant is turned off (like M9)                    - coolant_flood_off & coolant_mist_off

   */

    int rs274ngc::convert_stop(                      /* ARGUMENTS                                    */
    block_t& block,                          /* pointer to a block of RS274/NGC instructions */
    setup_t& settings)                       /* pointer to machine settings                  */
    {
        if (block.m_modes[4] == 0)
        {
            program_stop();
        }
        else if (block.m_modes[4] == 60)
        {
            pallet_shuttle();
            program_stop();
        }
        else if (block.m_modes[4] == 1)
        {
            program_optional_stop();
        }
        else if ((block.m_modes[4] == 2) or (block.m_modes[4] == 30))
        {                                         /* reset stuff here */
   /*1*/
            settings.current = settings.current + settings.origin_offset + settings.axis_offset;

            settings.origin_index = 1;
            settings.parameters[5220] = 1.0;
            settings.origin_offset.x = settings.parameters[5221];
            settings.origin_offset.y = settings.parameters[5222];
            settings.origin_offset.z = settings.parameters[5223];
            settings.origin_offset.a = settings.parameters[5224];
            settings.origin_offset.b = settings.parameters[5225];
            settings.origin_offset.c = settings.parameters[5226];

            settings.axis_offset = Position{};
            settings.current = settings.current - settings.origin_offset;

            offset_origin(settings.origin_offset);

            /*2*/ if (settings.plane != Plane::XY)
            {
                plane(Plane::XY);
                settings.plane = Plane::XY;
            }

            /*3*/ settings.distance_mode = DistanceMode::Absolute;

            /*4*/ settings.feed_mode = FeedMode::UnitsPerMinute;

            /*5*/ if (settings.feed_override != ON)
            {
                feed_override_enable();
                settings.feed_override = ON;
            }
            if (settings.speed_override != ON)
            {
                speed_override_enable();
                settings.speed_override = ON;
            }

            /*6*/ settings.cutter_comp_side = Side::Off;
            settings.program_x = UNKNOWN;

            /*7*/ spindle_stop();
            settings.spindle_turning = Direction::Stop;

            /*8*/ settings.motion_mode = G_1;

            /*9*/ if (settings.coolant.mist == ON)
            {
                coolant_mist_off();
                settings.coolant.mist = OFF;
            }
            if (settings.coolant.flood == ON)
            {
                coolant_flood_off();
                settings.coolant.flood = OFF;
            }

            if (block.m_modes[4] == 30)
                pallet_shuttle();
            program_end();
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
   This executes a linear command at cutting feed rate
   (if move is G_1) or a rapid command (if move is G_0).
   It also updates the setting of the position of the tool point to the
   end point of the move. If cutter radius compensation is on, it may
   also generate an arc before the straight move. Also, in INVERSE_TIME
   feed mode, feed_rate will be called the feed rate setting changed.

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
    	Position end;

        if (move == G_1)
        {
            if (settings.feed_mode == FeedMode::UnitsPerMinute)
            {
                error_if(settings.feed_rate == 0.0, NCE_CANNOT_DO_G1_WITH_ZERO_FEED_RATE);
            }
            else if (settings.feed_mode == FeedMode::InverseTime)
            {
                error_if(!block.f, NCE_F_WORD_MISSING_WITH_INVERSE_TIME_G1_MOVE);
            }
        }

        settings.motion_mode = move;
        find_ends(block, settings, &end.x, &end.y, &end.z, &end.a, &end.b, &end.c);
   /* not "IS ON" */
        if ((settings.cutter_comp_side != Side::Off) and (settings.cutter_comp_radius > 0.0)) /* radius always is >= 0 */
        {
            error_if(block.g_modes[0] == G_53, NCE_CANNOT_USE_G53_WITH_CUTTER_RADIUS_COMP);
            if (settings.program_x == UNKNOWN)
            {
                convert_straight_comp1(move, block, settings, end.x, end.y, end.z, end.a, end.b, end.c);
            }
            else
            {
                convert_straight_comp2 (move, block, settings, end.x, end.y, end.z, end.a, end.b, end.c);
            }
        }
        else if (move == G_0)
        {
            rapid(end);
            settings.current.x = end.x;
            settings.current.y = end.y;
        }
        else if (move == G_1)
        {
            if (settings.feed_mode == FeedMode::InverseTime)
                inverse_time_rate_straight(end.x, end.y, end.z, end.a, end.b, end.c, block, settings);
            linear(end);
            settings.current.x = end.x;
            settings.current.y = end.y;
        }
        else
            throw error(NCE_BUG_CODE_NOT_G0_OR_G1);

        settings.current.z = end.z;
        settings.current.a = end.a;
        settings.current.b = end.b;
        settings.current.c = end.c;
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
   or a rapid command.
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
    , double AA_end                               /* A coordinate of end point           */
    , double BB_end                               /* B coordinate of end point           */
    , double CC_end                               /* C coordinate of end point           */
    )
    {
        double alpha;
        double cx;                                /* first current point x then end point x */
        double cy;                                /* first current point y then end point y */
        double distance;
        double radius;
        Side side;
        double theta;

        side = settings.cutter_comp_side;
        cx = settings.current.x;
        cy = settings.current.y;

   /* always will be positive */
        radius = settings.cutter_comp_radius;
        distance = hypot((px - cx), (py -cy));

        error_if((side != Side::Left) and (side != Side::Right), NCE_BUG_SIDE_NOT_RIGHT_OR_LEFT);
        error_if(distance <= radius, NCE_CUTTER_GOUGING_WITH_CUTTER_RADIUS_COMP);

        theta = acos(radius/distance);
        alpha = (side == Side::Left) ? (atan2((cy - py), (cx - px)) - theta) : (atan2((cy - py), (cx - px)) + theta);
        cx = (px + (radius * cos(alpha)));   /* reset to end location */
        cy = (py + (radius * sin(alpha)));
        if (move == G_0)
            rapid({cx, cy, end_z, AA_end, BB_end, CC_end});
        else if (move == G_1)
        {
            if (settings.feed_mode == FeedMode::InverseTime)
                inverse_time_rate_straight(cx, cy, end_z, AA_end, BB_end, CC_end, block, settings);
            linear({cx, cy, end_z, AA_end, BB_end, CC_end});
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
   This executes a linear command at cutting feed rate
   or a rapid command.
   It also generates an arc to go around a corner, if necessary.
   It also updates the setting of the position of the tool point to
   the end point of the move and updates the programmed point.
   If INVERSE_TIME feed mode is in effect, it also calls feed_rate
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
   G1 move, no arc is inserted for a G0 move; a rapid is made
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
    , double AA_end                               /* A coordinate of end point           */
    , double BB_end                               /* B coordinate of end point           */
    , double CC_end                               /* C coordinate of end point           */
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
        Side side;
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
                rapid({end_x, end_y, end_z, AA_end, BB_end, CC_end});
            else if (move == G_1)
            {
                if (settings.feed_mode == FeedMode::InverseTime)
                    inverse_time_rate_straight(end_x, end_y, end_z, AA_end, BB_end, CC_end, block, settings);
                linear({end_x, end_y, end_z, AA_end, BB_end, CC_end});
            }
            else
                throw error(NCE_BUG_CODE_NOT_G0_OR_G1);
        }
        else
        {
            side = settings.cutter_comp_side;
   /* will always be positive */
            radius = settings.cutter_comp_radius;
            theta = atan2(settings.current.y - start_y, settings.current.x - start_x);
            alpha = atan2(py - start_y, px - start_x);

            if (side == Side::Left)
            {
                if (theta < alpha)
                    theta = (theta + TWO_PI);
                beta = ((theta - alpha) - PI2);
                gamma = PI2;
            }
            else if (side == Side::Right)
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
                rapid({end_x, end_y, end_z, AA_end, BB_end, CC_end});
            else if (move == G_1)
            {
                if (beta > small)                 /* ARC NEEDED */
                {
                    if (settings.feed_mode == FeedMode::InverseTime)
                        inverse_time_rate_as(start_x, start_y, (side == Side::Left) ? -1 : 1,
                        mid_x, mid_y, end_x, end_y, end_z, 
                        AA_end, BB_end, CC_end, block, settings);
                    arc(mid_x,mid_y,start_x, start_y, ((side == Side::Left) ? -1 : 1), settings.current.z, AA_end, BB_end, CC_end);
                    linear({end_x, end_y, end_z, AA_end, BB_end, CC_end});
                }
                else
                {
                    if (settings.feed_mode == FeedMode::InverseTime)
                        inverse_time_rate_straight(end_x,end_y,end_z, AA_end, BB_end, CC_end, block, settings);
                    linear({end_x, end_y, end_z, AA_end, BB_end, CC_end});
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
   spindle. The only function call this makes is to the tool_change
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

   2. Allow the executor of the tool_change function to change the state
   of the world however it pleases, and have the interpreter read the
   executor's world model after the tool_change function is carried out.
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
   program will contain an appropriate tool_length_offset command
   near the tool_change command, so that the incorrect setting is only
   temporary.

   In [NCMS, page 73, 74] there are three other legal approaches in addition
   to this one.

   */

    void rs274ngc::convert_tool_change(               /* ARGUMENTS                   */
    setup_t& settings)                       /* pointer to machine settings */
    {
        tool_change(settings.selected_tool_slot);
        settings.current_slot = settings.selected_tool_slot;
        settings.spindle_turning = Direction::Stop;
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
   A tool_length_offset function call is made. Current_z,
   tool_length_offset, and length_offset_index are reset.

   Called by: convert_g

   This is called to execute g43 or g49.

   The g49 RS274/NGC command translates into a tool_length_offset(0.0)
   function call.

   The g43 RS274/NGC command translates into a tool_length_offset(length)
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
            tool_length_offset(0.0);
            settings.current.z = (settings.current.z + settings.tool_length_offset);
            settings.tool_length_offset = 0.0;
            settings.length_offset_index = 0;
        }
        else if (g_code == G_43)
        {
            error_if(!block.h, NCE_OFFSET_INDEX_MISSING);
            index = *block.h;
            offset = settings.tool_table[index].length;
            tool_length_offset(offset);
            settings.current.z = (settings.current.z + settings.tool_length_offset - offset);
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
        error_if(*block.t > settings.tool_max, NCE_SELECTED_TOOL_SLOT_NUMBER_TOO_LARGE);
        tool_select(*block.t);
        settings.selected_tool_slot = *block.t;
    }

   /****************************************************************************/

   /* cycle_feed

   Returned Value: int (RS274NGC_OK)

   Side effects:
   linear is called.

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

   This writes a linear command appropriate for a cycle move with
   respect to the given plane. No rotary axis motion takes place.

   */

    void rs274ngc::cycle_feed(                        /* ARGUMENTS                  */
    Plane plane,                            /* currently selected plane   */
    double end1,                                  /* first coordinate value     */
    double end2,                                  /* second coordinate value    */
    double end3)                                  /* third coordinate value     */
    {
        if (plane == Plane::XY)
            linear({end1, end2, end3, _setup.current.a, _setup.current.b, _setup.current.c});
        else if (plane == Plane::YZ)
            linear({end3, end1, end2, _setup.current.a, _setup.current.b, _setup.current.c});
        else                                      /* if (plane == Plane::XZ) */
            linear({end2, end3, end1, _setup.current.a, _setup.current.b, _setup.current.c});
    }

   /****************************************************************************/

   /* cycle_traverse

   Returned Value: int (RS274NGC_OK)

   Side effects:
   rapid is called.

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

   This writes a rapid command appropriate for a cycle
   move with respect to the given plane. No rotary axis motion takes place.

   */

    void rs274ngc::cycle_traverse(                    /* ARGUMENTS                 */
    Plane plane,                            /* currently selected plane  */
    double end1,                                  /* first coordinate value    */
    double end2,                                  /* second coordinate value   */
    double end3)                                  /* third coordinate value    */
    {
        if (plane == Plane::XY)
            rapid({end1, end2, end3, _setup.current.a, _setup.current.b, _setup.current.c});
        else if (plane == Plane::YZ)
            rapid({end3, end1, end2, _setup.current.a, _setup.current.b, _setup.current.c});
        else                                      /* if (plane == Plane::XZ) */
            rapid({end2, end3, end1, _setup.current.a, _setup.current.b, _setup.current.c});
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
        if (block.f)
        {
   /* handle elsewhere */
            if (settings.feed_mode == FeedMode::InverseTime);
            else
            {
                convert_feed_rate(block, settings);
            }
        }
        if (block.s)
        {
            convert_speed(block, settings);
        }
        if (block.t)
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
    , double * AA_p                               /* pointer to end_a                       */
    , double * BB_p                               /* pointer to end_b                       */
    , double * CC_p                               /* pointer to end_c                       */
    )
    {
        DistanceMode mode;
        int middle;
        int comp;

        mode = settings.distance_mode;
        middle = (settings.program_x != UNKNOWN);
        comp = (settings.cutter_comp_side != Side::Off);

        if (block.g_modes[0] == G_53)            /* distance mode is absolute in this case */
        {
#ifdef DEBUG_EMC
            comment("interpreter: offsets temporarily suspended");
#endif
            *px = block.x ? (*block.x - (settings.origin_offset.x + settings.axis_offset.x)) : settings.current.x;
            *py = block.y ? (*block.y - (settings.origin_offset.y + settings.axis_offset.y)) : settings.current.y;
            *pz = block.z ? (*block.z - (settings.tool_length_offset + settings.origin_offset.z + settings.axis_offset.z)) : settings.current.z;
            
            *AA_p = block.a ? (*block.a - (settings.origin_offset.a + settings.axis_offset.a)) : settings.current.a;
            *BB_p = block.b ? (*block.b - (settings.origin_offset.b + settings.axis_offset.b)) : settings.current.b;
            *CC_p = block.c ? (*block.c - (settings.tool_length_offset + settings.origin_offset.c + settings.axis_offset.c)) : settings.current.c;
        }
        else if (mode == DistanceMode::Absolute)
        {
            *px = block.x ? *block.x : (comp and middle) ? settings.program_x : settings.current.x;
            *py = block.y ? *block.y : (comp and middle) ? settings.program_y : settings.current.y;
            *pz = block.z ? *block.z : settings.current.z;
            
            *AA_p = block.a ? *block.a : settings.current.a;
            *BB_p = block.b ? *block.b : settings.current.b;
            *CC_p = block.c ? *block.c : settings.current.c;
        }
        else                                      /* mode is MODE_INCREMENTAL */
        {
            *px = block.x
                ? ((comp and middle) ? (*block.x + settings.program_x) : (*block.x + settings.current.x))
                : ((comp and middle) ? settings.program_x : settings.current.x);

            *py = block.y
                ? ((comp and middle) ? (*block.y + settings.program_y) : (*block.y + settings.current.y))
                : ((comp and middle) ? settings.program_y : settings.current.y);

            *pz = block.z ? (settings.current.z + *block.z) : settings.current.z;
            
            *AA_p = block.a ? (settings.current.a + *block.a) : settings.current.a;
            *BB_p = block.b ? (settings.current.b + *block.b) : settings.current.b;
            *CC_p = block.c ? (settings.current.c + *block.c) : settings.current.c;
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
    double AA_1,             /* absolute a position         */
    double BB_1,             /* absolute b position         */
    double CC_1,             /* absolute c position         */
    double * x2,                                  /* pointer to relative x       */
    double * y2,                                  /* pointer to relative y       */
    double * z2,                                  /* pointer to relative z       */
    double * AA_2,           /* pointer to relative a       */
    double * BB_2,           /* pointer to relative b       */
    double * CC_2,           /* pointer to relative c       */
    setup_t& settings)                       /* pointer to machine settings */
    {
        *x2 = (x1 - (settings.origin_offset.x + settings.axis_offset.x));
        *y2 = (y1 - (settings.origin_offset.y + settings.axis_offset.y));
        *z2 = (z1 - (settings.tool_length_offset + settings.origin_offset.z + settings.axis_offset.z));
        *AA_2 = (AA_1 - (settings.origin_offset.a + settings.axis_offset.a));
        *BB_2 = (BB_1 - (settings.origin_offset.b + settings.axis_offset.b));
        *CC_2 = (CC_1 - (settings.origin_offset.c + settings.axis_offset.c));
    }

   /****************************************************************************/

   /* inverse_time_rate_arc

   Returned Value: int (RS274NGC_OK)

   Side effects: a call is made to feed_rate and _setup.feed_rate is set.

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
        rate = std::max(0.1, (length * *block.f));
        feed_rate (rate);
        settings.feed_rate = rate;
    }

   /****************************************************************************/

   /* inverse_time_rate_arc2

   Returned Value: int (RS274NGC_OK)

   Side effects: a call is made to feed_rate and _setup.feed_rate is set.

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
        rate = std::max(0.1, (length * *block.f));
        feed_rate (rate);
        settings.feed_rate = rate;
    }

   /****************************************************************************/

   /* inverse_time_rate_as

   Returned Value: int (RS274NGC_OK)

   Side effects: a call is made to feed_rate and _setup.feed_rate is set.

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
    double AA_end,                                /* A coord of end point of straight line       */
    double BB_end,                                /* B coord of end point of straight line       */
    double CC_end,                                /* C coord of end point of straight line       */
    block_t& block,                          /* pointer to a block of RS274 instructions          */
    setup_t& settings)                       /* pointer to machine settings                       */
    {
        double length;
        double rate;

        length = (
        	find_arc_length (settings.current.x, settings.current.y,
		        settings.current.z, start_x, start_y,
		        turn, mid_x, mid_y, settings.current.z)
		    +
            find_straight_length({end_x, end_y, end_z, AA_end, BB_end, CC_end}, 
		      				  	 {mid_x, mid_y, settings.current.z, AA_end, BB_end, CC_end}));
		        
        rate = std::max(0.1, (length * *block.f));
        feed_rate (rate);
        settings.feed_rate = rate;
    }

   /****************************************************************************/

   /* inverse_time_rate_straight

   Returned Value: int (RS274NGC_OK)

   Side effects: a call is made to feed_rate and _setup.feed_rate is set.

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
    double AA_end,                                /* A coordinate of end point of straight line */
    double BB_end,                                /* B coordinate of end point of straight line */
    double CC_end,                                /* C coordinate of end point of straight line */
    block_t& block,                          /* pointer to a block of RS274 instructions   */
    setup_t& settings)                       /* pointer to machine settings                */
    {
        double length;
        double rate;

        length = find_straight_length
            ({end_x, end_y, end_z, AA_end, BB_end, CC_end}, 
            {settings.current.x, settings.current.y, settings.current.z, 
            settings.current.a, settings.current.b, settings.current.c});
        rate = std::max(0.1, (length * *block.f));
        feed_rate (rate);
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
    setup_t& settings) const                       /* pointer to machine settings          */
    {
        block = block_t{};
        read_items(block, line, settings.parameters);
        block.enhance(settings);
        block.check_items(settings);
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

        error_if(line[*counter] != 'a', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.a, NCE_MULTIPLE_A_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.a = value;
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

        error_if(line [*counter] != '/', NCE_SLASH_MISSING_AFTER_FIRST_ATAN_ARGUMENT);
        *counter = (*counter + 1);
        error_if(line[*counter] != '[', NCE_LEFT_BRACKET_MISSING_AFTER_SLASH_WITH_ATAN);
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

        error_if(line[*counter] != 'b', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.b, NCE_MULTIPLE_B_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.b = value;
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

        error_if(line[*counter] != 'c', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.c, NCE_MULTIPLE_C_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.c = value;
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

        error_if(line[*counter] != '(', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
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

        error_if(line[*counter] != 'd', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.d, NCE_MULTIPLE_D_WORDS_ON_ONE_LINE);
        read_integer_value(line, counter, &value, parameters);
        error_if(value < 0, NCE_NEGATIVE_D_WORD_TOOL_RADIUS_INDEX_USED);
        unsigned int d = value;
        error_if(d > _setup.tool_max, NCE_TOOL_RADIUS_INDEX_TOO_BIG);
        block.d = d;
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

        error_if(line[*counter] != 'f', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.f, NCE_MULTIPLE_F_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        error_if(value < 0.0, NCE_NEGATIVE_F_WORD_USED);
        block.f = value;
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

        error_if(line[*counter] != 'g', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        read_real_value(line, counter, &value_read, parameters);
        value_read = (10.0 * value_read);
        value = (int)floor(value_read);

        if ((value_read - value) > 0.999)
            value = (int)ceil(value_read);
        else if ((value_read - value) > 0.001)
            throw error(NCE_G_CODE_OUT_OF_RANGE);

        error_if(value > 999, NCE_G_CODE_OUT_OF_RANGE);
        error_if(value < 0, NCE_NEGATIVE_G_CODE_USED);
        mode = _gees[value];
        error_if(mode == -1, NCE_UNKNOWN_G_CODE_USED);
        error_if(block.g_modes[mode] != -1, NCE_TWO_G_CODES_USED_FROM_SAME_MODAL_GROUP);
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

        error_if(line[*counter] != 'h', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.h, NCE_MULTIPLE_H_WORDS_ON_ONE_LINE);
        read_integer_value(line, counter, &value, parameters);
        error_if(value < 0, NCE_NEGATIVE_H_WORD_TOOL_LENGTH_OFFSET_INDEX_USED);
        unsigned int h = value;
        error_if(h > _setup.tool_max, NCE_TOOL_LENGTH_OFFSET_INDEX_TOO_BIG);
        block.h = h;
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

        error_if(line[*counter] != 'i', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.i, NCE_MULTIPLE_I_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.i = value;
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
    unsigned int * integer_ptr) const                            /* pointer to the value being read               */
    {
        int n;
        char c;

        for (n = *counter; ; n++)
        {
            c = line[n];
            if ((c < 48) or (c > 57))
                break;
        }
        error_if(n == *counter, NCE_BAD_FORMAT_UNSIGNED_INTEGER);
        if (sscanf(line + *counter, "%u", integer_ptr) == 0)
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

        error_if(line[*counter] != 'j', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.j, NCE_MULTIPLE_J_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.j = value;
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

        error_if(line[*counter] != 'k', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.k, NCE_MULTIPLE_K_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.k = value;
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

        error_if(line[*counter] != 'l', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.l, NCE_MULTIPLE_L_WORDS_ON_ONE_LINE);
        read_integer_value(line, counter, &value, parameters);
        error_if(value < 0, NCE_NEGATIVE_L_WORD_USED);
        block.l = value;
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
        unsigned int value;

        error_if(line[*counter] != 'n', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        read_integer_unsigned(line, counter, &value);
        error_if(value > 99999, NCE_LINE_NUMBER_GREATER_THAN_99999);
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

        error_if(line[*counter] != 'm', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        read_integer_value(line, counter, &value, parameters);
        error_if(value < 0, NCE_NEGATIVE_M_CODE_USED);
        error_if(value > 99, NCE_M_CODE_GREATER_THAN_99);
        mode = _ems[value];
        error_if(mode == -1, NCE_UNKNOWN_M_CODE_USED);
        error_if(block.m_modes[mode] != -1, NCE_TWO_M_CODES_USED_FROM_SAME_MODAL_GROUP);
        block.m_modes[mode] = value;
        ++block.m_count;
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
        switch(line[*counter])
        {
		    case '#':
		    	read_parameter_setting(line, counter, block, parameters);
		    	break;
		    case '(':
			    read_comment(line, counter, block, parameters);
		    	break;
		    case 'a':
		    	read_a(line, counter, block, parameters);
		    	break;
		    case 'b':
			    read_b(line, counter, block, parameters);
			    break;
		    case 'c':
			    read_c(line, counter, block, parameters);
			    break;
		    case 'd':
			    read_d(line, counter, block, parameters);
			    break;
		    case 'f':
			    read_f(line, counter, block, parameters);
			    break;
		    case 'g':
			    read_g(line, counter, block, parameters);
			    break;
		    case 'h':
			    read_h(line, counter, block, parameters);
			    break;
		    case 'i':
			    read_i(line, counter, block, parameters);
			    break;
		    case 'j':
			    read_j(line, counter, block, parameters);
			    break;
		    case 'k':
			    read_k(line, counter, block, parameters);
			    break;
		    case 'l':
			    read_l(line, counter, block, parameters);
			    break;
		    case 'm':
			    read_m(line, counter, block, parameters);
			    break;
		    case 'p':
			    read_p(line, counter, block, parameters);
			    break;
		    case 'q':
			    read_q(line, counter, block, parameters);
			    break;
		    case 'r':
			    read_r(line, counter, block, parameters);
			    break;
		    case 's':
			    read_s(line, counter, block, parameters);
			    break;
		    case 't':
			    read_t(line, counter, block, parameters);
			    break;
		    case 'x':
			    read_x(line, counter, block, parameters);
			    break;
		    case 'y':
			    read_y(line, counter, block, parameters);
			    break;
		    case 'z':
			    read_z(line, counter, block, parameters);
			    break;
			default:
				throw error(NCE_BAD_CHARACTER_USED);
        }
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
    BinaryOperation * operation) const                              /* pointer to operation to be read                */
    {
        char c;

        c = line[*counter];
        *counter = (*counter + 1);
        switch(c)
        {
            case '+':
                *operation = BinaryOperation::PLUS;
                break;
            case '-':
                *operation = BinaryOperation::MINUS;
                break;
            case '/':
                *operation = BinaryOperation::DIVIDED_BY;
                break;
            case '*':
                if(line[*counter] == '*')
                {
                    *operation = BinaryOperation::POWER;
                    *counter = (*counter + 1);
                }
                else
                    *operation = BinaryOperation::TIMES;
                break;
            case ']':
                *operation = BinaryOperation::RIGHT_BRACKET;
                break;
            case 'a':
                if((line[*counter] == 'n') and (line[(*counter)+1] == 'd'))
                {
                    *operation = BinaryOperation::AND2;
                    *counter = (*counter + 2);
                }
                else
                    throw error(NCE_UNKNOWN_OPERATION_NAME_STARTING_WITH_A);
                break;
            case 'm':
                if((line[*counter] == 'o') and (line[(*counter)+1] == 'd'))
                {
                    *operation = BinaryOperation::MODULO;
                    *counter = (*counter + 2);
                }
                else
                    throw error(NCE_UNKNOWN_OPERATION_NAME_STARTING_WITH_M);
                break;
            case 'o':
                if(line[*counter] == 'r')
                {
                    *operation = BinaryOperation::NON_EXCLUSIVE_OR;
                    *counter = (*counter + 1);
                }
                else
                    throw error(NCE_UNKNOWN_OPERATION_NAME_STARTING_WITH_O);
                break;
            case 'x':
                if((line[*counter] == 'o') and (line[(*counter)+1] == 'r'))
                {
                    *operation = BinaryOperation::EXCLUSIVE_OR;
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
    UnaryOperation * operation) const                              /* pointer to operation to be read                */
    {
        char c;

        c = line[*counter];
        *counter = (*counter + 1);
        switch (c)
        {
            case 'a':
                if((line[*counter] == 'b') and (line[(*counter)+1] == 's'))
                {
                    *operation = UnaryOperation::ABS;
                    *counter = (*counter + 2);
                }
                else if(strncmp((line + *counter), "cos", 3) == 0)
                {
                    *operation = UnaryOperation::ACOS;
                    *counter = (*counter + 3);
                }
                else if(strncmp((line + *counter), "sin", 3) == 0)
                {
                    *operation = UnaryOperation::ASIN;
                    *counter = (*counter + 3);
                }
                else if(strncmp((line + *counter), "tan", 3) == 0)
                {
                    *operation = UnaryOperation::ATAN;
                    *counter = (*counter + 3);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_A);
                break;
            case 'c':
                if((line[*counter] == 'o') and (line[(*counter)+1] == 's'))
                {
                    *operation = UnaryOperation::COS;
                    *counter = (*counter + 2);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_C);
                break;
            case 'e':
                if((line[*counter] == 'x') and (line[(*counter)+1] == 'p'))
                {
                    *operation = UnaryOperation::EXP;
                    *counter = (*counter + 2);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_E);
                break;
            case 'f':
                if((line[*counter] == 'i') and (line[(*counter)+1] == 'x'))
                {
                    *operation = UnaryOperation::FIX;
                    *counter = (*counter + 2);
                }
                else if((line[*counter] == 'u') and (line[(*counter)+1] == 'p'))
                {
                    *operation = UnaryOperation::FUP;
                    *counter = (*counter + 2);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_F);
                break;
            case 'l':
                if(line[*counter] == 'n')
                {
                    *operation = UnaryOperation::LN;
                    *counter = (*counter + 1);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_L);
                break;
            case 'r':
                if(strncmp((line + *counter), "ound", 4) == 0)
                {
                    *operation = UnaryOperation::ROUND;
                    *counter = (*counter + 4);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_R);
                break;
            case 's':
                if((line[*counter] == 'i') and (line[(*counter)+1] == 'n'))
                {
                    *operation = UnaryOperation::SIN;
                    *counter = (*counter + 2);
                }
                else if(strncmp((line + *counter), "qrt", 3) == 0)
                {
                    *operation = UnaryOperation::SQRT;
                    *counter = (*counter + 3);
                }
                else
                    throw error(NCE_UNKNOWN_WORD_STARTING_WITH_S);
                break;
            case 't':
                if((line[*counter] == 'a') and (line[(*counter)+1] == 'n'))
                {
                    *operation = UnaryOperation::TAN;
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

        error_if(line[*counter] != 'p', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.p, NCE_MULTIPLE_P_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        error_if(value < 0.0, NCE_NEGATIVE_P_WORD_USED);
        block.p = value;
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

        error_if(line[*counter] != '#', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        read_integer_value(line, counter, &index, parameters);
        error_if(((index < 1) or (static_cast<unsigned int>(index) >= RS274NGC_MAX_PARAMETERS)), NCE_PARAMETER_NUMBER_OUT_OF_RANGE);
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

        error_if(line[*counter] != '#', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        read_integer_value(line, counter, &index, parameters);
        error_if(((index < 1) or (static_cast<unsigned int>(index) >= RS274NGC_MAX_PARAMETERS)), NCE_PARAMETER_NUMBER_OUT_OF_RANGE);
        error_if(line[*counter] != '=', NCE_EQUAL_SIGN_MISSING_IN_PARAMETER_SETTING);
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

        error_if(line[*counter] != 'q', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.q, NCE_MULTIPLE_Q_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        error_if(value <= 0.0, NCE_NEGATIVE_OR_ZERO_Q_VALUE_USED);
        block.q = value;
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

        error_if(line[*counter] != 'r', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.r, NCE_MULTIPLE_R_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.r = value;
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

        error_if(line[*counter] != '[', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
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
        BinaryOperation operators[MAX_STACK];
        int stack_index;

        error_if(line[*counter] != '[', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        read_real_value(line, counter, values, parameters);
        read_operation(line, counter, operators);
        stack_index = 1;
        for(; operators[0] != BinaryOperation::RIGHT_BRACKET ;)
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
        for (; (c = line[n]) != 0; n++)
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
        error_if(c == 0, NCE_NO_CHARACTERS_FOUND_IN_READING_REAL_VALUE);
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

        error_if(line[*counter] != 's', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.s, NCE_MULTIPLE_S_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        error_if(value < 0.0, NCE_NEGATIVE_SPINDLE_SPEED_USED);
        block.s = value;
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

        error_if(line[*counter] != 't', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.t, NCE_MULTIPLE_T_WORDS_ON_ONE_LINE);
        read_integer_value(line, counter, &value, parameters);
        error_if(value < 0, NCE_NEGATIVE_TOOL_ID_USED);
        block.t = value;
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
   NCE_FILE_ENDED_WITH_NO_PERCENT_SIGN_OR_program_end
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
    unsigned int * length)                                 /* a pointer to an integer to be set           */
    {
        error_if(strlen(command) >= RS274NGC_TEXT_SIZE, NCE_COMMAND_TOO_LONG);
        strcpy(raw_line, command);
        strcpy(line, command);
        close_and_downcase(line);
        
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
        UnaryOperation operation;

        read_operation_unary (line, counter, &operation);
        error_if(line[*counter] != '[', NCE_LEFT_BRACKET_MISSING_AFTER_UNARY_OPERATION_NAME);
        read_real_expression (line, counter, double_ptr, parameters);

        if (operation == UnaryOperation::ATAN)
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

        error_if(line[*counter] != 'x', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.x, NCE_MULTIPLE_X_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.x = value;
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

        error_if(line[*counter] != 'y', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.y, NCE_MULTIPLE_Y_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.y = value;
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

        error_if(line[*counter] != 'z', NCE_BUG_FUNCTION_SHOULD_NOT_HAVE_BEEN_CALLED);
        *counter = (*counter + 1);
        error_if(!!block.z, NCE_MULTIPLE_Z_WORDS_ON_ONE_LINE);
        read_real_value(line, counter, &value, parameters);
        block.z = value;
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
        settings.current = current_position();
        auto probe_pos = probe_position();
        settings.parameters[5061] = probe_pos.x;
        settings.parameters[5062] = probe_pos.y;
        settings.parameters[5063] = probe_pos.z;
        settings.parameters[5064] = probe_pos.a;
        settings.parameters[5065] = probe_pos.b;
        settings.parameters[5066] = probe_pos.c;
        settings.parameters[5067] = probe_value();
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
            _setup.write_g_codes(&_setup.block1);
            _setup.write_m_codes(&_setup.block1);
            _setup.write_settings();
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

        get_parameter_filename(file_name, (RS274NGC_TEXT_SIZE - 1));
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
   A units canonical command call is made.
   A SET_FEED_REFERENCE canonical command call is made.
   A offset_origin canonical command call is made.
   An INIT_CANON call is made.

   Called By: external programs

   Currently we are running only in CANON_XYZ feed_reference mode.  There
   is no command regarding feed_reference in the rs274 language (we
   should try to get one added). The initialization routine, therefore,
   always calls SET_FEED_REFERENCE(CANON_XYZ).

   */

    void rs274ngc::init()                           /* NO ARGUMENTS */
    {
        char filename[RS274NGC_TEXT_SIZE];
        double * pars;                            // short name for _setup.parameters

        interp_init();

        _setup.length_units = units();
        units(_setup.length_units);

        get_parameter_filename(filename, RS274NGC_TEXT_SIZE);
        if (filename[0] == 0)
            strcpy(filename, RS274NGC_PARAMETER_FILE_NAME_DEFAULT);
        restore_parameters(filename);
        pars = _setup.parameters;
        
        _setup.origin_index = (int)(pars[5220] + 0.0001);
        error_if(((_setup.origin_index < 1) or (_setup.origin_index > 9)), NCE_COORDINATE_SYSTEM_INDEX_PARAMETER_5220_OUT_OF_RANGE);
        
        // starting index in parameters of origin offsets
        unsigned int k = (5200 + (_setup.origin_index * 20));

        _setup.axis_offset.x = pars[5211];
        _setup.axis_offset.y = pars[5212];
        _setup.axis_offset.z = pars[5213];
        _setup.axis_offset.a = pars[5214];
        _setup.axis_offset.b = pars[5215];
        _setup.axis_offset.c = pars[5216];

        _setup.origin_offset.x = pars[k + 1];
        _setup.origin_offset.y = pars[k + 2];
        _setup.origin_offset.z = pars[k + 3];
        _setup.origin_offset.a = pars[k + 4];
        _setup.origin_offset.b = pars[k + 5];
        _setup.origin_offset.c = pars[k + 6];
        
        offset_origin(_setup.origin_offset + _setup.axis_offset);
        
        feed_reference(FeedReference::XYZ);
   //_setup.active_g_codes initialized below
   //_setup.active_m_codes initialized below
   //_setup.active_settings initialized below
   //_setup.block1 does not need initialization
        _setup.blocktext[0] = 0;
   //_setup.current_slot set in rs274ngc_synch
   //_setup.current set in rs274ngc_synch
        _setup.cutter_comp_side = Side::Off;
   //_setup.cycle values do not need initialization
        _setup.distance_mode = DistanceMode::Absolute;
        _setup.feed_mode = FeedMode::UnitsPerMinute;
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
   //_setup.parameters set above
   //_setup.plane set in rs274ngc_synch
        _setup.probe_flag = OFF;
        _setup.program_x = UNKNOWN;          /* for cutter comp */
        _setup.program_y = UNKNOWN;          /* for cutter comp */
        _setup.retract_mode = OLD_Z;
   //_setup.selected_tool_slot set in rs274ngc_synch
   //_setup.speed set in rs274ngc_synch
        _setup.speed_feed_mode = SpeedFeedMode::Independant;
        _setup.speed_override = ON;
   //_setup.spindle_turning set in rs274ngc_synch
   //_setup.stack does not need initialization
   //_setup.stack_index does not need initialization
        _setup.tool_length_offset = 0.0;
   //_setup.tool_max set in rs274ngc_synch
   //_setup.tool_table set in rs274ngc_synch
        _setup.tool_table_index = 1;
   //_setup.traverse_rate set in rs274ngc_synch

   // Synch rest of settings to external world
        synch();
        
        _setup.write_g_codes(nullptr);
        _setup.write_m_codes(nullptr);
        _setup.write_settings();

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
        unsigned int n;

        error_if(_setup.tool_max > CANON_TOOL_MAX, NCE_TOOL_MAX_TOO_LARGE);
        for (n = 0; n <= _setup.tool_max; n++)
        {
            _setup.tool_table[n] = tool(n);
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
            error_if(!queue_empty(), NCE_QUEUE_IS_NOT_EMPTY_AFTER_PROBING);
            set_probe_data(_setup);
            _setup.probe_flag = OFF;
        }
        error_if(command == nullptr, NCE_FILE_NOT_OPEN);
        read_status = read_text(command, _setup.linetext, _setup.blocktext, &_setup.line_length);
        if (read_status == RS274NGC_EXECUTE_FINISH or read_status == RS274NGC_OK)
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
        unsigned int required;                             // number of next required parameter
        int index;                                // index into _required_parameters
        double * pars;                            // short name for _setup.parameters

   // open original for reading
        infile = fopen(filename, "r");
        error_if(infile == nullptr, NCE_UNABLE_TO_OPEN_FILE);

        pars = _setup.parameters;
        unsigned int k = 0;
        index = 0;
        required = _required_parameters[index++];
        while (feof(infile) == 0)
        {
            if (fgets(line, 256, infile) == nullptr)
            {
                break;
            }

   // try for a variable-value match in the file
            if (sscanf(line, "%d %lf", &variable, &value) == 2)
            {
                error_if(((variable <= 0) or (static_cast<unsigned int>(variable) >= RS274NGC_MAX_PARAMETERS)), NCE_PARAMETER_NUMBER_OUT_OF_RANGE);
                for (; k < RS274NGC_MAX_PARAMETERS; k++)
                {
                    if (k > static_cast<unsigned int>(variable))
                        throw error(NCE_PARAMETER_FILE_OUT_OF_ORDER);
                    else if (k == static_cast<unsigned int>(variable))
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
        error_if(required != RS274NGC_MAX_PARAMETERS, NCE_REQUIRED_PARAMETER_MISSING);
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
        unsigned int required;                             // number of next required parameter
        int index;                                // index into _required_parameters

   // rename as .bak
        strcpy(line, filename);
        strcat(line, RS274NGC_PARAMETER_FILE_BACKUP_SUFFIX);
        error_if(rename(filename, line) != 0, NCE_CANNOT_CREATE_BACKUP_FILE);

   // open backup for reading
        infile = fopen(line, "r");
        error_if(infile == nullptr, NCE_CANNOT_OPEN_BACKUP_FILE);

   // open original for writing
        outfile = fopen(filename, "w");
        error_if(outfile == nullptr, NCE_CANNOT_OPEN_VARIABLE_FILE);

        unsigned int k = 0;
        index = 0;
        required = _required_parameters[index++];
        while (feof(infile) == 0)
        {
            if (fgets(line, 256, infile) == nullptr)
            {
                break;
            }
   // try for a variable-value match
            if (sscanf(line, "%d %lf", &variable, &value) == 2)
            {
                error_if(((variable <= 0) or (static_cast<unsigned int>(variable) >= RS274NGC_MAX_PARAMETERS)), NCE_PARAMETER_NUMBER_OUT_OF_RANGE);
                for (; k < RS274NGC_MAX_PARAMETERS; k++)
                {
                    if (k > static_cast<unsigned int>(variable))
                        throw error(NCE_PARAMETER_FILE_OUT_OF_ORDER);
                    else if (k == static_cast<unsigned int>(variable))
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
        _setup.control_mode = motion_mode();
        _setup.current = current_position();
        _setup.current_slot = tool_slot();
        _setup.feed_rate = feed_rate();
        _setup.coolant.flood = coolant_flood() ? ON : OFF;
        _setup.coolant.mist = coolant_mist() ? ON : OFF;
        _setup.length_units = units();
        _setup.plane = plane();
        _setup.selected_tool_slot = tool_slot();
        _setup.speed = spindle_speed();
        _setup.spindle_turning = spindle_state();
        _setup.tool_max = tool_max();
        _setup.traverse_rate = rapid_rate();

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
        for (unsigned n = 0; n < RS274NGC_ACTIVE_G_CODES; n++)
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
        for (unsigned int n = 0; n < RS274NGC_ACTIVE_M_CODES; n++)
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
        for (unsigned int n = 0; n < RS274NGC_ACTIVE_SETTINGS; n++)
        {
            settings[n] = _setup.active_settings[n];
        }
    }

   /***********************************************************************/

   /* rs274ngc_line_length

   Returned Value: the length of the most recently read line

   Side Effects: none

   Called By: external programs

   */

    unsigned int rs274ngc::line_length()
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
    unsigned int max_size)                                 /* maximum number of characters to copy */
    {
        unsigned int n;
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
   /***********************************************************************/
   /* end of file */
