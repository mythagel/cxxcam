/*
 * copy_to.h
 *
 *  Created on: 06/05/2013
 *      Author: nicholas
 */

#ifndef NEF_COPY_TO_H_
#define NEF_COPY_TO_H_
#include "cgal.h"

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

#endif /* NEF_COPY_TO_H_ */
