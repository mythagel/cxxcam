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
 * arc.cpp
 *
 *  Created on: 2013-08-27
 *      Author: nicholas
 */

#include "arc.h"
#include <cmath>
#include "error.h"
#include "rs274ngc_return.hh"
#include "codes.h"

/***********************************************************************/

/* arc_data_comp_ijk

Returned Value: int
If any of the following errors occur, this returns the error code shown.
Otherwise, it returns RS274NGC_OK.
1. The two calculable values of the radius differ by more than
tolerance: NCE_RADIUS_TO_END_OF_ARC_DIFFERS_FROM_RADIUS_TO_START
2. move is not G_2 or G_3: NCE_BUG_CODE_NOT_G2_OR_G3

Side effects:
This finds and sets the values of center_x, center_y, and turn.

Called by: convert_arc_comp1

This finds the center coordinates and number of full or partial turns
counterclockwise of a helical or circular arc in ijk-format in the XY
plane. The center is computed easily from the current point and center
offsets, which are given. It is checked that the end point lies one
tool radius from the arc.

*/

void arc_data_comp_ijk(                 /* ARGUMENTS                               */
int move,                                     /* either G_2 (cw arc) or G_3 (ccw arc)             */
Side side,                                     /* either RIGHT or LEFT                             */
double tool_radius,                           /* radius of the tool                               */
double current_x,                             /* first coordinate of current point                */
double current_y,                             /* second coordinate of current point               */
double end_x,                                 /* first coordinate of arc end point                */
double end_y,                                 /* second coordinate of arc end point               */
double i_number,                              /* first coordinate offset of center from current   */
double j_number,                              /* second coordinate offset of center from current  */
double * center_x,                            /* pointer to first coordinate of center of arc     */
double * center_y,                            /* pointer to second coordinate of center of arc    */
int * turn,                                   /* pointer to number of full or partial circles CCW */
double tolerance)                             /* tolerance of differing radii                     */
{
    *center_x = (current_x + i_number);
    *center_y = (current_y + j_number);
    double arc_radius = hypot(i_number, j_number);
    double radius2 = hypot((*center_x - end_x), (*center_y - end_y));
    radius2 =
        (((side == Side::Left ) and (move == 30)) or
        ((side == Side::Right) and (move == 20))) ?
        (radius2 - tool_radius): (radius2 + tool_radius);
    error_if(fabs(arc_radius - radius2) > tolerance, NCE_RADIUS_TO_END_OF_ARC_DIFFERS_FROM_RADIUS_TO_START);
/* This catches an arc too small for the tool, also */
    if (move == G_2)
        *turn = -1;
    else if (move == G_3)
        *turn = 1;
    else
        throw error(NCE_BUG_CODE_NOT_G2_OR_G3);
}

/****************************************************************************/

/* arc_data_comp_r

Returned Value: int
If any of the following errors occur, this returns the error code shown.
Otherwise, it returns RS274NGC_OK.
1. The arc radius is too small to reach the end point:
NCE_RADIUS_TOO_SMALL_TO_REACH_END_POINT
2. The arc radius is not greater than the tool_radius, but should be:
NCE_TOOL_RADIUS_NOT_LESS_THAN_ARC_RADIUS_WITH_COMP
3. An imaginary value for offset would be found, which should never
happen if the theory is correct: NCE_BUG_IN_TOOL_RADIUS_COMP

Side effects:
This finds and sets the values of center_x, center_y, and turn.

Called by: convert_arc_comp1

This finds the center coordinates and number of full or partial turns
counterclockwise of a helical or circular arc (call it arc1) in
r-format in the XY plane.  Arc2 is constructed so that it is tangent
to a circle whose radius is tool_radius and whose center is at the
point (current_x, current_y) and passes through the point (end_x,
end_y). Arc1 has the same center as arc2. The radius of arc1 is one
tool radius larger or smaller than the radius of arc2.

If the value of the big_radius argument is negative, that means [NCMS,
page 21] that an arc larger than a semicircle is to be made.
Otherwise, an arc of a semicircle or less is made.

The algorithm implemented here is to construct a line L from the
current point to the end point, and a perpendicular to it from the
center of the arc which intersects L at point P. Since the distance
from the end point to the center and the distance from the current
point to the center are known, two equations for the length of the
perpendicular can be written. The right sides of the equations can be
set equal to one another and the resulting equation solved for the
length of the line from the current point to P. Then the location of
P, the length of the perpendicular, the angle of the perpendicular,
and the location of the center, can be found in turn.

This needs to be better documented, with figures. There are eight
possible arcs, since there are three binary possibilities: (1) tool
inside or outside arc, (2) clockwise or counterclockwise (3) two
positions for each arc (of the given radius) tangent to the tool
outline and through the end point. All eight are calculated below,
since theta, radius2, and turn may each have two values.

To see two positions for each arc, imagine the arc is a hoop, the
tool is a cylindrical pin, and the arc may rotate around the end point.
The rotation covers all possible positions of the arc. It is easy to
see the hoop is constrained by the pin at two different angles, whether
the pin is inside or outside the hoop.

*/

