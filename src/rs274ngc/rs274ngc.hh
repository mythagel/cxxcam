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

   /*
     rs274ngc.hh

   Declarations for the rs274abc translator.

   */

   /**********************/
   /* INCLUDE DIRECTIVES */
   /**********************/

#include "canon.hh"
#include <cstddef>
#include <exception>

   /**********************/
   /*   COMPILER MACROS  */
   /**********************/

#define RS274NGC_TEXT_SIZE 256

   // array sizes
#define RS274NGC_ACTIVE_G_CODES 12
#define RS274NGC_ACTIVE_M_CODES 7
#define RS274NGC_ACTIVE_SETTINGS 3

   // number of parameters in parameter table
#define RS274NGC_MAX_PARAMETERS 5400

   /**********************/
   /*      TYPEDEFS      */
   /**********************/

   /* distance_mode */
enum DISTANCE_MODE {MODE_ABSOLUTE, MODE_INCREMENTAL};

   /* retract_mode for cycles */
enum RETRACT_MODE {R_PLANE, OLD_Z};

   // on-off switch settings
enum ON_OFF : bool
{
	OFF = false,
	ON = true
};

struct block_t
{
    ON_OFF   a_flag;
    double   a_number;
    ON_OFF   b_flag;
    double   b_number;
    ON_OFF   c_flag;
    double   c_number;
    
    char     comment[256];
    int      d_number;
    double   f_number;
    int      g_modes[14];
    int      h_number;
    
    ON_OFF   i_flag;
    double   i_number;
    ON_OFF   j_flag;
    double   j_number;
    ON_OFF   k_flag;
    double   k_number;
    
    int      l_number;
    int      line_number;
    int      motion_to_be;
    int      m_count;
    int      m_modes[10];
    double   p_number;
    double   q_number;
    ON_OFF   r_flag;
    double   r_number;
    double   s_number;
    int      t_number;
    
    ON_OFF   x_flag;
    double   x_number;
    ON_OFF   y_flag;
    double   y_number;
    ON_OFF   z_flag;
    double   z_number;
};

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

struct setup_t
{
	struct position_t
	{
		double x;
		double y;
		double z;
		double a;
		double b;
		double c;
		
		position_t()
		 : x(), y(), z(), a(), b(), c()
		{}
	};

    position_t axis_offset; // g92offset
    position_t current;
    position_t origin_offset;
    
    int active_g_codes [RS274NGC_ACTIVE_G_CODES];                // array of active G codes
    int active_m_codes [RS274NGC_ACTIVE_M_CODES];                // array of active M codes
    double active_settings [RS274NGC_ACTIVE_SETTINGS];               // array of feed, speed, etc.
    block_t block1;                                 // parsed next block
    char blocktext[RS274NGC_TEXT_SIZE];           // linetext downcased, white space gone
    CANON_MOTION_MODE control_mode;               // exact path or cutting mode
    int current_slot;                             // carousel slot number of current tool
    double cutter_comp_radius;                    // current cutter compensation radius
    int cutter_comp_side;                         // current cutter compensation side
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
    DISTANCE_MODE distance_mode;                  // absolute or incremental
    int feed_mode;                                // G_93 (inverse time) or G_94 units/min
    ON_OFF feed_override;                         // whether feed override is enabled
    double feed_rate;                             // feed rate in current units/min
    ON_OFF flood;                                 // whether flood coolant is on
    int length_offset_index;                      // for use with tool length offsets
    CANON_UNITS length_units;                     // millimeters or inches
    int line_length;                              // length of line last read
    char linetext[RS274NGC_TEXT_SIZE];            // text of most recent line read
    ON_OFF mist;                                  // whether mist coolant is on
    int motion_mode;                              // active G-code for motion
    int origin_index;                             // active origin (1=G54 to 9=G59.3)
    double parameters [RS274NGC_MAX_PARAMETERS];                // system parameters
    int parameter_occurrence;                     // parameter buffer index
    int parameter_numbers[50];                    // parameter number buffer
    double parameter_values[50];                  // parameter value buffer
    CANON_PLANE plane;                            // active plane, XY-, YZ-, or XZ-plane
    ON_OFF probe_flag;                            // flag indicating probing done
    double program_x;                             // program x, used when cutter comp on
    double program_y;                             // program y, used when cutter comp on
    RETRACT_MODE retract_mode;                    // for cycles, old_z or r_plane
    int selected_tool_slot;                       // tool slot selected but not active
    int sequence_number;                          // sequence number of line last read
    double speed;                                 // current spindle speed in rpm
    CANON_SPEED_FEED_MODE speed_feed_mode;        // independent or synched
    ON_OFF speed_override;                        // whether speed override is enabled
    CANON_DIRECTION spindle_turning;              // direction spindle is turning
    double tool_length_offset;                    // current tool length offset
    int tool_max;                                 // highest number tool slot in carousel
    CANON_TOOL_TABLE tool_table [CANON_TOOL_MAX + 1];                     // index is slot number
    int tool_table_index;                         // tool index used with cutter comp
    double traverse_rate;                         // rate for traverse motions
};

   /*************************************************************************/
   /*

   Interface functions to call to tell the interpreter what to do.
   Return values indicate status of execution.
   These functions may change the state of the interpreter.

   */


