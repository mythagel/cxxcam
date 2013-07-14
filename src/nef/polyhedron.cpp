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
 * polyhedron.cpp
 *
 *  Created on: 04/02/2013
 *      Author: nicholas
 */

#include "polyhedron.h"
#include "private.h"
#include <istream>
#include <ostream>
#include "cgal.h"
#include <CGAL/Aff_transformation_3.h>
#include <algorithm>

typedef CGAL::Aff_transformation_3<Exact_Kernel> Aff_transformation_3;

namespace nef
{

polyhedron_t::polyhedron_t(const std::shared_ptr<private_t>& priv)
 : priv(priv)
{
}

void polyhedron_t::ensure_unique()
{
	if(!priv.unique())
		priv = std::make_shared<private_t>(*priv);
}

polyhedron_t::polyhedron_t()
 : priv(std::make_shared<private_t>())
{
}

polyhedron_t polyhedron_t::operator*(const polyhedron_t& poly) const
{
	return { std::make_shared<private_t>(priv->nef * poly.priv->nef) };
}
polyhedron_t polyhedron_t::operator+(const polyhedron_t& poly) const
{
	return { std::make_shared<private_t>(priv->nef + poly.priv->nef) };
}
polyhedron_t polyhedron_t::operator-(const polyhedron_t& poly) const
{
	return { std::make_shared<private_t>(priv->nef - poly.priv->nef) };
}
polyhedron_t polyhedron_t::operator^(const polyhedron_t& poly) const
{
	return { std::make_shared<private_t>(priv->nef ^ poly.priv->nef) };
}
polyhedron_t polyhedron_t::operator!() const
{
	return { std::make_shared<private_t>(!priv->nef) };
}
polyhedron_t& polyhedron_t::operator*=(const polyhedron_t& poly)
{
	ensure_unique();
	priv->nef *= poly.priv->nef;
	priv->regularise();
	return *this;
}
polyhedron_t& polyhedron_t::operator+=(const polyhedron_t& poly)
{
	ensure_unique();
	priv->nef += poly.priv->nef;
	priv->regularise();
	return *this;
}
polyhedron_t& polyhedron_t::operator-=(const polyhedron_t& poly)
{
	ensure_unique();
	priv->nef -= poly.priv->nef;
	priv->regularise();
	return *this;
}
polyhedron_t& polyhedron_t::operator^=(const polyhedron_t& poly)
{
	ensure_unique();
	priv->nef ^= poly.priv->nef;
	priv->regularise();
	return *this;
}

bool polyhedron_t::operator==(const polyhedron_t& poly) const
{
	return priv->nef == poly.priv->nef;
}
bool polyhedron_t::operator!=(const polyhedron_t& poly) const
{
	return priv->nef != poly.priv->nef;
}
bool polyhedron_t::operator<(const polyhedron_t& poly) const
{
	return priv->nef < poly.priv->nef;
}
bool polyhedron_t::operator>(const polyhedron_t& poly) const
{
	return priv->nef > poly.priv->nef;
}
bool polyhedron_t::operator<=(const polyhedron_t& poly) const
{
	return priv->nef <= poly.priv->nef;
}
bool polyhedron_t::operator>=(const polyhedron_t& poly) const
{
	return priv->nef >= poly.priv->nef;
}

polyhedron_t polyhedron_t::rotate(const polyhedron_t& polyhedron, double qw, double qx, double qy, double qz)
{
	Aff_transformation_3 rotation(
		1.0 - 2.0*qy*qy - 2.0*qz*qz, 2.0*qx*qy - 2.0*qz*qw, 2.0*qx*qz + 2.0*qy*qw, 0.0,
		2.0*qx*qy + 2.0*qz*qw, 1.0 - 2.0*qx*qx - 2.0*qz*qz, 2.0*qy*qz - 2.0*qx*qw, 0.0,
		2.0*qx*qz - 2.0*qy*qw, 2.0*qy*qz + 2.0*qx*qw, 1.0 - 2.0*qx*qx - 2.0*qy*qy, 0.0,
		0.0, 0.0, 0.0, 1.0);

	std::transform(P.points_begin(), P.points_end(), P.points_begin(), rotation);

	return {};
}
polyhedron_t polyhedron_t::translate(const polyhedron_t& polyhedron, double x, double y, double z)
{
	return {};
}

polyhedron_t::~polyhedron_t()
{
}

std::ostream& operator<<(std::ostream& os, const polyhedron_t& poly)
{
	return os << poly.priv->nef;
}
std::istream& operator>>(std::istream& is, polyhedron_t& poly)
{
	poly.ensure_unique();
	return is >> poly.priv->nef;
}

}

