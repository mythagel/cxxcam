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
 * primitives.h
 *
 *  Created on: 06/05/2013
 *      Author: nicholas
 */

#ifndef NEF_PRIMITIVES_H_
#define NEF_PRIMITIVES_H_
#include "polyhedron.h"

namespace nef
{

/*
 * x, y, z - Center
 * r - Radius
 * slices - number of segments
 */
polyhedron_t make_sphere(double x, double y, double z, double r, std::size_t slices);

/*
 * x1, y1, z1 - First corner
 * x2, y2, z2 - Opposite corner
 */
polyhedron_t make_box(double x1, double y1, double z1, double x2, double y2, double z2);

/*
 * x1, y1, z1 - Top center
 * x2, y2, z2 - Bottom center
 * top_radius - Radius at top
 * bottom_radius - Radius at bottom
 * slices - number of segments
 */
polyhedron_t make_cone(double x1, double y1, double z1, double x2, double y2, double z2, double top_radius, double bottom_radius, std::size_t slices);

}

#endif /* NEF_PRIMITIVES_H_ */
