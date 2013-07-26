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

namespace geom
{

namespace
{
CGAL::Bbox_3 bbox(const Polyhedron_3& poly)
{
	return CGAL::bounding_box(poly.points_begin(), poly.points_end()).bbox();
}

Polyhedron_3 to_poly(const polyheron_t poly)
{
	auto priv = get_priv(poly);
	if(priv->nef.is_simple())
	{
		Polyhedron_3 P;
		priv->nef.convert_to_polyhedron(P);
		return P;
	}
	else
	{
		throw std::runtime_error("polyhedron is not 2-manifold.");
	}
}

}

bool intersects(const polyhedron_t& p0, const polyhedron_t& p1)
{
	
	auto b0 = bbox()
}

}

