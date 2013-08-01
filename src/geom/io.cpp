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
 * io.cpp
 *
 *  Created on: 23/05/2013
 *      Author: nicholas
 */
#include "io.h"
#include "polyhedron.h"
#include "private.h"
#include <CGAL/iterator.h>
#include <cassert>

namespace geom
{

object_t to_object(const polyhedron_t& poly)
{
	typedef Polyhedron_3::Halfedge_around_facet_circulator Halfedge_around_facet_circulator;
	typedef typename Polyhedron_3::Vertex_const_iterator Vertex_const_iterator;
	
	auto P = to_Polyhedron_3(poly);
	object_t o;
	
	o.vertices.reserve(P.size_of_vertices());
	for(auto vi = P.vertices_begin(); vi != P.vertices_end(); ++vi)
		o.vertices.push_back({to_double(vi->point().x()), to_double(vi->point().y()), to_double(vi->point().z())});
	
	typedef CGAL::Inverse_index<Vertex_const_iterator> Index;
	Index index(P.vertices_begin(), P.vertices_end());
	
	o.faces.reserve(P.size_of_facets());
	for(auto fi = P.facets_begin(); fi != P.facets_end(); ++fi)
	{
		auto hc = fi->facet_begin();
		auto hc_end = hc;
		
		auto n = circulator_size(hc);
		assert(n >= 3);
		
	    o.faces.emplace_back();
	    object_t::face& f = o.faces.back();
	    f.reserve(n);
		do
		{
			f.push_back(index[Vertex_const_iterator(hc->vertex())]);
			++hc;
		} while(hc != hc_end);
	}
	return o;
}

}

