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
#include <stdexcept>

namespace nef
{

polyhedron_t::private_t::private_t()
{
}
polyhedron_t::private_t::private_t(const Nef_polyhedron_3& nef)
 : nef(nef)
{
	regularise();
}

void polyhedron_t::private_t::regularise()
{
	nef = nef.regularization();
	if(!nef.is_simple())
		throw std::runtime_error("polyhedron_t: polyhedron is not 2-manifold.");
}

polyhedron_t make_polyhedron(std::shared_ptr<polyhedron_t::private_t> priv)
{
	return { priv };
}
std::shared_ptr<polyhedron_t::private_t> get_priv(polyhedron_t& polyhedron)
{
	polyhedron.ensure_unique();
	return polyhedron.priv;
}
std::shared_ptr<const polyhedron_t::private_t> get_priv(const polyhedron_t& polyhedron)
{
	return polyhedron.priv;
}

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

