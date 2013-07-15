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
 * Configuration.cpp
 *
 *  Created on: 2013-07-15
 *      Author: nicholas
 */

#include "translate.h"
#include "private.h"
#include <CGAL/Aff_transformation_3.h>
#include <algorithm>

typedef CGAL::Aff_transformation_3<Exact_Kernel> Aff_transformation_3;

namespace nef
{

polyhedron_t rotate(const polyhedron_t& polyhedron, double qw, double qx, double qy, double qz)
{
	Aff_transformation_3 rotation();

	//std::transform(P.points_begin(), P.points_end(), P.points_begin(), rotation);

	return {};
}
polyhedron_t translate(const polyhedron_t& polyhedron, double x, double y, double z)
{
	return {};
}

}

