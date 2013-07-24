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
 * primitives.cpp
 *
 *  Created on: 06/05/2013
 *      Author: nicholas
 */

#include "primitives.h"
#include "polyhedron.h"
#include "private.h"
#include "cgal.h"
#include "Geometry.h"
#include <cassert>

namespace geom
{

polyhedron_t make_sphere(const primitives::point_3& center, double r, std::size_t slices)
{
	using namespace dolfin;
	
	Polyhedron_3 P;
	make_sphere(Sphere(Point(center.x, center.y, center.z), r, slices), P);
	assert(P.is_valid());
	assert(P.is_closed());

	auto priv = std::make_shared<polyhedron_t::private_t>( P );
	return make_polyhedron( std::move(priv) );
}

polyhedron_t make_box(const primitives::point_3& p1, const primitives::point_3& p2)
{
	using namespace dolfin;
	
	Polyhedron_3 P;
	make_box(Box(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z), P);
	assert(P.is_valid());
	assert(P.is_closed());

	auto priv = std::make_shared<polyhedron_t::private_t>( P );
	return make_polyhedron( std::move(priv) );
}

polyhedron_t make_cone(const primitives::point_3& top_center, const primitives::point_3& bottom_center, double top_radius, double bottom_radius, std::size_t slices)
{
	using namespace dolfin;
	
	Polyhedron_3 P;
	make_cone(Cone(Point(top_center.x, top_center.y, top_center.z), Point(bottom_center.x, bottom_center.y, bottom_center.z), top_radius, bottom_radius, slices), P);
	assert(P.is_valid());
	assert(P.is_closed());

	auto priv = std::make_shared<polyhedron_t::private_t>( P );
	return make_polyhedron( std::move(priv) );
}

}

