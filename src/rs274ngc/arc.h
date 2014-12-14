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
 * arc.h
 *
 *  Created on: 2013-08-27
 *      Author: nicholas
 */

#ifndef ARC_H_
#define ARC_H_
#include "types.h"

void arc_data_comp_ijk(int move, Side side, double tool_radius, double current_x, double current_y, double end_x, double end_y, bool ij_absolute, double i_number, double j_number, int p_number, double * center_x, double * center_y, int * turn, double tolerance);

void arc_data_comp_r(int move, Side side, double tool_radius, double current_x, double current_y, double end_x, double end_y, double big_radius, int p_number, double * center_x, double * center_y, int * turn);

void arc_data_ijk(int move, double current_x, double current_y, double end_x, double end_y, bool ij_absolute, double i_number, double j_number, int p_number, double * center_x, double * center_y, int * turn, double tolerance);

void arc_data_r(int move, double current_x, double current_y, double end_x, double end_y, double radius, int p_number, double * center_x, double * center_y, int * turn, double tolerance);

double find_arc_length(double x1, double y1, double z1, double center_x, double center_y, int turn, double x2, double y2, double z2);

double find_straight_length(const Position& end, const Position& start);

double find_turn(double x1, double y1, double center_x, double center_y, int turn, double x2, double y2);

#endif /* ARC_H_ */
