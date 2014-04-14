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
 * query.cpp
 *
 *  Created on: 2013-07-26
 *      Author: nicholas
 */

#include "query.h"
#include "polyhedron.h"
#include "private.h"
#include <CGAL/bounding_box.h>
#include <CGAL/Bbox_3.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <cmath>

typedef CGAL::AABB_face_graph_triangle_primitive<Polyhedron_3> AABB_polyhedron_triangle_primitive;
typedef CGAL::AABB_traits<Nef_Kernel, AABB_polyhedron_triangle_primitive> AABB_traits;
typedef CGAL::AABB_tree<AABB_traits> AABB_tree;

#include <stdexcept>

namespace geom
{

namespace
{
CGAL::Bbox_3 bbox(const Polyhedron_3& poly)
{
	return CGAL::bounding_box(poly.points_begin(), poly.points_end()).bbox();
}
}

bool intersects(const polyhedron_t& p0, const polyhedron_t& p1)
{
	auto poly0 = to_Polyhedron_3(p0);
	auto poly1 = to_Polyhedron_3(p1);
	
	if(!do_overlap(bbox(poly0), bbox(poly1)))
		return false;
	
	return !((p0 * p1).empty());
}

double distance(const polyhedron_t& poly, const query::point_3& p)
{
	auto P = to_Polyhedron_3(poly);
	
	// TODO constructing the AABB tree is expensive.
	// design an interface where this can be cached between calls.
	AABB_tree aabb_p0 { P.facets_begin(), P.facets_end(), P};
	aabb_p0.accelerate_distance_queries();
	
	Point_3 query(p.x, p.y, p.z);
    auto dist2 = aabb_p0.squared_distance(query);
	
	return sqrt(to_double(dist2));
}

query::bbox_3 bounding_box(const polyhedron_t& poly)
{
    auto p0 = to_Polyhedron_3(poly);
    auto b = bbox(p0);
    return {{b.xmin(), b.ymin(), b.zmin()}, {b.xmax(), b.ymax(), b.zmax()}};
}

}

