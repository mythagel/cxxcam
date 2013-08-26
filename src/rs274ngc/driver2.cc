#include "rs274ngc.hh"
#include "rs274ngc_return.hh"
#include <iostream>
#include <string>
#include <cmath>
#include <cstring>

class interpreter : public rs274ngc
{
private:
	virtual void interp_init()
	{
		_parameter_file_name[0] = 0;
		_spindle_speed = 0;
		_spindle_turning = Direction::Stop;
		_traverse_rate = 60;
	}

	Plane       _active_plane = Plane::XY;
	int               _active_slot = 1;
	double            _feed_rate = 0.0;
	int               _flood = 0;
	double            _length_unit_factor = 1; /* 1 for MM 25.4 for inch */
	Units       _length_unit_type = Units::Metric;
	int               _line_number = 1;
	int               _mist = 0;
	Motion _motion_mode = Motion::Continious;

	char                     _parameter_file_name[100];
	double            _probe_position_a = 0;   /*AA*/
	double            _probe_position_b = 0;   /*BB*/
	double            _probe_position_c = 0;   /*CC*/
	double            _probe_position_x = 0;
	double            _probe_position_y = 0;
	double            _probe_position_z = 0;
	double            _program_origin_a = 0;   /*AA*/
	double            _program_origin_b = 0;   /*BB*/
	double            _program_origin_c = 0;   /*CC*/
	double            _program_origin_x = 0;
	double            _program_origin_y = 0;
	double            _program_origin_z = 0;
	double            _program_position_a = 0; /*AA*/
	double            _program_position_b = 0; /*BB*/
	double            _program_position_c = 0; /*CC*/
	double            _program_position_x = 0;
	double            _program_position_y = 0;
	double            _program_position_z = 0;
	double            _spindle_speed;
	Direction   _spindle_turning;
	int                      _tool_max = 68;          /*Not static. Driver reads  */
	Tool         _tools[CANON_TOOL_MAX];  /*Not static. Driver writes */
	double            _traverse_rate;
public:

void print_nc_line_number()
{
    char text[256];
    int k;
    int m;

    line_text(text, 256);
    for (k = 0;
        ((k < 256) and
        ((text[k] == '\t') or (text[k] == ' ') or (text[k] == '/')));
        k++);
    if ((k < 256) and ((text[k] == 'n') or (text[k] == 'N')))
    {
        fputc('N', stdout);
        for (k++, m = 0;
            ((k < 256) and (text[k] >= '0') and (text[k] <= '9'));
            k++, m++)
        fputc(text[k], stdout);
        for (; m < 6; m++)
            fputc(' ', stdout);
    }
    else if (k < 256)
        fprintf(stdout, "N..... ");
}


#define PRINT0(control) do \
{ \
    fprintf(stdout, "%5d ", _line_number++); \
    print_nc_line_number(); \
    fprintf(stdout, control); \
} while(0)
#define PRINT1(control, arg1) do \
{ \
    fprintf(stdout, "%5d ", _line_number++); \
    print_nc_line_number(); \
    fprintf(stdout, control, arg1); \
} while(0)
#define PRINT2(control, arg1, arg2) do \
{ \
    fprintf(stdout, "%5d ", _line_number++); \
    print_nc_line_number(); \
    fprintf(stdout, control, arg1, arg2); \
} while(0)
#define PRINT3(control, arg1, arg2, arg3) do \
{ \
    fprintf(stdout, "%5d ", _line_number++); \
    print_nc_line_number(); \
    fprintf(stdout, control, arg1, arg2, arg3); \
} while(0)
#define PRINT4(control, arg1, arg2, arg3, arg4) do \
{ \
    fprintf(stdout, "%5d ", _line_number++); \
    print_nc_line_number(); \
    fprintf(stdout, control, arg1, arg2, arg3, arg4); \
} while(0)
#define PRINT5(control, arg1, arg2, arg3, arg4, arg5) do \
{ \
    fprintf(stdout, "%5d ", _line_number++); \
    print_nc_line_number(); \
    fprintf(stdout, control, arg1, arg2, arg3, arg4, arg5); \
} while(0)
#define PRINT6(control, arg1, arg2, arg3, arg4, arg5, arg6) do \
{ \
    fprintf(stdout, "%5d ", _line_number++); \
    print_nc_line_number(); \
    fprintf(stdout, control, arg1, arg2, arg3, arg4, arg5, arg6); \
} while(0)
#define PRINT7(control, arg1, arg2, arg3, arg4, arg5, arg6, arg7) do \
{ \
    fprintf(stdout, "%5d ", _line_number++); \
    print_nc_line_number(); \
    fprintf(stdout, control, \
    arg1, arg2, arg3, arg4, arg5, arg6, arg7); \
} while(0)
#define PRINT10(control,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10) \
do \
{ \
    fprintf(stdout, "%5d ", _line_number++); \
    print_nc_line_number(); \
    fprintf(stdout, control, \
    arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10); \
} while(0)

