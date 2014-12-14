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
 * setup.cpp
 *
 *  Created on: 2013-08-27
 *      Author: nicholas
 */

#include "setup.h"
#include "codes.h"

setup_t::setup_t()
{
	blocktext[0] = 0;
	line_length = 0;
	linetext[0] = 0;
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

void setup_t::write_g_codes(const block_t* block)
{
    auto& gez = active_g_codes;
    gez[0] = 0 /*unused*/;
    gez[1] = motion_mode;
    gez[2] = ((block == nullptr) ? -1 : block->g_modes[0]);
    gez[3] = (plane == Plane::XY) ? G_17 : (plane == Plane::XZ) ? G_18 : G_19;
    gez[4] = (cutter_comp_side == Side::Right) ? G_42 : (cutter_comp_side == Side::Left) ? G_41 : G_40;
    gez[5] = (length_units == Units::Imperial) ? G_20 : G_21;
    gez[6] = (distance_mode == DistanceMode::Absolute) ? G_90 : G_91;
    gez[7] = (feed_mode == FeedMode::UnitsPerMinute) ? G_93 : G_94;
    gez[8] = (origin_index < 7) ? (530 + (10 * origin_index)) : (584 + origin_index);
    gez[9] = (tool_length_offset == 0.0) ? G_49 : G_43;
    gez[10] = (retract_mode == OLD_Z) ? G_98 : G_99;
    gez[11] = (control_mode == Motion::Continuous) ? G_64 : (control_mode == Motion::Exact_Path) ? G_61 : G_61_1;
    gez[13] = (spindle_mode == SpindleMode::ConstantRPM) ? G_97 : G_96;
    gez[14] = (ijk_distance_mode == DistanceMode::Absolute) ? G_90_1 : G_91_1;
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

void setup_t::write_m_codes(const block_t* block)
{
    auto& emz = active_m_codes;
    emz[0] = 0/*unused*/;
    emz[1] = (block == nullptr) ? -1 : block->m_modes[4];/* 1 stopping    */
    emz[2] = (spindle_turning == Direction::Stop) ? 5 : (spindle_turning == Direction::Clockwise) ? 3 : 4;/* 2 spindle     */
    emz[3] = (block == nullptr) ? -1 : block->m_modes[6];/* 3 tool change */
    emz[4] = (coolant.mist == ON) ? 7 : (coolant.flood == ON) ? -1 : 9;/* 4 mist        */
    emz[5] = (coolant.flood == ON) ? 8 : -1;/* 5 flood       */
    emz[6] = (feed_override == ON) ? 48 : 49;/* 6 overrides   */
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

void setup_t::write_settings()
{
    auto& vals = active_settings;
    vals[0] = 0/*unused*/;
    vals[1] = feed_rate;       /* 1 feed rate       */
    vals[2] = speed;           /* 2 spindle speed   */
}

