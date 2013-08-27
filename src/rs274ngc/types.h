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
 * types.h
 *
 *  Created on: 2013-08-27
 *      Author: nicholas
 */

#ifndef TYPES_H_
#define TYPES_H_
#include <cstddef>

static const size_t RS274NGC_TEXT_SIZE = 256;
static const size_t RS274NGC_ACTIVE_G_CODES = 12;
static const size_t RS274NGC_ACTIVE_M_CODES = 7;
static const size_t RS274NGC_ACTIVE_SETTINGS = 3;
static const size_t RS274NGC_MAX_PARAMETERS = 5400;

/* numerical constants */
static const double TOLERANCE_INCH = 0.0002;
static const double TOLERANCE_MM  = 0.002;
/* angle threshold for concavity for cutter compensation, in radians */
static const double TOLERANCE_CONCAVE_CORNER = 0.01;

static const double TINY = 1e-12;                                /* for arc_data_r */
static const double UNKNOWN = 1e-20;
static const double TWO_PI =  6.2831853071795864;
static const double PI =      3.1415926535897932;
static const double PI2 =     1.5707963267948966;

   // English - Metric conversion (long number keeps error buildup down)
static const double MM_PER_INCH = 25.4;
static const double INCH_PER_MM = 0.039370078740157477;

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

enum class FeedMode
{
	UnitsPerMinute,
	InverseTime
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

static const size_t CANON_TOOL_MAX = 128;                        // max size of carousel handled
static const size_t CANON_TOOL_ENTRY_LEN = 256;                  // how long each file line can be

struct Tool
{
    int id;
    double length;
    double diameter;
    
    Tool();
};

struct Position
{
	double x;
	double y;
	double z;
	double a;
	double b;
	double c;
	
	Position();
	Position(double x, double y, double z, double a, double b, double c);
	Position(double x, double y, double z);
	
	Position operator+(const Position& p) const;
	Position operator-(const Position& p) const;
};

enum class DistanceMode
{
	Absolute,
	Incremental
};

   /* retract_mode for cycles */
enum RETRACT_MODE {R_PLANE, OLD_Z};

#endif /* TYPES_H_ */