void arc_data_comp_r(                   /* ARGUMENTS                                 */
int move,                                     /* either G_2 (cw arc) or G_3 (ccw arc)             */
Side side,                                     /* either RIGHT or LEFT                             */
double tool_radius,                           /* radius of the tool                               */
double current_x,                             /* first coordinate of current point                */
double current_y,                             /* second coordinate of current point               */
double end_x,                                 /* first coordinate of arc end point                */
double end_y,                                 /* second coordinate of arc end point               */
double big_radius,                            /* radius of arc                                    */
double * center_x,                            /* pointer to first coordinate of center of arc     */
double * center_y,                            /* pointer to second coordinate of center of arc    */
int * turn)                                   /* pointer to number of full or partial circles CCW */
{
    double abs_radius;                        /* absolute value of big_radius          */
    double alpha;                             /* direction of line from current to end */
    double distance;                          /* length of line L from current to end  */
    double mid_length;                        /* length from current point to point P  */
    double offset;                            /* length of line from P to center       */
    double radius2;                           /* distance from center to current point */
    double mid_x;                             /* x-value of point P                    */
    double mid_y;                             /* y-value of point P                    */
    double theta;                             /* direction of line from P to center    */

    abs_radius = fabs(big_radius);
    error_if((abs_radius <= tool_radius) and (((side == Side::Left ) and (move == G_3)) or
        ((side == Side::Right) and (move == G_2))),
        NCE_TOOL_RADIUS_NOT_LESS_THAN_ARC_RADIUS_WITH_COMP);

    distance = hypot((end_x - current_x), (end_y - current_y));
    alpha = atan2 ((end_y - current_y), (end_x - current_x));
    theta = (((move == G_3) and (big_radius > 0)) or
        ((move == G_2) and (big_radius < 0))) ?
        (alpha + PI2) : (alpha - PI2);
    radius2 = (((side == Side::Left ) and (move == G_3)) or
        ((side == Side::Right) and (move == G_2))) ?
        (abs_radius - tool_radius) : (abs_radius + tool_radius);
    error_if(distance > (radius2 + abs_radius), NCE_RADIUS_TOO_SMALL_TO_REACH_END_POINT);
    mid_length = ((radius2 * radius2) + (distance * distance) - (abs_radius * abs_radius)) / (2.0 * distance);
    mid_x = current_x + (mid_length * cos(alpha));
    mid_y = current_y + (mid_length * sin(alpha));
    error_if((radius2 * radius2) <= (mid_length * mid_length), NCE_BUG_IN_TOOL_RADIUS_COMP);
    offset = sqrt((radius2 * radius2) - (mid_length * mid_length));
    
    *center_x = mid_x + (offset * cos(theta));
    *center_y = mid_y + (offset * sin(theta));
    *turn = (move == G_2) ? -1 : 1;
}

/****************************************************************************/

/* arc_data_ijk

Returned Value: int
If any of the following errors occur, this returns the error code shown.
Otherwise, it returns RS274NGC_OK.
1. The two calculable values of the radius differ by more than
tolerance: NCE_RADIUS_TO_END_OF_ARC_DIFFERS_FROM_RADIUS_TO_START
2. The move code is not G_2 or G_3: NCE_BUG_CODE_NOT_G2_OR_G3
3. Either of the two calculable values of the radius is zero:
NCE_ZERO_RADIUS_ARC

Side effects:
This finds and sets the values of center_x, center_y, and turn.

Called by:
convert_arc2
convert_arc_comp2

This finds the center coordinates and number of full or partial turns
counterclockwise of a helical or circular arc in ijk-format. This
function is used by convert_arc2 for all three planes, so "x" and
"y" really mean "first_coordinate" and "second_coordinate" wherever
they are used here as suffixes of variable names. The i and j prefixes
are handled similarly.

*/

