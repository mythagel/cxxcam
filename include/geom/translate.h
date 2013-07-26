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
 * translate.h
 *
 *  Created on: 2013-07-15
 *      Author: nicholas
 */

#ifndef GEOM_TRANSLATE_H_
#define GEOM_TRANSLATE_H_

namespace geom
{

class polyhedron_t;

/*
 * Create a new polyhedron rotated by the given quaternion.
 */
polyhedron_t rotate(const polyhedron_t& polyhedron, double qw, double qx, double qy, double qz);

/*
 * Create a new polyhedron translated by the given vector.
 */
polyhedron_t translate(const polyhedron_t& polyhedron, double x, double y, double z);

/*
 * Create a new polyhedron scaled by the given factor.
 */
polyhedron_t scale(const polyhedron_t& polyhedron, double s);

}

#endif /* GEOM_TRANSLATE_H_ */
