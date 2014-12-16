/* cxxcam - C++ CAD/CAM driver library.
 * Copyright (C) 2013  Nicholas Gill
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * block.cpp
 *
 *  Created on: 2013-08-23
 *      Author: nicholas
 */

#include "block.h"
#include "setup.h"
#include "codes.h"
#include "error.h"
#include "rs274ngc_return.hh"
#include <algorithm>

block_t::block_t()
{
	comment[0] = 0;
    std::fill(std::begin(g_modes), std::end(g_modes), -1);
	motion_to_be = -1;
	m_count = 0;
    std::fill(std::begin(m_modes), std::end(m_modes), -1);
	parameter_occurrence = 0;
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

void block_t::enhance(const setup_t& settings)
{
    bool axis_flag = (x or y or z or a or b or c);
    bool mode_zero_covets_axes = (g_modes[0] == G_10) or (g_modes[0] == G_28) or (g_modes[0] == G_30) or (g_modes[0] == G_92);

    if (g_modes[1] != -1)
    {
        if (g_modes[1] == G_80)
        {
            error_if(axis_flag and (not mode_zero_covets_axes), NCE_CANNOT_USE_AXIS_VALUES_WITH_G80);
            error_if((not axis_flag) and (g_modes[0] == G_92), NCE_ALL_AXES_MISSING_WITH_G92);
        }
        else
        {
            error_if(mode_zero_covets_axes, NCE_CANNOT_USE_TWO_G_CODES_THAT_BOTH_USE_AXIS_VALUES);
            error_if(not axis_flag, NCE_ALL_AXES_MISSING_WITH_MOTION_CODE);
        }
        motion_to_be = g_modes[1];
    }
    else if (mode_zero_covets_axes)
    {                                         /* other 3 can get by without axes but not G92 */
        error_if((not axis_flag) and (g_modes[0] == G_92), NCE_ALL_AXES_MISSING_WITH_G92);
    }
    else if (axis_flag)
    {
        error_if(((settings.motion_mode == -1) or (settings.motion_mode == G_80)), NCE_CANNOT_USE_AXIS_VALUES_WITHOUT_A_G_CODE_THAT_USES_THEM);
        motion_to_be = settings.motion_mode;
    }
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

void block_t::check_g_codes(const setup_t& settings) const
{
    int mode0;
    int p_int;

    mode0 = g_modes[0];

    if (mode0 == -1)
    {
    }
    else if (mode0 == G_4)
    {
        error_if(!p, NCE_DWELL_TIME_MISSING_WITH_G4);
    }
    else if (mode0 == G_10)
    {
        p_int = (int)(*p + 0.0001);
        error_if(*l != 2, NCE_LINE_WITH_G10_DOES_NOT_HAVE_L2);
        error_if(((*p + 0.0001) - p_int) > 0.0002, NCE_P_VALUE_NOT_AN_INTEGER_WITH_G10_L2);
        error_if((p_int < 1) or (p_int > 9), NCE_P_VALUE_OUT_OF_RANGE_WITH_G10_L2);
    }
    else if (mode0 == G_28)
    {
    }
    else if (mode0 == G_30)
    {
    }
    else if (mode0 == G_53)
    {
        error_if((motion_to_be != G_0) and (motion_to_be != G_1), NCE_MUST_USE_G0_OR_G1_WITH_G53);
	    error_if(((g_modes[3] == G_91) or ((g_modes[3] != G_90) and
	    (settings.distance_mode == DistanceMode::Incremental))),
	    NCE_CANNOT_USE_G53_INCREMENTAL);
    }
    else if (mode0 == G_92)
    {
    }
    else if ((mode0 == G_92_1) or (mode0 == G_92_2) or (mode0 == G_92_3))
    {
    }
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

void block_t::check_items(const setup_t& settings) const
{
    check_g_codes(settings);
    check_m_codes();
    check_other_codes();
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

void block_t::check_m_codes() const
{
	// max number of m codes on one line
	static const size_t MAX_EMS = 4;
    error_if(m_count > MAX_EMS, NCE_TOO_MANY_M_CODES_ON_LINE);
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

void block_t::check_other_codes() const
{
    int motion;

    motion = motion_to_be;
    if (a)
    {
        error_if((g_modes[1] > G_80) and (g_modes[1] < G_90), NCE_CANNOT_PUT_AN_A_IN_CANNED_CYCLE);
    }
    if (b)
    {
        error_if((g_modes[1] > G_80) and (g_modes[1] < G_90), NCE_CANNOT_PUT_A_B_IN_CANNED_CYCLE);
    }
    if (c)
    {
        error_if((g_modes[1] > G_80) and (g_modes[1] < G_90), NCE_CANNOT_PUT_A_C_IN_CANNED_CYCLE);
    }
    if (d)
    {
        error_if((g_modes[7] != G_41) and (g_modes[7] != G_42) and (g_modes[14] != G_96), NCE_D_WORD_WITH_NO_G41_OR_G42);
    }
    if (h)
    {
        error_if(g_modes[8] != G_43, NCE_H_WORD_WITH_NO_G43);
    }

    if (i)                  /* could still be useless if yz_plane arc */
    {
        error_if((motion != G_2) and (motion != G_3) and (motion != G_87), NCE_I_WORD_WITH_NO_G2_OR_G3_OR_G87_TO_USE_IT);
    }

    if (j)                  /* could still be useless if xz_plane arc */
    {
        error_if((motion != G_2) and (motion != G_3) and (motion != G_87), NCE_J_WORD_WITH_NO_G2_OR_G3_OR_G87_TO_USE_IT);
    }

    if (k)                  /* could still be useless if xy_plane arc */
    {
        error_if((motion != G_2) and (motion != G_3) and (motion != G_87), NCE_K_WORD_WITH_NO_G2_OR_G3_OR_G87_TO_USE_IT);
    }

    if (l)
    {
        error_if(((motion < G_81) or (motion > G_89)) and (g_modes[0] != G_10), NCE_L_WORD_WITH_NO_CANNED_CYCLE_OR_G10);
    }

    if (p)
    {
        error_if(
            (g_modes[0] != G_10) and
            (g_modes[0] != G_4) and
            (motion != G_82) and 
            (motion != G_86) and
            (motion != G_88) and 
            (motion != G_89) and
            (motion != G_2) and 
            (motion != G_3),
            NCE_P_WORD_WITH_NO_G4_G10_G82_G86_G88_G89);

      error_if((motion == G_2 || motion == G_3) && std::round(*p) < 1, "P value should be 1 or greater with G2 or G3");            
    }

    if (q)
    {
        error_if(motion != G_83, NCE_Q_WORD_WITH_NO_G83);
    }

    if (r)
    {
        error_if((((motion != G_2) and (motion != G_3)) and
            ((motion < G_81) or (motion > G_89))),
            NCE_R_WORD_WITH_NO_G_CODE_THAT_USES_IT);
    }
    
    if (!s)
    {
        error_if(g_modes[14] == G_96, NCE_S_WORD_MISSING_WITH_G96);
    }
}