void arc_data_ijk(                      /* ARGUMENTS                                       */
int move,                                     /* either G_2 (cw arc) or G_3 (ccw arc)            */
double current_x,                             /* first coordinate of current point               */
double current_y,                             /* second coordinate of current point              */
double end_x,                                 /* first coordinate of arc end point               */
double end_y,                                 /* second coordinate of arc end point              */
double i_number,                              /* first coordinate offset of center from current  */
double j_number,                              /* second coordinate offset of center from current */
double * center_x,                            /* pointer to first coordinate of center of arc    */
double * center_y,                            /* pointer to second coordinate of center of arc   */
int * turn,                                   /* pointer to no. of full or partial circles CCW   */
double tolerance)                             /* tolerance of differing radii                    */
{
    double radius;                            /* radius to current point */
    double radius2;                           /* radius to end point     */
    *center_x = (current_x + i_number);
    *center_y = (current_y + j_number);
    radius = hypot((*center_x - current_x), (*center_y - current_y));
    radius2 = hypot((*center_x - end_x), (*center_y - end_y));
    error_if((radius == 0.0) or (radius2 == 0.0), NCE_ZERO_RADIUS_ARC);
    error_if(fabs(radius - radius2) > tolerance, NCE_RADIUS_TO_END_OF_ARC_DIFFERS_FROM_RADIUS_TO_START);
    if (move == G_2)
        *turn = -1;
    else if (move == G_3)
        *turn = 1;
    else
        throw error(NCE_BUG_CODE_NOT_G2_OR_G3);
}

/****************************************************************************/

/* arc_data_r

Returned Value: int
If any of the following errors occur, this returns the error shown.
Otherwise, it returns RS274NGC_OK.
1. The radius is too small to reach the end point:
NCE_ARC_RADIUS_TOO_SMALL_TO_REACH_END_POINT
2. The current point is the same as the end point of the arc
(so that it is not possible to locate the center of the circle):
NCE_CURRENT_POINT_SAME_AS_END_POINT_OF_ARC

Side effects:
This finds and sets the values of center_x, center_y, and turn.

Called by:
convert_arc2
convert_arc_comp2

This finds the center coordinates and number of full or partial turns
counterclockwise of a helical or circular arc in the r format. This
function is used by convert_arc2 for all three planes, so "x" and
"y" really mean "first_coordinate" and "second_coordinate" wherever
they are used here as suffixes of variable names.

If the value of the radius argument is negative, that means [NCMS,
page 21] that an arc larger than a semicircle is to be made.
Otherwise, an arc of a semicircle or less is made.

The algorithm used here is based on finding the midpoint M of the line
L between the current point and the end point of the arc. The center
of the arc lies on a line through M perpendicular to L.

*/

void arc_data_r(                        /* ARGUMENTS                                     */
int move,                                     /* either G_2 (cw arc) or G_3 (ccw arc)          */
double current_x,                             /* first coordinate of current point             */
double current_y,                             /* second coordinate of current point            */
double end_x,                                 /* first coordinate of arc end point             */
double end_y,                                 /* second coordinate of arc end point            */
double radius,                                /* radius of arc                                 */
double * center_x,                            /* pointer to first coordinate of center of arc  */
double * center_y,                            /* pointer to second coordinate of center of arc */
int * turn)                                   /* pointer to no. of full or partial circles CCW */
{
    double abs_radius;                        /* absolute value of given radius */
    double half_length;                       /* distance from M to end point   */
    double mid_x;                             /* first coordinate of M          */
    double mid_y;                             /* second coordinate of M         */
    double offset;                            /* distance from M to center      */
    double theta;                             /* angle of line from M to center */
    double turn2;                             /* absolute value of half of turn */

    error_if((end_x == current_x) and (end_y == current_y), NCE_CURRENT_POINT_SAME_AS_END_POINT_OF_ARC);
    abs_radius = fabs(radius);
    mid_x = (end_x + current_x)/2.0;
    mid_y = (end_y + current_y)/2.0;
    half_length = hypot((mid_x - end_x), (mid_y - end_y));
    error_if((half_length/abs_radius) > (1+TINY), NCE_ARC_RADIUS_TOO_SMALL_TO_REACH_END_POINT);
    if ((half_length/abs_radius) > (1-TINY))
        half_length = abs_radius;        /* allow a small error for semicircle */
/* check needed before calling asin   */
    if (((move == G_2) and (radius > 0)) or
        ((move == G_3) and (radius < 0)))
        theta = atan2((end_y - current_y), (end_x - current_x)) - PI2;
    else
        theta = atan2((end_y - current_y), (end_x - current_x)) + PI2;

    turn2 = asin (half_length/abs_radius);
    offset = abs_radius * cos(turn2);
    *center_x = mid_x + (offset * cos(theta));
    *center_y = mid_y + (offset * sin(theta));
    *turn = (move == G_2) ? -1 : 1;
}

/****************************************************************************/