class rs274ngc
{
public:
	struct error : std::exception
	{
		int code;
		error(int code);
		virtual const char* what() const noexcept;
		virtual ~error() noexcept;
	};
private:
    setup_t _setup;

	// pointer to function that reads
	typedef int (rs274ngc::*read_function_pointer) (char *, int *, block_t&, double *);
	read_function_pointer _readers[127];

	int arc_data_comp_ijk(int move, int side, double tool_radius, double current_x, double current_y, double end_x, double end_y, double i_number, double j_number, double * center_x, double * center_y, int * turn, double tolerance);
	int arc_data_comp_r(int move, int side, double tool_radius, double current_x, double current_y, double end_x, double end_y, double big_radius, double * center_x, double * center_y, int * turn);
	int arc_data_ijk(int move, double current_x, double current_y, double end_x, double end_y, double i_number, double j_number, double * center_x, double * center_y, int * turn, double tolerance);
	int arc_data_r(int move, double current_x, double current_y, double end_x, double end_y, double radius, double * center_x, double * center_y, int * turn);
	int check_g_codes(block_t& block, setup_t& settings);
	int check_items(block_t& block, setup_t& settings);
	int check_m_codes(block_t& block);
	int check_other_codes(block_t& block);
	int close_and_downcase(char * line);
	int convert_arc(int move, block_t& block, setup_t& settings);
	int convert_arc2(int move, block_t& block, setup_t& settings, double * current1, double * current2, double * current3, double end1, double end2, double end3, double AA_end, double BB_end, double CC_end, double offset1, double offset2);
	int convert_arc_comp1(int move, block_t& block, setup_t& settings, double end_x, double end_y, double end_z, double AA_end, double BB_end, double CC_end);
	int convert_arc_comp2(int move, block_t& block, setup_t& settings, double end_x, double end_y, double end_z, double AA_end, double BB_end, double CC_end);
	int convert_axis_offsets(int g_code, block_t& block, setup_t& settings);
	int convert_comment(char * comment);
	int convert_control_mode(int g_code, setup_t& settings);
	int convert_coordinate_system(int g_code, setup_t& settings);
	int convert_cutter_compensation(int g_code, block_t& block, setup_t& settings);
	int convert_cutter_compensation_off(setup_t& settings);
	int convert_cutter_compensation_on(int side, block_t& block, setup_t& settings);
	int convert_cycle(int motion, block_t& block, setup_t& settings);
	int convert_cycle_g81(CANON_PLANE plane, double x, double y, double clear_z, double bottom_z);
	int convert_cycle_g82(CANON_PLANE plane, double x, double y, double clear_z, double bottom_z, double dwell);
	int convert_cycle_g83(CANON_PLANE plane, double x, double y, double r, double clear_z, double bottom_z, double delta);
	int convert_cycle_g84(CANON_PLANE plane, double x, double y, double clear_z, double bottom_z, CANON_DIRECTION direction, CANON_SPEED_FEED_MODE mode);
	int convert_cycle_g85(CANON_PLANE plane, double x, double y, double clear_z, double bottom_z);
	int convert_cycle_g86(CANON_PLANE plane, double x, double y, double clear_z, double bottom_z, double dwell, CANON_DIRECTION direction);
	int convert_cycle_g87(CANON_PLANE plane, double x, double offset_x, double y, double offset_y, double r, double clear_z, double middle_z, double bottom_z, CANON_DIRECTION direction);
	int convert_cycle_g88(CANON_PLANE plane, double x, double y, double bottom_z, double dwell, CANON_DIRECTION direction);
	int convert_cycle_g89(CANON_PLANE plane, double x, double y, double clear_z, double bottom_z, double dwell);
	int convert_cycle_xy(int motion, block_t& block, setup_t& settings);
	int convert_cycle_yz(int motion, block_t& block, setup_t& settings);
	int convert_cycle_zx(int motion, block_t& block, setup_t& settings);
	int convert_distance_mode(int g_code, setup_t& settings);
	int convert_dwell(double time);
	int convert_feed_mode(int g_code, setup_t& settings);
	int convert_feed_rate(block_t& block, setup_t& settings);
	int convert_g(block_t& block, setup_t& settings);
	int convert_home(int move, block_t& block, setup_t& settings);
	int convert_length_units(int g_code, setup_t& settings);
	int convert_m(block_t& block, setup_t& settings);
	int convert_modal_0(int code, block_t& block, setup_t& settings);
	int convert_motion(int motion, block_t& block, setup_t& settings);
	int convert_probe(block_t& block, setup_t& settings);
	int convert_retract_mode(int g_code, setup_t& settings);
	int convert_setup(block_t& block, setup_t& settings);
	int convert_set_plane(int g_code, setup_t& settings);
	int convert_speed(block_t& block, setup_t& settings);
	int convert_stop(block_t& block, setup_t& settings);
	int convert_straight(int move, block_t& block, setup_t& settings);
	int convert_straight_comp1(int move, block_t& block, setup_t& settings, double px, double py, double end_z, double AA_end, double BB_end, double CC_end);
	int convert_straight_comp2(int move, block_t& block, setup_t& settings, double px, double py, double end_z, double AA_end, double BB_end, double CC_end);
	int convert_tool_change(setup_t& settings);
	int convert_tool_length_offset(int g_code, block_t& block, setup_t& settings);
	int convert_tool_select(block_t& block, setup_t& settings);
	int cycle_feed(CANON_PLANE plane, double end1, double end2, double end3);
	int cycle_traverse(CANON_PLANE plane, double end1, double end2, double end3);
	int enhance_block(block_t& block, setup_t& settings);
	int execute_binary(double * left, int operation, double * right);
	int execute_binary1(double * left, int operation, double * right);
	int execute_binary2(double * left, int operation, double * right);
	int execute_block(block_t& block, setup_t& settings);
	int execute_unary(double * double_ptr, int operation);
	double find_arc_length(double x1, double y1, double z1, double center_x, double center_y, int turn, double x2, double y2, double z2);
	int find_ends(block_t& block, setup_t& settings, double * px, double * py, double * pz, double * AA_p, double * BB_p, double * CC_p);
	int find_relative(double x1, double y1, double z1, double AA_1, double BB_1, double CC_1, double * x2, double * y2, double * z2, double * AA_2, double * BB_2, double * CC_2,setup_t& settings);
	double find_straight_length(double x2, double y2, double z2, double AA_2, double BB_2, double CC_2, double x1, double y1, double z1, double AA_1, double BB_1, double CC_1);
	double find_turn(double x1, double y1, double center_x, double center_y, int turn, double x2, double y2);
	int init_block(block_t& block);
	int inverse_time_rate_arc(double x1, double y1, double z1, double cx, double cy, int turn, double x2, double y2, double z2, block_t& block, setup_t& settings);
	int inverse_time_rate_arc2(double start_x, double start_y, int turn1, double mid_x, double mid_y, double cx, double cy, int turn2, double end_x, double end_y, double end_z, block_t& block, setup_t& settings);
	int inverse_time_rate_as(double start_x, double start_y, int turn, double mid_x, double mid_y, double end_x, double end_y, double end_z, double AA_end, double BB_end, double CC_end, block_t& block, setup_t& settings);
	int inverse_time_rate_straight(double end_x, double end_y, double end_z, double AA_end, double BB_end, double CC_end, block_t& block, setup_t& settings);
	int parse_line(char * line, block_t& block,setup_t& settings);
	int precedence(int an_operator);
	int read_a(char * line, int * counter, block_t& block, double * parameters);
	int read_atan(char * line, int * counter, double * double_ptr, double * parameters);
	int read_b(char * line, int * counter, block_t& block, double * parameters);
	int read_c(char * line, int * counter, block_t& block, double * parameters);
	int read_comment(char * line, int * counter, block_t& block, double * parameters);
	int read_d(char * line, int * counter, block_t& block, double * parameters);
	int read_f(char * line, int * counter, block_t& block, double * parameters);
	int read_g(char * line, int * counter, block_t& block, double * parameters);
	int read_h(char * line, int * counter, block_t& block, double * parameters);
	int read_i(char * line, int * counter, block_t& block, double * parameters);
	int read_integer_unsigned(char * line, int * counter, int * integer_ptr);
	int read_integer_value(char * line, int * counter, int * integer_ptr, double * parameters);
	int read_items(block_t& block, char * line, double * parameters);
	int read_j(char * line, int * counter, block_t& block, double * parameters);
	int read_k(char * line, int * counter, block_t& block, double * parameters);
	int read_l(char * line, int * counter, block_t& block, double * parameters);
	int read_line_number(char * line, int * counter, block_t& block);
	int read_m(char * line, int * counter, block_t& block, double * parameters);
	int read_one_item(char * line, int * counter, block_t& block, double * parameters);
	int read_operation(char * line, int * counter, int * operation);
	int read_operation_unary(char * line, int * counter, int * operation);
	int read_p(char * line, int * counter, block_t& block, double * parameters);
	int read_parameter(char * line, int * counter, double * double_ptr, double * parameters);
	int read_parameter_setting(char * line, int * counter, block_t& block, double * parameters);
	int read_q(char * line, int * counter, block_t& block, double * parameters);
	int read_r(char * line, int * counter, block_t& block, double * parameters);
	int read_real_expression(char * line, int * counter, double * hold2, double * parameters);
	int read_real_number(char * line, int * counter, double * double_ptr);
	int read_real_value(char * line, int * counter, double * double_ptr, double * parameters);
	int read_s(char * line, int * counter, block_t& block, double * parameters);
	int read_t(char * line, int * counter, block_t& block, double * parameters);
	int read_text(const char * command, char * raw_line, char * line, int * length);
	int read_unary(char * line, int * counter, double * double_ptr, double * parameters);
	int read_x(char * line, int * counter, block_t& block, double * parameters);
	int read_y(char * line, int * counter, block_t& block, double * parameters);
	int read_z(char * line, int * counter, block_t& block, double * parameters);
	int set_probe_data(setup_t& settings);
	int write_g_codes(const block_t* block, setup_t& settings);
	int write_m_codes(const block_t* block, setup_t& settings);
	int write_settings(setup_t& settings);

public:

