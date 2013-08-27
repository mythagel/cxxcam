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

/*

Significant modifications by Nicholas Gill.
Slight restructure then plan to rip out gcode parsing for
post processor tool. Current design discards the raw block
when interpreting the high level motion etc. (which in intentional).

Plan to reuse gcode parsing and interpreter individually.

*/

#ifndef RS274NGC_HH
#define RS274NGC_HH
#include "types.h"
#include "setup.h"
#include <cstddef>

class rs274ngc
{
public:
private:
    setup_t _setup;

	static void arc_data_comp_ijk(int move, Side side, double tool_radius, double current_x, double current_y, double end_x, double end_y, double i_number, double j_number, double * center_x, double * center_y, int * turn, double tolerance);
	static void arc_data_comp_r(int move, Side side, double tool_radius, double current_x, double current_y, double end_x, double end_y, double big_radius, double * center_x, double * center_y, int * turn);
	static void arc_data_ijk(int move, double current_x, double current_y, double end_x, double end_y, double i_number, double j_number, double * center_x, double * center_y, int * turn, double tolerance);
	static void arc_data_r(int move, double current_x, double current_y, double end_x, double end_y, double radius, double * center_x, double * center_y, int * turn);

	static void close_and_downcase(char * line);
	
	void convert_arc(int move, block_t& block, setup_t& settings);
	void convert_arc2(int move, block_t& block, setup_t& settings, double * current1, double * current2, double * current3, double end1, double end2, double end3, double AA_end, double BB_end, double CC_end, double offset1, double offset2);
	void convert_arc_comp1(int move, block_t& block, setup_t& settings, Position end);
	void convert_arc_comp2(int move, block_t& block, setup_t& settings, Position end);
	void convert_axis_offsets(int g_code, block_t& block, setup_t& settings);
	void convert_comment(char * comment);
	void convert_control_mode(int g_code, setup_t& settings);
	void convert_coordinate_system(int g_code, setup_t& settings);
	void convert_cutter_compensation(int g_code, block_t& block, setup_t& settings);
	void convert_cutter_compensation_off(setup_t& settings);
	void convert_cutter_compensation_on(Side side, block_t& block, setup_t& settings);
	void convert_cycle(int motion, block_t& block, setup_t& settings);
	void convert_cycle_g81(Plane plane, double x, double y, double clear_z, double bottom_z);
	void convert_cycle_g82(Plane plane, double x, double y, double clear_z, double bottom_z, double dwell);
	void convert_cycle_g83(Plane plane, double x, double y, double r, double clear_z, double bottom_z, double delta);
	void convert_cycle_g84(Plane plane, double x, double y, double clear_z, double bottom_z, Direction direction, SpeedFeedMode mode);
	void convert_cycle_g85(Plane plane, double x, double y, double clear_z, double bottom_z);
	void convert_cycle_g86(Plane plane, double x, double y, double clear_z, double bottom_z, double dwell, Direction direction);
	void convert_cycle_g87(Plane plane, double x, double offset_x, double y, double offset_y, double r, double clear_z, double middle_z, double bottom_z, Direction direction);
	void convert_cycle_g88(Plane plane, double x, double y, double bottom_z, double dwell, Direction direction);
	void convert_cycle_g89(Plane plane, double x, double y, double clear_z, double bottom_z, double dwell);
	void convert_cycle_xy(int motion, block_t& block, setup_t& settings);
	void convert_cycle_yz(int motion, block_t& block, setup_t& settings);
	void convert_cycle_zx(int motion, block_t& block, setup_t& settings);
	void convert_distance_mode(int g_code, setup_t& settings);
	void convert_dwell(double time);
	void convert_feed_mode(int g_code, setup_t& settings);
	void convert_feed_rate(block_t& block, setup_t& settings);
	void convert_g(block_t& block, setup_t& settings);
	void convert_home(int move, block_t& block, setup_t& settings);
	void convert_length_units(int g_code, setup_t& settings);
	void convert_m(block_t& block, setup_t& settings);
	void convert_modal_0(int code, block_t& block, setup_t& settings);
	void convert_motion(int motion, block_t& block, setup_t& settings);
	void convert_probe(block_t& block, setup_t& settings);
	void convert_retract_mode(int g_code, setup_t& settings);
	void convert_setup(block_t& block, setup_t& settings);
	void convert_set_plane(int g_code, setup_t& settings);
	void convert_speed(block_t& block, setup_t& settings);
	int convert_stop(block_t& block, setup_t& settings);
	void convert_straight(int move, block_t& block, setup_t& settings);
	void convert_straight_comp1(int move, block_t& block, setup_t& settings, double px, double py, double end_z, double AA_end, double BB_end, double CC_end);
	void convert_straight_comp2(int move, block_t& block, setup_t& settings, double px, double py, double end_z, double AA_end, double BB_end, double CC_end);
	void convert_tool_change(setup_t& settings);
	void convert_tool_length_offset(int g_code, block_t& block, setup_t& settings);
	void convert_tool_select(block_t& block, setup_t& settings);
	
