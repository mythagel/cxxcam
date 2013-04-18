/*
 * nef_polyhedron.cpp
 *
 *  Created on: 04/02/2013
 *      Author: nicholas
 */

#include "nef_polyhedron.h"

//#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
//#include <CGAL/Nef_polyhedron_3.h>

#include <CGAL/Gmpz.h>
#include <CGAL/Extended_homogeneous.h>
#include <CGAL/Nef_polyhedron_3.h>

#include <CGAL/IO/Nef_polyhedron_iostream_3.h>

//typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
//typedef CGAL::Nef_polyhedron_3<Kernel> Nef_polyhedron_3;

typedef CGAL::Extended_homogeneous<CGAL::Gmpz> Kernel;
typedef CGAL::Nef_polyhedron_3<Kernel> Nef_polyhedron_3;

struct nef_polyhedron_t::private_t
{
	private_t() = default;
	private_t(const Nef_polyhedron_3& nef)
	 : nef(nef)
	{
	}
	Nef_polyhedron_3 nef;
};

std::ostream& operator<<(std::ostream& os, const nef_polyhedron_t& poly)
{
	return os << poly.priv->nef;
}
std::istream& operator>>(std::istream& is, nef_polyhedron_t& poly)
{
	poly.ensure_unique();
	return is >> poly.priv->nef;
}

nef_polyhedron_t::nef_polyhedron_t(const std::shared_ptr<private_t>& priv)
 : priv(priv)
{
}

void nef_polyhedron_t::ensure_unique()
{
	if(!priv.unique())
		priv = std::make_shared<private_t>(*priv);
}

nef_polyhedron_t::nef_polyhedron_t()
 : priv(std::make_shared<private_t>())
{
}

nef_polyhedron_t& nef_polyhedron_t::operator=(const nef_polyhedron_t& poly)
{
	priv = std::make_shared<private_t>(*poly.priv);
	return *this;
}

nef_polyhedron_t nef_polyhedron_t::operator*(const nef_polyhedron_t& poly) const
{
	return { std::make_shared<private_t>(priv->nef * poly.priv->nef) };
}
nef_polyhedron_t nef_polyhedron_t::operator+(const nef_polyhedron_t& poly) const
{
	return { std::make_shared<private_t>(priv->nef + poly.priv->nef) };
}
nef_polyhedron_t nef_polyhedron_t::operator-(const nef_polyhedron_t& poly) const
{
	return { std::make_shared<private_t>(priv->nef - poly.priv->nef) };
}
nef_polyhedron_t nef_polyhedron_t::operator^(const nef_polyhedron_t& poly) const
{
	return { std::make_shared<private_t>(priv->nef ^ poly.priv->nef) };
}
nef_polyhedron_t nef_polyhedron_t::operator!() const
{
	return { std::make_shared<private_t>(!priv->nef) };
}
nef_polyhedron_t& nef_polyhedron_t::operator*=(const nef_polyhedron_t& poly)
{
	ensure_unique();
	priv->nef *= poly.priv->nef;
	return *this;
}
nef_polyhedron_t& nef_polyhedron_t::operator+=(const nef_polyhedron_t& poly)
{
	ensure_unique();
	priv->nef += poly.priv->nef;
	return *this;
}
nef_polyhedron_t& nef_polyhedron_t::operator-=(const nef_polyhedron_t& poly)
{
	ensure_unique();
	priv->nef -= poly.priv->nef;
	return *this;
}
nef_polyhedron_t& nef_polyhedron_t::operator^=(const nef_polyhedron_t& poly)
{
	ensure_unique();
	priv->nef ^= poly.priv->nef;
	return *this;
}

bool nef_polyhedron_t::operator==(const nef_polyhedron_t& poly) const
{
	return priv->nef == poly.priv->nef;
}
bool nef_polyhedron_t::operator!=(const nef_polyhedron_t& poly) const
{
	return priv->nef != poly.priv->nef;
}
bool nef_polyhedron_t::operator<(const nef_polyhedron_t& poly) const
{
	return priv->nef < poly.priv->nef;
}
bool nef_polyhedron_t::operator>(const nef_polyhedron_t& poly) const
{
	return priv->nef > poly.priv->nef;
}
bool nef_polyhedron_t::operator<=(const nef_polyhedron_t& poly) const
{
	return priv->nef <= poly.priv->nef;
}
bool nef_polyhedron_t::operator>=(const nef_polyhedron_t& poly) const
{
	return priv->nef >= poly.priv->nef;
}

nef_polyhedron_t::~nef_polyhedron_t()
{
}

