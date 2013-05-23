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
 * io.cpp
 *
 *  Created on: 23/05/2013
 *      Author: nicholas
 */
#include "io.h"
#include "private.h"
#include <CGAL/IO/Polyhedron_iostream.h>
#include <ostream>

namespace nef
{

void write_off(std::ostream& os, const polyhedron_t& poly)
{
	if(poly.priv->nef.is_simple())
	{
		Polyhedron_3 P;
		poly.priv->nef.convert_to_polyhedron(P);
		os << P;
	}
	else
	{
		throw std::runtime_error("polyhedron is not 2-manifold.");
	}
}

}

