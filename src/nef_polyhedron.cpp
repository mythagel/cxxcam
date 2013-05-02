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
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

// glide
#include <CGAL/minkowski_sum_3.h>
#include <utility>

// Meshing (for volume)
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/Polyhedral_mesh_domain_3.h>
#include <CGAL/Polyhedral_mesh_domain_with_features_3.h>
#include <CGAL/make_mesh_3.h>
#include <CGAL/refine_mesh_3.h>

// Copy between kernels
// Would like to use this but it doesn't seem to build.
//#include <CGAL/Polyhedron_copy_3.h>
// soo....
#include <CGAL/Modifier_base.h>
#include <CGAL/Inverse_index.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel Exact_Kernel;
typedef CGAL::Nef_polyhedron_3<Exact_Kernel> Nef_polyhedron_3;
typedef CGAL::Mesh_polyhedron_3<Exact_Kernel>::type Polyhedron_3;
typedef Exact_Kernel::Point_3 Point_3;

// Meshing
typedef CGAL::Exact_predicates_inexact_constructions_kernel Inexact_Kernel;
typedef CGAL::Mesh_polyhedron_3<Inexact_Kernel>::type Mesh_polyhedron_3;

typedef CGAL::Polyhedral_mesh_domain_with_features_3<Inexact_Kernel, Mesh_polyhedron_3> Mesh_domain;
typedef CGAL::Mesh_triangulation_3<Mesh_domain>::type Tr;
typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;
typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;

// Copy
template <class Polyhedron_input, class Polyhedron_output>
struct Copy_polyhedron_to
 : public CGAL::Modifier_base<typename Polyhedron_output::HalfedgeDS>
{
	Copy_polyhedron_to(const Polyhedron_input& in_poly)
	 : in_poly(in_poly)
	{
	}

	void operator()(typename Polyhedron_output::HalfedgeDS& out_hds)
	{
		typedef typename Polyhedron_output::HalfedgeDS Output_HDS;
		typedef typename Polyhedron_input::HalfedgeDS Input_HDS;

		CGAL::Polyhedron_incremental_builder_3<Output_HDS> builder(out_hds);

		typedef typename Polyhedron_input::Vertex_const_iterator Vertex_const_iterator;
		typedef typename Polyhedron_input::Facet_const_iterator  Facet_const_iterator;
		typedef typename Polyhedron_input::Halfedge_around_facet_const_circulator HFCC;

		builder.begin_surface(in_poly.size_of_vertices(), in_poly.size_of_facets(), in_poly.size_of_halfedges());

		for(auto vi = in_poly.vertices_begin(); vi != in_poly.vertices_end(); ++vi)
		{
			typename Polyhedron_output::Point_3 p(
					::CGAL::to_double( vi->point().x()),
					::CGAL::to_double( vi->point().y()),
					::CGAL::to_double( vi->point().z()));
			builder.add_vertex(p);
		}

		typedef CGAL::Inverse_index<Vertex_const_iterator> Index;
		Index index(in_poly.vertices_begin(), in_poly.vertices_end());

		for(auto fi = in_poly.facets_begin(); fi != in_poly.facets_end(); ++fi)
		{
			HFCC hc = fi->facet_begin();
			HFCC hc_end = hc;
			builder.begin_facet();
			do
			{
				builder.add_vertex_to_facet(index[hc->vertex()]);
				++hc;
			} while(hc != hc_end);
			builder.end_facet();
		}
		builder.end_surface();
	}
private:
	const Polyhedron_input& in_poly;
};

template <class Poly_A, class Poly_B>
void copy_to(const Poly_A& poly_a, Poly_B& poly_b)
{
	Copy_polyhedron_to<Poly_A, Poly_B> modifier(poly_a);
	poly_b.delegate(modifier);
	CGAL_assertion(poly_b.is_valid());
}

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

	polyline poly = { point_range(begin(point_line), end(point_line)) };
	Nef_polyhedron_3 N1(poly.begin(), poly.end(), Nef_polyhedron_3::Polylines_tag());

	// Need to make a copy because minkowski_sum_3 can modify its arguments
	auto nef = priv->nef;
	auto glided_priv = std::make_shared<private_t>(CGAL::minkowski_sum_3(nef, N1));

	if (glided_priv->nef.is_simple())
		return { glided_priv };
	else
		// TODO result is not a 2-manifold. what now?
		return {};
}

double nef_polyhedron_t::volume() const
{
	Mesh_polyhedron_3 PK;
	{
		Polyhedron_3 EP;
		priv->nef.convert_to_polyhedron(EP);
		copy_to(EP, PK);
	}

	assert(PK.is_valid());

	Mesh_domain domain(PK);
	domain.detect_features();

	Mesh_criteria criteria(CGAL::parameters::facet_angle = 25, CGAL::parameters::facet_size = 0.15,
			CGAL::parameters::facet_distance = 0.008, CGAL::parameters::cell_radius_edge_ratio = 3);

	auto c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria, CGAL::parameters::no_perturb(), CGAL::parameters::no_exude());

	auto triangulation = c3t3.triangulation();

	double volume(0);
//	std::vector<double> volumes;
//	volumes.reserve(triangulation.number_of_finite_cells());
	for(auto it = triangulation.finite_cells_begin(); it != triangulation.finite_cells_end(); ++it)
	{
		auto tetr = triangulation.tetrahedron(it);
		volume += tetr.volume();
	}
//	std::sort(volumes.begin(), volumes.end());
//	return std::accumulate(volumes.begin(), volumes.end(), 0.0);

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
