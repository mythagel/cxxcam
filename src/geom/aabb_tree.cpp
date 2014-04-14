/* 
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
 * aabb_tree.cpp
 *
 *  Created on: 2014-04-14
 *      Author: nicholas
 */

#include "aabb_tree.h"
#include "private.h"
#include "cgal.h"
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>

typedef CGAL::AABB_face_graph_triangle_primitive<Polyhedron_3> AABB_polyhedron_triangle_primitive;
typedef CGAL::AABB_traits<Nef_Kernel, AABB_polyhedron_triangle_primitive> AABB_traits;
typedef CGAL::AABB_tree<AABB_traits> AABB_tree;

namespace geom
{

struct aabb_tree_t::private_t
{
	private_t(const polyhedron_t& poly);
	
    Polyhedron_3 P;
	AABB_tree aabb;
};

aabb_tree_t::private_t::private_t(const polyhedron_t& poly)
 : P(to_Polyhedron_3(poly)), aabb(P.facets_begin(), P.facets_end(), P)
{
	aabb.accelerate_distance_queries();
}

aabb_tree_t::aabb_tree_t(const polyhedron_t& poly)
 : priv(std::make_shared<private_t>(poly))
{
}

aabb_tree_t::~aabb_tree_t()
{
}

}