   /* Representation */

virtual void offset_origin(const Position& pos)
{
    fprintf(stdout, "%5d ", _line_number++);
    print_nc_line_number();
    fprintf(stdout, "SET_ORIGIN_OFFSETS(%.4f, %.4f, %.4f"
        ", %.4f"                                  /*AA*/
        ", %.4f"                                  /*BB*/
        ", %.4f"                                  /*CC*/
        ")\n", pos.x, pos.y, pos.z
        , pos.a                                       /*AA*/
        , pos.b                                       /*BB*/
        , pos.c                                       /*CC*/
        );
    _program_position_x = _program_position_x + _program_origin_x - pos.x;
    _program_position_y = _program_position_y + _program_origin_y - pos.y;
    _program_position_z = _program_position_z + _program_origin_z - pos.z;
    _program_position_a = _program_position_a + _program_origin_a - pos.a;
    _program_position_b = _program_position_b + _program_origin_b - pos.b;
    _program_position_c = _program_position_c + _program_origin_c - pos.c;

    _program_origin_x = pos.x;
    _program_origin_y = pos.y;
    _program_origin_z = pos.z;
    _program_origin_a = pos.a;                   /*AA*/
    _program_origin_b = pos.b;                   /*BB*/
    _program_origin_c = pos.c;                   /*CC*/
}


virtual void units(Units u)
{
    if (u == Units::Imperial)
    {
        PRINT0("USE_LENGTH_UNITS(CANON_UNITS_INCHES)\n");
        if (_length_unit_type == Units::Metric)
        {
            _length_unit_type = Units::Imperial;
            _length_unit_factor = 25.4;
            _program_origin_x = (_program_origin_x / 25.4);
            _program_origin_y = (_program_origin_y / 25.4);
            _program_origin_z = (_program_origin_z / 25.4);
            _program_position_x = (_program_position_x / 25.4);
            _program_position_y = (_program_position_y / 25.4);
            _program_position_z = (_program_position_z / 25.4);
        }
    }
    else if (u == Units::Metric)
    {
        PRINT0("USE_LENGTH_UNITS(CANON_UNITS_MM)\n");
        if (_length_unit_type == Units::Imperial)
        {
            _length_unit_type = Units::Metric;
            _length_unit_factor = 1.0;
            _program_origin_x = (_program_origin_x * 25.4);
            _program_origin_y = (_program_origin_y * 25.4);
            _program_origin_z = (_program_origin_z * 25.4);
            _program_position_x = (_program_position_x * 25.4);
            _program_position_y = (_program_position_y * 25.4);
            _program_position_z = (_program_position_z * 25.4);
        }
    }
    else
        PRINT0("USE_LENGTH_UNITS(UNKNOWN)\n");
}


   /* Free Space Motion */
virtual void rapid_rate(double rate)
{
    PRINT1("SET_TRAVERSE_RATE(%.4f)\n", rate);
    _traverse_rate = rate;
}


virtual void rapid(const Position& pos)
{
    fprintf(stdout, "%5d ", _line_number++);
    print_nc_line_number();
    fprintf(stdout, "STRAIGHT_TRAVERSE(%.4f, %.4f, %.4f"
        ", %.4f"                                  /*AA*/
        ", %.4f"                                  /*BB*/
        ", %.4f"                                  /*CC*/
        ")\n", pos.x, pos.y, pos.z
        , pos.a                                       /*AA*/
        , pos.b                                       /*BB*/
        , pos.c                                       /*CC*/
        );
    _program_position_x = pos.x;
    _program_position_y = pos.y;
    _program_position_z = pos.z;
    _program_position_a = pos.a;                 /*AA*/
    _program_position_b = pos.b;                 /*BB*/
    _program_position_c = pos.c;                 /*CC*/
}


