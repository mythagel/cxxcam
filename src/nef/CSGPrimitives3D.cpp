// Copyright (C) 2012 Anders Logg
//
// This file is part of DOLFIN.
//
// DOLFIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DOLFIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.
//
// Modified by Benjamin Kehlet, 2012
// Significantly Modified by Nicholas Gill, 2013
//
// First added:  2012-04-12
// Last changed: 2012-11-12

#include <sstream>
#include "CSGPrimitives3D.h"
#include <stdexcept>

namespace dolfin
{

//-----------------------------------------------------------------------------
// Sphere
//-----------------------------------------------------------------------------
Sphere::Sphere(Point c, double r, std::size_t slices)
 : c(c), r(r), slices(slices)
{
	if (r < std::numeric_limits<double>::epsilon())
		throw std::runtime_error("Sphere has zero or negative radius");

	if (slices < 1)
		throw std::runtime_error("Can't create sphere with zero slices");
}
//-----------------------------------------------------------------------------
// Box
//-----------------------------------------------------------------------------
Box::Box(double x0, double x1, double x2, double y0, double y1, double y2)
 : _x0(x0), _x1(x1), _x2(x2), _y0(y0), _y1(y1), _y2(y2)
{
	// FIXME: Check validity of coordinates here
	if (near(x0, y0) || near(x1, y2) || near(x2, y2))
		throw std::runtime_error("Box degenerated");
}
//-----------------------------------------------------------------------------
// Cone
//-----------------------------------------------------------------------------
Cone::Cone(Point top, Point bottom, double top_radius, double bottom_radius, std::size_t slices)
 : top(top), bottom(bottom), top_radius(top_radius), bottom_radius(bottom_radius), slices(slices)
{
	if (near(top_radius, 0.0) && near(bottom_radius, 0.0))
		throw std::runtime_error("Cone with zero thickness");

	if (top.distance(bottom) < std::numeric_limits<double>::epsilon())
		throw std::runtime_error("Cone with zero length");

}
//-----------------------------------------------------------------------------
Tetrahedron::Tetrahedron(Point x0, Point x1, Point x2, Point x3)
 : x0(x0), x1(x1), x2(x2), x3(x3)
{
}
//-----------------------------------------------------------------------------

}