/* find_arc_length

Returned Value: double (length of path between start and end points)

Side effects: none

Called by:
inverse_time_rate_arc
inverse_time_rate_arc2
inverse_time_rate_as

This calculates the length of the path that will be made relative to
the XYZ axes for a motion in which the X,Y,Z, motion is a circular or
helical arc with its axis parallel to the Z-axis. If tool length
compensation is on, this is the path of the tool tip; if off, the
length of the path of the spindle tip. Any rotary axis motion is
ignored.

If the arc is helical, it is coincident with the hypotenuse of a right
triangle wrapped around a cylinder. If the triangle is unwrapped, its
base is [the radius of the cylinder times the number of radians in the
helix] and its height is [z2 - z1], and the path length can be found
by the Pythagorean theorem.

This is written as though it is only for arcs whose axis is parallel to
the Z-axis, but it will serve also for arcs whose axis is parallel
to the X-axis or Y-axis, with suitable permutation of the arguments.

This works correctly when turn is zero (find_turn returns 0 in that
case).

*/

double find_arc_length(                /* ARGUMENTS                          */
double x1,                                    /* X-coordinate of start point        */
double y1,                                    /* Y-coordinate of start point        */
double z1,                                    /* Z-coordinate of start point        */
double center_x,                              /* X-coordinate of arc center         */
double center_y,                              /* Y-coordinate of arc center         */
int turn,                                     /* no. of full or partial circles CCW */
double x2,                                    /* X-coordinate of end point          */
double y2,                                    /* Y-coordinate of end point          */
double z2)                                    /* Z-coordinate of end point          */
{
   double radius = hypot((center_x - x1), (center_y - y1));
    
    /* amount of turn of arc in radians */
   double theta = find_turn(x1, y1, center_x, center_y, turn, x2, y2);
    if (z2 == z1)
        return (radius * fabs(theta));
    else
        return hypot((radius * theta), (z2 - z1));
}

/****************************************************************************/

/* find_straight_length

Returned Value: double (length of path between start and end points)

Side effects: none

Called by:
inverse_time_rate_straight
inverse_time_rate_as

This calculates a number to use in feed rate calculations when inverse
time feed mode is used, for a motion in which X,Y,Z,A,B, and C each change
linearly or not at all from their initial value to their end value.

This is used when the feed_reference mode is CANON_XYZ, which is
always in rs274NGC.

If any of the X, Y, or Z axes move or the A-axis, B-axis, and C-axis
do not move, this is the length of the path relative to the XYZ axes
from the first point to the second, and any rotary axis motion is
ignored. The length is the simple Euclidean distance.

The formula for the Euclidean distance "length" of a move involving
only the A, B and C axes is based on a conversation with Jim Frohardt at
Boeing, who says that the Fanuc controller on their 5-axis machine
interprets the feed rate this way. Note that if only one rotary axis
moves, this formula returns the absolute value of that axis move,
which is what is desired.

*/

double find_straight_length(const Position& end, const Position& start)
{
    /* straight line */
    if ((start.x != end.x) or (start.y != end.y) or (start.z != end.z) or ((end.a == start.a) and (end.b == start.b) and (end.c == start.c)))
        return sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2) + pow(end.z - start.z, 2));
    else
        return sqrt(pow(end.a - start.a, 2) + pow(end.b - start.b, 2) + pow(end.c - start.c, 2));
}

/****************************************************************************/

/* find_turn

Returned Value: double (angle in radians between two radii of a circle)

Side effects: none

Called by: find_arc_length

All angles are in radians.

*/

double find_turn(                      /* ARGUMENTS                          */
double x1,                                    /* X-coordinate of start point        */
double y1,                                    /* Y-coordinate of start point        */
double center_x,                              /* X-coordinate of arc center         */
double center_y,                              /* Y-coordinate of arc center         */
int turn,                                     /* no. of full or partial circles CCW */
double x2,                                    /* X-coordinate of end point          */
double y2)                                    /* Y-coordinate of end point          */
{
    double alpha;                             /* angle of first radius                      */
    double beta;                              /* angle of second radius                     */
    double theta;                             /* amount of turn of arc CCW - negative if CW */

    if (turn == 0)
        return 0.0;
    alpha = atan2((y1 - center_y), (x1 - center_x));
    beta = atan2((y2 - center_y), (x2 - center_x));
    if (turn > 0)
    {
        if (beta <= alpha)
            beta = (beta + TWO_PI);
        theta = ((beta - alpha) + ((turn - 1) * TWO_PI));
    }
    else                                      /* turn < 0 */
    {
        if (alpha <= beta)
            alpha = (alpha + TWO_PI);
        theta = ((beta - alpha) + ((turn + 1) * TWO_PI));
    }
    return theta;
}

