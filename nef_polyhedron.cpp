/*
 * nef_polyhedron.cpp
 *
 *  Created on: 04/02/2013
 *      Author: nicholas
 */

#include "nef_polyhedron.h"
#include <istream>
#include <ostream>

//#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
//#include <CGAL/Nef_polyhedron_3.h>

#include <CGAL/Gmpz.h>
#include <CGAL/Extended_homogeneous.h>
#include <CGAL/Nef_polyhedron_3.h>

#include <CGAL/IO/Nef_polyhedron_iostream_3.h>

//typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
//typedef CGAL::Nef_polyhedron_3<Kernel> Nef_polyhedron_3;

#include <CGAL/minkowski_sum_3.h>

typedef CGAL::Extended_homogeneous<CGAL::Gmpz> Kernel;
typedef CGAL::Nef_polyhedron_3<Kernel> Nef_polyhedron_3;
typedef Kernel::Point_3 Point_3;

struct nef_polyhedron_t::private_t
{
	private_t() = default;
	private_t(const Nef_polyhedron_3& nef)
	 : nef(nef)
	{
	}
	Nef_polyhedron_3 nef;
};

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

nef_polyhedron_t nef_polyhedron_t::glide(/*polyline*/) const
{
	/*
	 * Need to convert to different kernel before glide operation
	 */
//	typedef Point_3* point_iterator;
//	typedef std::pair<point_iterator,point_iterator>  point_range;
//	typedef std::list<point_range> polyline;
//
//	Point_3 pl[] = { Point_3(-100, 0, 0), Point_3(40, -70, 0), Point_3(40, 50, 40), Point_3(-90, -60, 60), Point_3(0, 0, -100), Point_3(30, 0, 150) };
//
//	polyline poly;
//	poly.push_back(point_range(pl, pl + 6));
//
//	auto glided_priv = std::make_shared<private_t>();
//
//	Nef_polyhedron_3 N1(poly.begin(), poly.end(), Nef_polyhedron_3::Polylines_tag());
//	glided_priv->nef = CGAL::minkowski_sum_3(priv->nef, N1);
//
//	if (glided_priv->nef.is_simple())
//	{
//		return { glided_priv };
//	}
//	else
	{
		// TODO result is not a 2-manifold. what now?
		return {};
	}
}

std::ostream& operator<<(std::ostream& os, const nef_polyhedron_t& poly)
{
	return os << poly.priv->nef;
}
std::istream& operator>>(std::istream& is, nef_polyhedron_t& poly)
{
	poly.ensure_unique();
	return is >> poly.priv->nef;
}