   /* Machining Attributes */
virtual void feed_rate(double rate)
{
    PRINT1("SET_FEED_RATE(%.4f)\n", rate);
    _feed_rate = rate;
}


virtual void feed_reference(FeedReference reference)
{
    PRINT1("SET_FEED_REFERENCE(%s)\n",
        (reference == FeedReference::Workpiece) ? "CANON_WORKPIECE" : "CANON_XYZ");
}


virtual void motion_mode(Motion mode)
{
    if (mode == Motion::Exact_Stop)
    {
        PRINT0("SET_MOTION_CONTROL_MODE(CANON_EXACT_STOP)\n");
        _motion_mode = Motion::Exact_Stop;
    }
    else if (mode == Motion::Exact_Path)
    {
        PRINT0("SET_MOTION_CONTROL_MODE(CANON_EXACT_PATH)\n");
        _motion_mode = Motion::Exact_Path;
    }
    else if (mode == Motion::Continious)
    {
        PRINT0("SET_MOTION_CONTROL_MODE(CANON_CONTINUOUS)\n");
        _motion_mode = Motion::Continious;
    }
    else
        PRINT0("SET_MOTION_CONTROL_MODE(UNKNOWN)\n");
}


virtual void plane(Plane pl)
{
    PRINT1("SELECT_PLANE(CANON_PLANE_%s)\n",
        ((pl == Plane::XY) ? "XY" :
    (pl == Plane::YZ) ? "YZ" :
    (pl == Plane::XZ) ? "XZ" : "UNKNOWN"));
    _active_plane = pl;
}


virtual void cutter_radius_comp(double radius)
{PRINT1("SET_CUTTER_RADIUS_COMPENSATION(%.4f)\n", radius);}

virtual void cutter_radius_comp_start(Side direction)
{
    PRINT1("START_CUTTER_RADIUS_COMPENSATION(%s)\n",
        (direction == Side::Left)  ? "LEFT"  :
    (direction == Side::Right) ? "RIGHT" : "UNKNOWN");
}


virtual void cutter_radius_comp_stop()
{PRINT0 ("STOP_CUTTER_RADIUS_COMPENSATION()\n");}

virtual void speed_feed_sync_start()
{PRINT0 ("START_SPEED_FEED_SYNCH()\n");}

virtual void speed_feed_sync_stop()
{PRINT0 ("STOP_SPEED_FEED_SYNCH()\n");}

   /* Machining Functions */

virtual void arc(double end0, double end1, double axis0, double axis1, int rotation, double end_point, double a, double b, double c)
{
    fprintf(stdout, "%5d ", _line_number++);
    print_nc_line_number();
    fprintf(stdout, "ARC_FEED(%.4f, %.4f, %.4f, %.4f, %d, %.4f"
        ", %.4f"                                  /*AA*/
        ", %.4f"                                  /*BB*/
        ", %.4f"                                  /*CC*/
        ")\n", end0, end1, axis0, axis1,
        rotation, end_point
        , a                                       /*AA*/
        , b                                       /*BB*/
        , c                                       /*CC*/
        );
    if (_active_plane == Plane::XY)
    {
        _program_position_x = end0;
        _program_position_y = end1;
        _program_position_z = end_point;
    }
    else if (_active_plane == Plane::YZ)
    {
        _program_position_x = end_point;
        _program_position_y = end0;
        _program_position_z = end1;
    }
    else                                          /* if (_active_plane == CANON_PLANE_XZ) */
    {
        _program_position_x = end1;
        _program_position_y = end_point;
        _program_position_z = end0;
    }
    _program_position_a = a;                 /*AA*/
    _program_position_b = b;                 /*BB*/
    _program_position_c = c;                 /*CC*/
}


virtual void linear(const Position& pos)
{
    fprintf(stdout, "%5d ", _line_number++);
    print_nc_line_number();
    fprintf(stdout, "STRAIGHT_FEED(%.4f, %.4f, %.4f"
        ", %.4f"                                  /*AA*/
        ", %.4f"                                  /*BB*/
        ", %.4f"                                  /*CC*/
        ")\n", pos.x, pos.y, pos.z
        , pos.a                                       /*AA*/
        , pos.b                                       /*BB*/
        , pos.c                                       /*CC*/
        );
    _program_position_x = pos.x;
    _program_position_y = pos.y;
    _program_position_z = pos.z;
    _program_position_a = pos.a;                 /*AA*/
    _program_position_b = pos.b;                 /*BB*/
    _program_position_c = pos.c;                 /*CC*/
}


