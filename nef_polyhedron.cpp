/*
 * nef_polyhedron.cpp
 *
 *  Created on: 04/02/2013
 *      Author: nicholas
 */

#include "nef_polyhedron.h"
#include <istream>
#include <ostream>
#include <cassert>

#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/IO/Nef_polyhedron_iostream_3.h>

#if 1
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
typedef CGAL::Exact_predicates_exact_constructions_kernel Exact_Kernel;
#else
#include <CGAL/Gmpz.h>
#include <CGAL/Extended_homogeneous.h>
typedef CGAL::Extended_homogeneous<CGAL::Gmpz> Exact_Kernel;
#endif

// glide
#include <CGAL/minkowski_sum_3.h>

// Meshing (for volume)
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/Polyhedral_mesh_domain_3.h>
#include <CGAL/Polyhedral_mesh_domain_with_features_3.h>
#include <CGAL/make_mesh_3.h>
#include <CGAL/refine_mesh_3.h>

typedef CGAL::Nef_polyhedron_3<Exact_Kernel> Nef_polyhedron_3;
typedef Exact_Kernel::Point_3 Point_3;

// Meshing
typedef CGAL::Mesh_polyhedron_3<Exact_Kernel>::type Exact_Polyhedron;
typedef CGAL::Exact_predicates_inexact_constructions_kernel Inexact_Kernel;
typedef CGAL::Mesh_polyhedron_3<Inexact_Kernel>::type Inexact_Polyhedron;

typedef CGAL::Polyhedral_mesh_domain_with_features_3<Inexact_Kernel, Inexact_Polyhedron> Mesh_domain;
typedef CGAL::Mesh_triangulation_3<Mesh_domain>::type Tr;
typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;
typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;

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

nef_polyhedron_t nef_polyhedron_t::glide(const polyline_t& path) const
{
	typedef std::vector<Point_3>::const_iterator point_iterator;
	typedef std::pair<point_iterator, point_iterator>  point_range;
	typedef std::vector<point_range> polyline;

	std::vector<Point_3> point_line;
	for(auto& point : path.line)
		point_line.emplace_back(point.x, point.y, point.z);

	polyline poly = { point_range(point_line.begin(), point_line.end()) };
	Nef_polyhedron_3 N1(poly.begin(), poly.end(), Nef_polyhedron_3::Polylines_tag());

	auto glided_priv = std::make_shared<private_t>(CGAL::minkowski_sum_3(priv->nef, N1));

	if (glided_priv->nef.is_simple())
	{
		return { glided_priv };
	}
	else
	{
		// TODO result is not a 2-manifold. what now?
		return {};
	}
}

double nef_polyhedron_t::volume() const
{
	Inexact_Polyhedron PK;
	/*
	 * TODO convert nef_polyhedron to exact polyhedron and then convert
	 * exact polyhedron to inexact for meshing.
	 */
//	priv->nef.convert_to_polyhedron(PK);

	assert(PK.is_valid());

	Mesh_domain domain(PK);
	domain.detect_features();

	Mesh_criteria criteria(CGAL::parameters::facet_angle = 25, CGAL::parameters::facet_size = 0.15,
			CGAL::parameters::facet_distance = 0.008, CGAL::parameters::cell_radius_edge_ratio = 3);

	C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria, CGAL::parameters::no_perturb(), CGAL::parameters::no_exude());

	Tr triangulation = c3t3.triangulation();
	double volume(0);
	for(Tr::Finite_cells_iterator it = triangulation.finite_cells_begin(); it != triangulation.finite_cells_end(); ++it)
	{
		typename Tr::Tetrahedron tetr = triangulation.tetrahedron(it);
		volume += tetr.volume();
	}

	return volume;
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