	void cycle_feed(Plane plane, double end1, double end2, double end3);
	void cycle_traverse(Plane plane, double end1, double end2, double end3);
	static void execute_binary(double * left, BinaryOperation operation, double * right);
	int execute_block(block_t& block, setup_t& settings);
	static void execute_unary(double * double_ptr, UnaryOperation operation);
	static double find_arc_length(double x1, double y1, double z1, double center_x, double center_y, int turn, double x2, double y2, double z2);
	static void find_ends(block_t& block, setup_t& settings, double * px, double * py, double * pz, double * AA_p, double * BB_p, double * CC_p);
	static void find_relative(double x1, double y1, double z1, double AA_1, double BB_1, double CC_1, double * x2, double * y2, double * z2, double * AA_2, double * BB_2, double * CC_2,setup_t& settings);
	static double find_straight_length(const Position& end, const Position& start);
	static double find_turn(double x1, double y1, double center_x, double center_y, int turn, double x2, double y2);
	void inverse_time_rate_arc(double x1, double y1, double z1, double cx, double cy, int turn, double x2, double y2, double z2, block_t& block, setup_t& settings);
	void inverse_time_rate_arc2(double start_x, double start_y, int turn1, double mid_x, double mid_y, double cx, double cy, int turn2, double end_x, double end_y, double end_z, block_t& block, setup_t& settings);
	void inverse_time_rate_as(double start_x, double start_y, int turn, double mid_x, double mid_y, double end_x, double end_y, double end_z, double AA_end, double BB_end, double CC_end, block_t& block, setup_t& settings);
	void inverse_time_rate_straight(double end_x, double end_y, double end_z, double AA_end, double BB_end, double CC_end, block_t& block, setup_t& settings);
	void parse_line(const char * line, block_t& block,setup_t& settings) const;
	static int precedence(BinaryOperation an_operator);
	void read_a(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_atan(const char * line, int * counter, double * double_ptr, double * parameters) const;
	void read_b(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_c(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_comment(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_d(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_f(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_g(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_h(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_i(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_integer_unsigned(const char * line, int * counter, unsigned int * integer_ptr) const;
	void read_integer_value(const char * line, int * counter, int * integer_ptr, double * parameters) const;
	void read_items(block_t& block, const char * line, double * parameters) const;
	void read_j(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_k(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_l(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_line_number(const char * line, int * counter, block_t& block) const;
	void read_m(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_one_item(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_operation(const char * line, int * counter, BinaryOperation * operation) const;
	void read_operation_unary(const char * line, int * counter, UnaryOperation * operation) const;
	void read_p(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_parameter(const char * line, int * counter, double * double_ptr, double * parameters) const;
	void read_parameter_setting(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_q(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_r(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_real_expression(const char * line, int * counter, double * hold2, double * parameters) const;
	void read_real_number(const char * line, int * counter, double * double_ptr) const;
	void read_real_value(const char * line, int * counter, double * double_ptr, double * parameters) const;
	void read_s(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_t(const char * line, int * counter, block_t& block, double * parameters) const;
	static int read_text(const char * command, char * raw_line, char * line, unsigned int * length);
	void read_unary(const char * line, int * counter, double * double_ptr, double * parameters) const;
	void read_x(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_y(const char * line, int * counter, block_t& block, double * parameters) const;
	void read_z(const char * line, int * counter, block_t& block, double * parameters) const;
	void set_probe_data(setup_t& settings);

private:
	virtual void interp_init() =0;

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
	virtual void dwell(double seconds) =0;

	virtual void spindle_start_clockwise() =0;
	virtual void spindle_start_counterclockwise() =0;
	virtual void spindle_stop() =0;
	virtual Direction spindle_state() const =0;
	virtual void spindle_speed(double r) =0;
	virtual double spindle_speed() const =0;
	virtual void spindle_orient(double orientation, Direction direction) =0;

	virtual void tool_length_offset(double length) =0;
	virtual void tool_change(int slot) =0;
	virtual void tool_select(int i) =0;
	virtual int tool_slot() const =0;
	virtual Tool tool(int pocket) const =0;
	virtual unsigned int tool_max() const =0;

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

	virtual void pallet_shuttle() =0;

	virtual void probe_off() =0;
	virtual void probe_on() =0;
	virtual Position probe_position() const =0;
	virtual double probe_value() const =0;

	virtual void program_optional_stop() =0;
	virtual void program_end() =0;
	virtual void program_stop() =0;

	virtual void get_parameter_filename(char* filename, size_t max_size) const =0;
	virtual Position current_position() const =0;
	virtual bool queue_empty() const =0;

public:

	rs274ngc();

	// execute a line of NC code
	int execute();
	// stop running
	void exit();
	// get ready to run
	void init();
	// load a tool table
	void load_tool_table();
	// read the command
	int read(const char * command);
	// reset yourself
	void reset();
	// restore interpreter variables from a file
	void restore_parameters(const char * filename);
	// save interpreter variables to file
	void save_parameters(const char * filename, const double parameters[]);
	// synchronize your internal model with the external world
	void synch();

	// copy active G codes into array [0]..[11]
	void active_g_codes(int * codes);
	// copy active M codes into array [0]..[6]
	void active_m_codes(int * codes);
	// copy active F, S settings into array [0]..[2]
	void active_settings(double * settings);

	// return the length of the most recently read line
	unsigned int line_length();
	// copy the text of the most recently read line into the line_text array,
	// but stop at max_size if the text is longer
	void line_text(char * line_text, unsigned int max_size);

};

#endif