   /* This models backing the probe off 0.01 inch or 0.254 mm from the probe
   point towards the previous location after the probing, if the probe
   point is not the same as the previous point -- which it should not be. */

virtual void probe(const Position& pos)
{
    double distance;
    double dx, dy, dz;
    double backoff;

    dx = (_program_position_x - pos.x);
    dy = (_program_position_y - pos.y);
    dz = (_program_position_z - pos.z);
    distance = sqrt((dx * dx) + (dy * dy) + (dz * dz));

    fprintf(stdout, "%5d ", _line_number++);
    print_nc_line_number();
    fprintf(stdout, "STRAIGHT_PROBE(%.4f, %.4f, %.4f"
        ", %.4f"                                  /*AA*/
        ", %.4f"                                  /*BB*/
        ", %.4f"                                  /*CC*/
        ")\n", pos.x, pos.y, pos.z
        , pos.a                                       /*AA*/
        , pos.b                                       /*BB*/
        , pos.c                                       /*CC*/
        );
    _probe_position_x = pos.x;
    _probe_position_y = pos.y;
    _probe_position_z = pos.z;
    _probe_position_a = pos.a;                   /*AA*/
    _probe_position_b = pos.b;                   /*BB*/
    _probe_position_c = pos.c;                   /*CC*/
    if (distance == 0)
    {
        _program_position_x = _program_position_x;
        _program_position_y = _program_position_y;
        _program_position_z = _program_position_z;
    }
    else
    {
        backoff = ((_length_unit_type == Units::Metric) ? 0.254 : 0.01);
        _program_position_x = (pos.x + (backoff * (dx / distance)));
        _program_position_y = (pos.y + (backoff * (dy / distance)));
        _program_position_z = (pos.z + (backoff * (dz / distance)));
    }
    _program_position_a = pos.a;                 /*AA*/
    _program_position_b = pos.b;                 /*BB*/
    _program_position_c = pos.c;                 /*CC*/
}


virtual void dwell(double seconds)
{PRINT1("DWELL(%.4f)\n", seconds);}

   /* Spindle Functions */

virtual void spindle_start_clockwise()
{
    PRINT0("START_SPINDLE_CLOCKWISE()\n");
    _spindle_turning = ((_spindle_speed == 0) ? Direction::Stop :
    Direction::Clockwise);
}


virtual void spindle_start_counterclockwise()
{
    PRINT0("START_SPINDLE_COUNTERCLOCKWISE()\n");
    _spindle_turning = ((_spindle_speed == 0) ? Direction::Stop :
    Direction::CounterClockwise);
}


virtual void spindle_speed(double r)
{
    PRINT1("SET_SPINDLE_SPEED(%.4f)\n", r);
    _spindle_speed = r;
}


virtual void spindle_stop()
{
    PRINT0("STOP_SPINDLE_TURNING()\n");
    _spindle_turning = Direction::Stop;
}


virtual void spindle_orient(double orientation, Direction direction)
{
    PRINT2("ORIENT_SPINDLE(%.4f, %s)\n", orientation,
        (direction == Direction::Clockwise) ? "CANON_CLOCKWISE" :
    "CANON_COUNTERCLOCKWISE");
}


   /* Tool Functions */

virtual void tool_length_offset(double length)
{PRINT1("USE_TOOL_LENGTH_OFFSET(%.4f)\n", length);}

virtual void tool_change(int slot)
{
    PRINT1("CHANGE_TOOL(%d)\n", slot);
    _active_slot = slot;
}


virtual void tool_select(int i)
{PRINT1("SELECT_TOOL(%d)\n", i);}

   /* Misc Functions */

virtual void axis_clamp(Axis axis)
{
    PRINT1("CLAMP_AXIS(%s)\n",
        (axis == Axis::X) ? "CANON_AXIS_X" :
    (axis == Axis::Y) ? "CANON_AXIS_Y" :
    (axis == Axis::Z) ? "CANON_AXIS_Z" :
    (axis == Axis::A) ? "CANON_AXIS_A" :
    (axis == Axis::B) ? "CANON_AXIS_B" :
    (axis == Axis::C) ? "CANON_AXIS_C" : "UNKNOWN");
}


virtual void comment(const char *s)
{PRINT1("COMMENT(\"%s\")\n", s);}

virtual void feed_override_disable()
{PRINT0("DISABLE_FEED_OVERRIDE()\n");}

virtual void speed_override_disable()
{PRINT0("DISABLE_SPEED_OVERRIDE()\n");}

virtual void feed_override_enable()
{PRINT0("ENABLE_FEED_OVERRIDE()\n");}

virtual void speed_override_enable()
{PRINT0("ENABLE_SPEED_OVERRIDE()\n");}

virtual void coolant_flood_off()
{
    PRINT0("FLOOD_OFF()\n");
    _flood = 0;
}


virtual void coolant_flood_on()
{
    PRINT0("FLOOD_ON()\n");
    _flood = 1;
}


virtual void message(const char *s)
{PRINT1("MESSAGE(\"%s\")\n", s);}

virtual void coolant_mist_off()
{
    PRINT0("MIST_OFF()\n");
    _mist = 0;
}


virtual void coolant_mist_on()
{
    PRINT0("MIST_ON()\n");
    _mist = 1;
}


virtual void pallet_shuttle()
{PRINT0("PALLET_SHUTTLE()\n");}


virtual void probe_off()
{PRINT0("TURN_PROBE_OFF()\n");}

virtual void probe_on()
{PRINT0("TURN_PROBE_ON()\n");}

virtual void axis_unclamp(Axis axis)
{
    PRINT1("UNCLAMP_AXIS(%s)\n",
        (axis == Axis::X) ? "CANON_AXIS_X" :
    (axis == Axis::Y) ? "CANON_AXIS_Y" :
    (axis == Axis::Z) ? "CANON_AXIS_Z" :
    (axis == Axis::A) ? "CANON_AXIS_A" :
    (axis == Axis::B) ? "CANON_AXIS_B" :
    (axis == Axis::C) ? "CANON_AXIS_C" : "UNKNOWN");
}


