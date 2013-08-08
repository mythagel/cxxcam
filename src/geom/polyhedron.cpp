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
#include <CGAL/IO/Nef_polyhedron_iostream_3.h>
#include <CGAL/OFF_to_nef_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>

namespace geom
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
//	if(!nef.is_valid(false, 3))
//		throw std::runtime_error("polyhedron_t: polyhedron is not valid.");
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

Polyhedron_3 to_Polyhedron_3(const polyhedron_t& polyhedron)
{
	auto priv = get_priv(polyhedron);
	if(!priv->nef.is_simple())
		throw std::runtime_error("to_poly: polyhedron is not 2-manifold.");
	
	Polyhedron_3 P;
	priv->nef.convert_to_polyhedron(P);
	return P;
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

bool polyhedron_t::empty() const
{
	return priv->nef.is_empty();
}

polyhedron_t::~polyhedron_t()
{
}

namespace format
{

enum Type
{
	OFF,
	NEF
};

int itag()
{
    static const int t = std::ios_base::xalloc();
    return t;
}

std::ios_base& off(std::ios_base& ios)
{
	ios.iword(itag()) = OFF;
	return ios;
}
std::ios_base& nef(std::ios_base& ios)
{
	ios.iword(itag()) = NEF;
	return ios;
}

}

std::ostream& operator<<(std::ostream& os, const polyhedron_t& poly)
{
	switch(os.iword(format::itag()))
	{
		case format::OFF:
			os << to_Polyhedron_3(poly);
			break;
		case format::NEF:
			os << poly.priv->nef;
			break;
		default:
			throw std::logic_error("polyhedron_t: Unknown data format.");
	}
	return os;
}
std::istream& operator>>(std::istream& is, polyhedron_t& poly)
{
	poly.ensure_unique();
	switch(is.iword(format::itag()))
	{
		case format::OFF:
			CGAL::OFF_to_nef_3(is, poly.priv->nef);
			break;
		case format::NEF:
			is >> poly.priv->nef;
			break;
		default:
			throw std::logic_error("polyhedron_t: Unknown data format.");
	}
	return is;
}

}

