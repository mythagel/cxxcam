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
#include "private.h"
#include "cgal.h"
#include "Geometry.h"

namespace nef
{

polyhedron_t make_sphere(double x, double y, double z, double r, std::size_t slices)
{
	using namespace dolfin;
	
	Polyhedron_3 P;
	make_sphere(Sphere(Point(x, y, z), r, slices), P);

	auto priv = std::make_shared<polyhedron_t::private_t>( P );
	return { priv };
}

polyhedron_t make_box(double x1, double y1, double z1, double x2, double y2, double z2)
{
	using namespace dolfin;
	
	Polyhedron_3 P;
	make_box(Box(x1, y1, z1, x2, y2, z2), P);
	
	auto priv = std::make_shared<polyhedron_t::private_t>( P );
	return { priv };
}

polyhedron_t make_cone(double x1, double y1, double z1, double x2, double y2, double z2, double top_radius, double bottom_radius, std::size_t slices)
{
	using namespace dolfin;
	
	Polyhedron_3 P;
	make_cone(Cone(Point(x1, y1, z1), Point(x2, y2, z2), top_radius, bottom_radius, slices), P);
	
	auto priv = std::make_shared<polyhedron_t::private_t>( P );
	return { priv };
}

}

