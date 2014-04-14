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
 * aabb_tree.h
 *
 *  Created on: 2014-04-14
 *      Author: nicholas
 */

#ifndef GEOM_AABB_TREE_H_
#define GEOM_AABB_TREE_H_
#include <memory>
#include "polyhedron.h"

namespace geom
{

namespace aabb
{
struct point
{
	double x;
	double y;
	double z;
};
struct ray
{
	point p;
	point q;
};
struct segment
{
	point p;
	point q;
};
struct triangle
{
	point p;
	point q;
	point r;
};
struct plane_3
{
	point p;
	point q;
	point r;
};
}


class aabb_tree_t
{
public:
	struct private_t;
private:
	std::shared_ptr<private_t> priv;
	aabb_tree_t(const std::shared_ptr<private_t>& priv);
public:
	aabb_tree_t(const polyhedron_t& poly);
	aabb_tree_t(const aabb_tree_t&) = default;
	aabb_tree_t(aabb_tree_t&&) = default;
	aabb_tree_t& operator=(const aabb_tree_t&) = default;
	aabb_tree_t& operator=(aabb_tree_t&&) = default;

/*
    OutputIterator 	all_intersected_primitives (const Query &query, OutputIterator out) const
    boost::optional< Primitive_id > 	any_intersected_primitive (const Query &query) const
    OutputIterator 	all_intersections (const Query &query, OutputIterator out) const
    boost::optional< typename Intersection_and_primitive_id< Query >::Type > any_intersection (const Query &query) const

    size_type 	number_of_intersected_primitives (const Query &query) const
    bool 	do_intersect (const Query &query) const
    FT 	squared_distance (const Point &query) const
    Point 	closest_point (const Point &query) const
    Point_and_primitive_id 	closest_point_and_primitive (const Point &query) const
*/
    
	~aabb_tree_t();
};

}

#endif /* AABB_TREE_H_ */