   /* Program Functions */

virtual void program_stop()
{PRINT0("PROGRAM_STOP()\n");}

virtual void program_optional_stop()
{PRINT0("OPTIONAL_PROGRAM_STOP()\n");}

virtual void program_end()
{PRINT0("PROGRAM_END()\n");}

   /*************************************************************************/

   /* Canonical "Give me information" functions

   In general, returned values are valid only if any canonical do it commands
   that may have been called for have been executed to completion. If a function
   returns a valid value regardless of execution, that is noted in the comments
   below.

   */

   /* The interpreter is not using this function
   // Returns the system angular unit factor, in units / degree
   extern double GET_EXTERNAL_ANGLE_UNIT_FACTOR()
   {
   return 1;
   }
   */

   /* Returns the system feed rate */
virtual double feed_rate() const
{
    return _feed_rate;
}


   /* Returns the system flood coolant setting zero = off, non-zero = on */
virtual bool coolant_flood() const
{
    return _flood;
}


   /* Returns the system length unit type */
virtual Units units() const
{
    return _length_unit_type;
}


   /* Returns the system mist coolant setting zero = off, non-zero = on */
virtual bool coolant_mist() const
{
    return _mist;
}


   // Returns the current motion control mode
virtual Motion motion_mode() const
{
    return _motion_mode;
}


virtual void get_parameter_filename(char* filename, size_t max_size) const
{
    if (strlen(_parameter_file_name) < max_size)
        strcpy(filename, _parameter_file_name);
    else
        filename[0] = 0;
}


virtual Plane plane() const
{
    return _active_plane;
}


virtual Position current_position() const
{
	return {_program_position_x, _program_position_y, _program_position_z, _program_position_a, _program_position_b, _program_position_c};
}

virtual Position probe_position() const
{
	return {_probe_position_x, _probe_position_y, _probe_position_z, _probe_position_a, _probe_position_b, _probe_position_c};
}

   /* Returns the value for any analog non-contact probing. */
   /* This is a dummy of a dummy, returning a useless value. */
   /* It is not expected this will ever be called. */
virtual double probe_value() const
{
    return 1.0;
}


   /* Returns zero if queue is not empty, non-zero if the queue is empty */
   /* In the stand-alone interpreter, there is no queue, so it is always empty */
virtual bool queue_empty() const
{
    return 1;
}


   /* Returns the system value for spindle speed in rpm */
virtual double spindle_speed() const
{
    return _spindle_speed;
}


   /* Returns the system value for direction of spindle turning */
virtual Direction spindle_state() const
{
    return _spindle_turning;
}


   /* Returns the system value for the carousel slot in which the tool
   currently in the spindle belongs. Return value zero means there is no
   tool in the spindle. */
virtual int tool_slot() const
{
    return _active_slot;
}


   /* Returns maximum number of tools */
virtual unsigned int tool_max() const
{
    return _tool_max;
}


   /* Returns the CANON_TOOL_TABLE structure associated with the tool
      in the given pocket */
virtual Tool tool(int pocket) const
{
    return _tools[pocket];
}


   /* Returns the system traverse rate */
virtual double rapid_rate() const
{
    return _traverse_rate;
}

};


int main()
{
	interpreter interp;
	
	interp.init();

	std::string line;
	while(std::getline(std::cin, line))
	{
		int status;
		
		status = interp.read(line.c_str());
		if(status != RS274NGC_OK)
		{
			if(status != RS274NGC_EXECUTE_FINISH)
			{
				std::cerr << "Error readine line!: \n";
				std::cerr << line <<"\n";
				return status;
			}
		}
		
		status = interp.execute();
		if(status != RS274NGC_OK)
			return status;
	}

    interp.exit();
    return 0;
}

