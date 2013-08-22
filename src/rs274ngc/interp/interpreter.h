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

struct Tool
{
    int id;
    double length;
    double diameter;
    
    Tool()
     : id(), length(), diameter()
    {
    }
};

struct Position
{
	double x;
	double y;
	double z;
	double a;
	double b;
	double c;
	
	Position()
	 : x(), y(), z(), a(), b(), c()
	{}
	Position(double x, double y, double z, double a, double b, double c)
	 : x(x), y(y), z(z), a(a), b(b), c(c)
	{}
	Position(double x, double y, double z)
	 : x(x), y(y), z(z), a(), b(), c()
	{}
};

class interpreter
{
private:

	virtual void offset_origin(const Position& pos) =0;
	
	virtual void units(Units u) =0;
	virtual Units units() const =0;
	
	virtual void plane(Plane pl) =0;
	virtual Plane plane() const =0;
	
	virtual void rapid_rate(double rate) =0;
	virtual double rapid_rate() const =0;
	
	virtual void feed_rate(double rate) =0;
	virtual double feed_rate() const =0;
	virtual void feed_reference(FeedReference reference) =0;
	
	virtual void motion_mode(Motion mode) =0;
	virtual Motion motion_mode() const =0;
	
	virtual void cutter_radius_comp(double radius) =0;
	virtual void cutter_radius_comp_start(Side direction) =0;
	virtual void cutter_radius_comp_stop() =0;
	
	virtual void speed_feed_sync_start() =0;
	virtual void speed_feed_sync_stop() =0;
	
	virtual void rapid(const Position& pos) =0;
	virtual void arc(double end0, double end1, double axis0, double axis1, int rotation, double end_point, double a, double b, double c) =0;
	virtual void linear(const Position& pos) =0;
	virtual void probe(const Position& pos) =0;
	virtual void stop() =0;
	virtual void dwell(double seconds) =0;

	virtual void spindle_start_clockwise() =0;
	virtual void spindle_start_counterclockwise() =0;
	virtual void spindle_stop() =0;
	virtual Direction spindle_state() const =0;
	virtual void spindle_speed(double r) =0;
	virtual double spindle_speed() const =0;
	virtual void spindle_orient(double orientation, Direction direction) =0;

	virtual void tool_length_offset(double length) =0;
	virtual double tool_length_offset() const =0;
	virtual void tool_change(int slot) =0;
	virtual void tool_select(int i) =0;
	virtual int tool_slot() const =0;
	virtual Tool tool(int pocket) const =0;
	virtual int tool_max() const =0;

	virtual void axis_clamp(Axis axis) =0;
	virtual void axis_unclamp(Axis axis) =0;

	virtual void comment(const char *s) =0;

	virtual void feed_override_disable() =0;
	virtual void feed_override_enable() =0;

	virtual void speed_override_disable() =0;
	virtual void speed_override_enable() =0;

	virtual void coolant_flood_off() =0;
	virtual void coolant_flood_on() =0;
	virtual bool coolant_flood() const =0;
	
	virtual void coolant_mist_off() =0;
	virtual void coolant_mist_on() =0;
	virtual bool coolant_mist() const =0;

	virtual void message(const char *s) =0;

	virtual void pallet_shuffle() =0;

	virtual void probe_off() =0;
	virtual void probe_on() =0;
	virtual Position probe_position() const =0;
	virtual double probe_value() const =0;

	virtual void program_optional_stop() =0;
	virtual void program_end() =0;
	virtual void program_stop() =0;

	virtual void get_parameter_filename(char* filename, int max_size) const =0;
	virtual Position current_position() const =0;
	virtual bool queue_empty() const =0;

public:

	virtual ~interpreter();
};

}

#endif /* INTERPRETER_H_ */
