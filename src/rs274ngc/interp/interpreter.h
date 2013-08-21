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
 * interpreter.h
 *
 *  Created on: 2013-08-21
 *      Author: nicholas
 */

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

namespace gcode
{

enum class Plane
{
	XY,
	YZ,
	XZ
};

enum class Units
{
	Imperial,
	Metric
};

enum class Motion
{
	Exact_Stop,
	Exact_Path,
	Continious
};

enum class SpeedFeedMode
{
	Synched,
	Independant
};

enum class Direction
{
	Stop,
	Clockwise,
	CounterClockwise
};

enum class FeedReference
{
	Workpiece,
	XYZ
};

enum class Side
{
	Right,
	Left,
	Off
};

enum class Axis
{
	X, Y, Z,
	A, B, C
};

struct ToolTable
{
    int id;
    double length;
    double diameter;
};

class interpreter
{
private:

	virtual void offset_origin(double x, double y, double z, double a, double b, double c);
	virtual void units(Units u);
	virtual void plane(Plane pl);
	virtual void rapid_rate(double rate);
	virtual void rapid(double x, double y, double z, double a, double b, double c);
	virtual void feed_rate(double rate);
	virtual void feed_reference(FeedReference reference);
	virtual void motion_mode(Motion mode);
	virtual void cutter_radius_comp(double radius);
	virtual void start_cutter_radius_comp(Side direction);
	virtual void stop_cutter_radius_comp();
	virtual void start_speed_feed_sync();
	virtual void stop_speed_feed_sync();
	virtual void arc(double end0, double end1, double axis0, double axis1, int rotation, double end_point, double a, double b, double c);
	virtual void linear(double x, double y, double z, double a, double b, double c);
	virtual void probe(double x, double y, double z, double a, double b, double c);
	virtual void stop();
	virtual void dwell(double seconds);

	virtual void spindle_retract_traverse();
	virtual void spindle_start_clockwise();
	virtual void spindle_start_counterclockwise();
	virtual void spindle_speed(double r);
	virtual void spindle_stop();
	virtual void spindle_retract();
	virtual void spindle_orient(double orientation, CANON_DIRECTION direction);
	virtual void spindle_lock_z();
	virtual void spindle_use_force();
	virtual void spindle_use_no_force();

	virtual void tool_length_offset(double length);
	virtual void tool_change(int slot);
	virtual void tool_select(int i);

	virtual void axis_clamp(Axis axis);
	virtual void axis_unclamp(Axis axis);

	virtual void comment(const char *s);

	virtual void feed_override_disable();
	virtual void feed_override_enable();

	virtual void speed_override_disable();
	virtual void speed_override_enable();

	virtual void coolant_flood_off();
	virtual void coolant_flood_on();
	virtual void coolant_mist_off();
	virtual void coolant_mist_on();

	virtual void message(const char *s);

	virtual void pallet_shuffle();

	virtual void probe_off();
	virtual void probe_on();

	virtual void program_optional_stop();
	virtual void program_end();
	virtual void program_stop();



virtual double get_external_feedrate() const;
virtual int get_external_coolant_flood() const;
virtual Units get_external_length_units() const;
virtual int get_external_coolant_mist() const;
virtual Motion get_external_motion_mode() const;

   // returns nothing but copies the name of the parameter file into
   // the filename array, stopping at max_size if the name is longer
   // An empty string may be placed in filename.
void GET_EXTERNAL_PARAMETER_FILE_NAME(char * filename, int max_size);

   // returns the currently active plane
CANON_PLANE GET_EXTERNAL_PLANE();

   // returns the current a-axis position
double GET_EXTERNAL_POSITION_A();

   // returns the current b-axis position
double GET_EXTERNAL_POSITION_B();

   // returns the current c-axis position
double GET_EXTERNAL_POSITION_C();

   // returns the current x-axis position
double GET_EXTERNAL_POSITION_X();

   // returns the current y-axis position
double GET_EXTERNAL_POSITION_Y();

   // returns the current z-axis position
double GET_EXTERNAL_POSITION_Z();

   // Returns the machine A-axis position at the last probe trip.
double GET_EXTERNAL_PROBE_POSITION_A();

   // Returns the machine B-axis position at the last probe trip.
double GET_EXTERNAL_PROBE_POSITION_B();

   // Returns the machine C-axis position at the last probe trip.
double GET_EXTERNAL_PROBE_POSITION_C();

   // Returns the machine X-axis position at the last probe trip.
double GET_EXTERNAL_PROBE_POSITION_X();

   // Returns the machine Y-axis position at the last probe trip.
double GET_EXTERNAL_PROBE_POSITION_Y();

   // Returns the machine Z-axis position at the last probe trip.
double GET_EXTERNAL_PROBE_POSITION_Z();

   // Returns the value for any analog non-contact probing.
double GET_EXTERNAL_PROBE_VALUE();

   // Returns zero if queue is not empty, non-zero if the queue is empty
   // This always returns a valid value
int GET_EXTERNAL_QUEUE_EMPTY();

   // Returns the system value for spindle speed in rpm
double GET_EXTERNAL_SPEED();

   // Returns the system value for direction of spindle turning
CANON_DIRECTION GET_EXTERNAL_SPINDLE();

   // returns current tool length offset
double GET_EXTERNAL_TOOL_LENGTH_OFFSET();

   // Returns number of slots in carousel
int GET_EXTERNAL_TOOL_MAX();

   // Returns the system value for the carousel slot in which the tool
   // currently in the spindle belongs. Return value zero means there is no
   // tool in the spindle.
int GET_EXTERNAL_TOOL_SLOT();

   // Returns the CANON_TOOL_TABLE structure associated with the tool
   // in the given pocket
CANON_TOOL_TABLE GET_EXTERNAL_TOOL_TABLE(int pocket);

   // Returns the system traverse rate
double GET_EXTERNAL_TRAVERSE_RATE();

#endif                                            /* ifndef CANON_HH */

};

}

#endif /* INTERPRETER_H_ */
