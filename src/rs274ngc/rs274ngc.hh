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

#include <cstdio>
#include "canon.hh"

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
enum ON_OFF {OFF, ON};

struct block
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

typedef block * block_pointer;

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

struct setup
{
    double AA_axis_offset;                        // A-axis g92 offset
    double AA_current;                            // current A-axis position
    double AA_origin_offset;                      // A-axis origin offset
    double BB_axis_offset;                        // B-axis g92offset
    double BB_current;                            // current B-axis position
    double BB_origin_offset;                      // B-axis origin offset
    double CC_axis_offset;                        // C-axis g92offset
    double CC_current;                            // current C-axis position
    double CC_origin_offset;                      // C-axis origin offset
    int active_g_codes
        [RS274NGC_ACTIVE_G_CODES];                // array of active G codes
    int active_m_codes
        [RS274NGC_ACTIVE_M_CODES];                // array of active M codes
    double active_settings
        [RS274NGC_ACTIVE_SETTINGS];               // array of feed, speed, etc.
    double axis_offset_x;                         // X-axis g92 offset
    double axis_offset_y;                         // Y-axis g92 offset
    double axis_offset_z;                         // Z-axis g92 offset
    block block1;                                 // parsed next block
    char blocktext[RS274NGC_TEXT_SIZE];           // linetext downcased, white space gone
    CANON_MOTION_MODE control_mode;               // exact path or cutting mode
    int current_slot;                             // carousel slot number of current tool
    double current_x;                             // current X-axis position
    double current_y;                             // current Y-axis position
    double current_z;                             // current Z-axis position
    double cutter_comp_radius;                    // current cutter compensation radius
    int cutter_comp_side;                         // current cutter compensation side
    double cycle_cc;                              // cc-value (normal) for canned cycles
    double cycle_i;                               // i-value for canned cycles
    double cycle_j;                               // j-value for canned cycles
    double cycle_k;                               // k-value for canned cycles
    int cycle_l;                                  // l-value for canned cycles
    double cycle_p;                               // p-value (dwell) for canned cycles
    double cycle_q;                               // q-value for canned cycles
    double cycle_r;                               // r-value for canned cycles
    DISTANCE_MODE distance_mode;                  // absolute or incremental
    int feed_mode;                                // G_93 (inverse time) or G_94 units/min
    ON_OFF feed_override;                         // whether feed override is enabled
    double feed_rate;                             // feed rate in current units/min
    char filename[RS274NGC_TEXT_SIZE];            // name of currently open NC code file
    FILE * file_pointer;                          // file pointer for open NC code file
    ON_OFF flood;                                 // whether flood coolant is on
    int length_offset_index;                      // for use with tool length offsets
    CANON_UNITS length_units;                     // millimeters or inches
    int line_length;                              // length of line last read
    char linetext[RS274NGC_TEXT_SIZE];            // text of most recent line read
    ON_OFF mist;                                  // whether mist coolant is on
    int motion_mode;                              // active G-code for motion
    int origin_index;                             // active origin (1=G54 to 9=G59.3)
    double origin_offset_x;                       // origin offset x
    double origin_offset_y;                       // origin offset y
    double origin_offset_z;                       // origin offset z
    double parameters
        [RS274NGC_MAX_PARAMETERS];                // system parameters
    int parameter_occurrence;                     // parameter buffer index
    int parameter_numbers[50];                    // parameter number buffer
    double parameter_values[50];                  // parameter value buffer
    ON_OFF percent_flag;                          // ON means first line was percent sign
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
    char stack[50][80];                           // stack of calls for error reporting
    int stack_index;                              // index into the stack
    double tool_length_offset;                    // current tool length offset
    int tool_max;                                 // highest number tool slot in carousel
    CANON_TOOL_TABLE tool_table
        [CANON_TOOL_MAX + 1];                     // index is slot number
    int tool_table_index;                         // tool index used with cutter comp
    double traverse_rate;                         // rate for traverse motions
};

typedef setup * setup_pointer;

   // pointer to function that reads
typedef int (*read_function_pointer) (char *, int *, block_pointer, double *);

   /*************************************************************************/
   /*

   Interface functions to call to tell the interpreter what to do.
   Return values indicate status of execution.
   These functions may change the state of the interpreter.

   */

   // close the currently open NC code file
extern int rs274ngc_close();

   // execute a line of NC code
extern int rs274ngc_execute();

   // stop running
extern int rs274ngc_exit();

   // get ready to run
extern int rs274ngc_init();

   // load a tool table
extern int rs274ngc_load_tool_table();

   // open a file of NC code
extern int rs274ngc_open(const char *filename);

   // read the mdi or the next line of the open NC code file
extern int rs274ngc_read(const char * mdi = 0);

   // reset yourself
extern int rs274ngc_reset();

   // restore interpreter variables from a file
extern int rs274ngc_restore_parameters(const char * filename);

   // save interpreter variables to file
extern int rs274ngc_save_parameters(const char * filename, const double parameters[]);

   // synchronize your internal model with the external world
extern int rs274ngc_synch();

   /*************************************************************************/
   /* 

   Interface functions to call to get information from the interpreter.
   If a function has a return value, the return value contains the information.
   If a function returns nothing, information is copied into one of the
   arguments to the function. These functions do not change the state of
   the interpreter.

   */

   // copy active G codes into array [0]..[11]
void rs274ngc_active_g_codes(int * codes);

   // copy active M codes into array [0]..[6]
void rs274ngc_active_m_codes(int * codes);

   // copy active F, S settings into array [0]..[2]
void rs274ngc_active_settings(double * settings);

   // copy the text of the error message whose number is error_code into the
   // error_text array, but stop at max_size if the text is longer.
void rs274ngc_error_text(int error_code, char * error_text, size_t max_size);

   // copy the name of the currently open file into the file_name array,
   // but stop at max_size if the name is longer
void rs274ngc_file_name(char * file_name, size_t max_size);

   // return the length of the most recently read line
int rs274ngc_line_length();

   // copy the text of the most recently read line into the line_text array,
   // but stop at max_size if the text is longer
void rs274ngc_line_text(char * line_text, size_t max_size);

   // return the current sequence number (how many lines read)
int rs274ngc_sequence_number();

   // copy the function name from the stack_index'th position of the
   // function call stack at the time of the most recent error into
   // the function name string, but stop at max_size if the name is longer
void rs274ngc_stack_name(int stack_index, char * function_name, int max_size);

#endif
