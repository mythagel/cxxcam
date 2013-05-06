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
	return *this;
}
polyhedron_t& polyhedron_t::operator+=(const polyhedron_t& poly)
{
	ensure_unique();
	priv->nef += poly.priv->nef;
	return *this;
}
polyhedron_t& polyhedron_t::operator-=(const polyhedron_t& poly)
{
	ensure_unique();
	priv->nef -= poly.priv->nef;
	return *this;
}
polyhedron_t& polyhedron_t::operator^=(const polyhedron_t& poly)
{
	ensure_unique();
	priv->nef ^= poly.priv->nef;
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

