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
 * setup.h
 *
 *  Created on: 2013-08-27
 *      Author: nicholas
 */

#ifndef SETUP_H_
#define SETUP_H_
#include "types.h"
#include "block.h"

   /*

   The current_x, current_y, and current_z are the location of the tool
   in the current coordinate system. current_x and current_y differ from
   program_x and program_y when cutter radius compensation is on.
   current_z is the position of the tool tip in program coordinates when
   tool length compensation is using the actual tool length; it is the
   position of the spindle when tool length is zero.

   In a setup, the axis_offset values are set by g92 and the origin_offset
   values are set by g54 - g59.3. The net origin offset uses both values
   and is not represented here

   */

   // on-off switch settings
enum ON_OFF : bool
{
	OFF = false,
	ON = true
};

struct setup_t
{
	setup_t();
	
	void write_g_codes(const block_t* block);
	void write_m_codes(const block_t* block);
	void write_settings();

    Position axis_offset; // g92offset
    Position current;
    Position origin_offset;
    
    int active_g_codes [RS274NGC_ACTIVE_G_CODES];                // array of active G codes
    int active_m_codes [RS274NGC_ACTIVE_M_CODES];                // array of active M codes
    double active_settings [RS274NGC_ACTIVE_SETTINGS];               // array of feed, speed, etc.
    block_t block1;                                 // parsed next block
    char blocktext[RS274NGC_TEXT_SIZE];           // linetext downcased, white space gone
    Motion control_mode;               // exact path or cutting mode
    int current_slot;                             // carousel slot number of current tool
    double cutter_comp_radius;                    // current cutter compensation radius
    Side cutter_comp_side;                         // current cutter compensation side
    struct
    {
		double cc;                              // cc-value (normal) for canned cycles
		double i;                               // i-value for canned cycles
		double j;                               // j-value for canned cycles
		double k;                               // k-value for canned cycles
		int l;                                  // l-value for canned cycles
		double p;                               // p-value (dwell) for canned cycles
		double q;                               // q-value for canned cycles
		double r;                               // r-value for canned cycles
    } cycle;
    DistanceMode distance_mode;                  // absolute or incremental
    DistanceMode ijk_distance_mode;                  // absolute or incremental
    FeedMode feed_mode;                                // G_93 (inverse time) or G_94 units/min
    ON_OFF feed_override;                         // whether feed override is enabled
    double feed_rate;                             // feed rate in current units/min
    struct
    {
		ON_OFF flood;                                 // whether flood coolant is on
		ON_OFF mist;                                  // whether mist coolant is on
    } coolant;
    int length_offset_index;                      // for use with tool length offsets
    Units length_units;                     // millimeters or inches
    unsigned int line_length;                              // length of line last read
    char linetext[RS274NGC_TEXT_SIZE];            // text of most recent line read
    int motion_mode;                              // active G-code for motion
    int origin_index;                             // active origin (1=G54 to 9=G59.3)
    double parameters [RS274NGC_MAX_PARAMETERS];                // system parameters
    Plane plane;                            // active plane, XY-, YZ-, or XZ-plane
    ON_OFF probe_flag;                            // flag indicating probing done
    double program_x;                             // program x, used when cutter comp on
    double program_y;                             // program y, used when cutter comp on
    RETRACT_MODE retract_mode;                    // for cycles, old_z or r_plane
    int selected_tool_slot;                       // tool slot selected but not active
    double speed;                                 // current spindle speed in rpm
    SpeedFeedMode speed_feed_mode;        // independent or synched
    ON_OFF speed_override;                        // whether speed override is enabled
    Direction spindle_turning;              // direction spindle is turning
    double tool_length_offset;                    // current tool length offset
    unsigned int tool_max;                                 // highest number tool slot in carousel
    Tool tool_table [CANON_TOOL_MAX + 1];                     // index is slot number
    int tool_table_index;                         // tool index used with cutter comp
    double traverse_rate;                         // rate for traverse motions
};

#endif /* SETUP_H_ */
