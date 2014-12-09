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
 * query.h
 *
 *  Created on: 2013-07-26
 *      Author: nicholas
 */

#ifndef QUERY_H_
#define QUERY_H_

namespace geom
{

class polyhedron_t;

namespace query
{
struct point_3
{
	double x;
	double y;
	double z;
};
struct bbox_3
{
    point_3 min;
    point_3 max;
};
}

/* implement query operations for polyhedron types.
 */

bool intersects(const polyhedron_t& p0, const polyhedron_t& p1);

double distance(const polyhedron_t& poly, const query::point_3& p);

query::bbox_3 bounding_box(const polyhedron_t& poly);

}

#endif /* QUERY_H_ */