	rs274ngc();

	   // execute a line of NC code
	int execute();

	   // stop running
	int exit();

	   // get ready to run
	int init();

	   // load a tool table
	int load_tool_table();

	   // read the command
	int read(const char * command);

	   // reset yourself
	int reset();

	   // restore interpreter variables from a file
	int restore_parameters(const char * filename);

	   // save interpreter variables to file
	int save_parameters(const char * filename, const double parameters[]);

	   // synchronize your internal model with the external world
	int synch();

	   /*************************************************************************/
	   /* 

	   Interface functions to call to get information from the interpreter.
	   If a function has a return value, the return value contains the information.
	   If a function returns nothing, information is copied into one of the
	   arguments to the function. These functions do not change the state of
	   the interpreter.

	   */

	   // copy active G codes into array [0]..[11]
	void active_g_codes(int * codes);

	   // copy active M codes into array [0]..[6]
	void active_m_codes(int * codes);

	   // copy active F, S settings into array [0]..[2]
	void active_settings(double * settings);

	   // copy the text of the error message whose number is error_code into the
	   // error_text array, but stop at max_size if the text is longer.
	void error_text(int error_code, char * error_text, size_t max_size);

	   // return the length of the most recently read line
	int line_length();

	   // copy the text of the most recently read line into the line_text array,
	   // but stop at max_size if the text is longer
	void line_text(char * line_text, size_t max_size);

	   // return the current sequence number (how many lines read)
	int sequence_number();

};

#endif
