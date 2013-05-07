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
 * primitives.cpp
 *
 *  Created on: 06/05/2013
 *      Author: nicholas
 */

#include "primitives.h"
#include "private.h"
#include "cgal.h"
#include <initializer_list>

namespace nef
{

/*
 * Inspired by CGAL geometry converters in dolfin
 */
template <class Polyhedron>
class Build_block
 : public CGAL::Modifier_base<typename Polyhedron::HalfedgeDS>
{
public:
	Build_block(double x0, double y0, double z0, double x1, double y1, double z1)
	 : _x0(std::min(x0, x1)), _y0(std::min(y0, y1)), _z0(std::min(z0, z1)),
	   _x1(std::max(x0, x1)), _y1(std::max(y0, y1)), _z1(std::max(z0, z1))
	{
	}

	void operator()(typename Polyhedron::HalfedgeDS& hds)
	{
		typedef CGAL::Polyhedron_incremental_builder_3<typename Polyhedron::HalfedgeDS> Polyhedron_incremental_builder_3;
		Polyhedron_incremental_builder_3 builder(hds, true);

		auto add_vertex = [&builder](const typename Polyhedron_incremental_builder_3::Point_3& point)
		{
			builder.add_vertex(point);
		};
		auto add_facet = [&builder](std::initializer_list<int> vertices)
		{
			builder.begin_facet();
			for(auto vertex : vertices)
				builder.add_vertex_to_facet(vertex);
			builder.end_facet();
		};

		builder.begin_surface(8, 12);

		add_vertex({_x1, _y0, _z0});
		add_vertex({_x0, _y0, _z1});
		add_vertex({_x0, _y0, _z0});
		add_vertex({_x0, _y1, _z0});
		add_vertex({_x1, _y0, _z1});
		add_vertex({_x0, _y1, _z1});
		add_vertex({_x1, _y1, _z0});
		add_vertex({_x1, _y1, _z1});

		add_facet({1, 2, 3});
		add_facet({1, 3, 5});
		add_facet({1, 5, 4});
		add_facet({4, 5, 7});
		add_facet({4, 7, 0});
		add_facet({0, 7, 6});
		add_facet({0, 6, 2});
		add_facet({2, 6, 3});
		add_facet({7, 5, 6});
		add_facet({6, 5, 3});
		add_facet({1, 4, 2});
		add_facet({2, 4, 0});

		builder.end_surface();
	}
private:
	double _x0;
	double _y0;
	double _z0;

	double _x1;
	double _y1;
	double _z1;
};

polyhedron_t make_block(double x0, double y0, double z0, double x1, double y1, double z1)
{
	Polyhedron_3 poly;
	Build_block<Polyhedron_3> builder(x0, y0, z0, x1, y1, z1);
	poly.delegate(builder);
	CGAL_assertion(poly.is_closed());
	CGAL_assertion(poly.is_valid());

	auto priv = std::make_shared<polyhedron_t::private_t>( Nef_polyhedron_3(poly) );
	return { priv };
}

}

