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
	// bbox filter first.
	auto b0 = bbox(to_Polyhedron_3(p0));
	auto b1 = bbox(to_Polyhedron_3(p1));
	
	if(!do_overlap(b0, b1))
		return false;
	
	// TODO
	throw std::runtime_error("NI!");
}

}

