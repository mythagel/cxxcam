// Copyright (C) 2012 Benjamin Kehlet
//
// This file is part of DOLFIN.
//
// DOLFIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DOLFIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.
//
// Significantly Modified by Nicholas Gill, 2013 (Originally GeometryToCGALConverter.cpp)
//
// First added:  2012-05-10
// Last changed: 2012-05-10

#include <limits>
#include <cassert>
#include <initializer_list>
#include "CSGPrimitives3D.h"
#include "Point.h"
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Modifier_base.h>


namespace dolfin
{
namespace detail
{

static const double PI = 3.14159265358979323846;

// TODO Correct narrowing conversion warnings this code causes.
// Note that DOLFIN code below relies on integer ops for correctness.
template<typename HalfedgeDS>
inline void add_facet(CGAL::Polyhedron_incremental_builder_3<HalfedgeDS>& builder, std::initializer_list<std::size_t> vertices)
{
	builder.begin_facet();
	for (auto vertex : vertices)
		builder.add_vertex_to_facet(vertex);
	builder.end_facet();
}

template<typename HalfedgeDS, typename Point_3>
inline void add_vertex(CGAL::Polyhedron_incremental_builder_3<HalfedgeDS>& builder, const Point_3& point)
{
	builder.add_vertex(point);
}

template<typename Polyhedron_3>
class Build_sphere : public CGAL::Modifier_base<typename Polyhedron_3::HalfedgeDS>
{
public:
	Build_sphere(const Sphere& sphere)
	 : sphere(sphere)
	{
	}

	void operator()(typename Polyhedron_3::HalfedgeDS& hds)
	{
		const std::size_t num_slices = sphere.slices;
		const std::size_t num_sectors = sphere.slices * 2 + 1;

		const Point top = sphere.c + Point(sphere.r, 0, 0);
		const Point bottom = sphere.c - Point(sphere.r, 0, 0);
		const Point axis = Point(1, 0, 0);

		const int num_vertices = num_slices * num_sectors + 2;
		const int num_facets = num_sectors * 2 * num_slices;

		CGAL::Polyhedron_incremental_builder_3<typename Polyhedron_3::HalfedgeDS> builder(hds, true);
		builder.begin_surface(num_vertices, num_facets);

		const Point slice_rotation_axis(0, 1, 0);
		for (std::size_t i = 0; i != num_slices; ++i)
		{
			const Point sliced = axis.rotate(slice_rotation_axis, (i + 1) * PI / (num_slices + 1));
			for (std::size_t j = 0; j < num_sectors; j++)
			{
				const Point direction = sliced.rotate(axis, j * 2.0 * PI / num_sectors);
				add_vertex(builder, sphere.c + direction * sphere.r);
			}
		}

		// Add bottom has index num_vertices-1, top has index num_vertices-2
		add_vertex(builder, top);
		add_vertex(builder, bottom);

		// Add the side facets
		for (std::size_t i = 0; i != num_slices - 1; ++i)
		{
			for (std::size_t j = 0; j != num_sectors; ++j)
			{
				const std::size_t offset1 = i * num_sectors;
				const std::size_t offset2 = (i + 1) * num_sectors;

				add_facet(builder, { offset1 + j, offset1 + (j + 1) % num_sectors, offset2 + j });
				add_facet(builder, { offset2 + (j + 1) % num_sectors, offset2 + j, offset1 + (j + 1) % num_sectors });
			}
		}

		// Add the top and bottom facets
		const std::size_t top_offset = num_sectors * (num_slices - 1);
		for (std::size_t i = 0; i != num_sectors; ++i)
		{
			// Bottom facet
			add_facet(builder, { num_vertices - 2, (i + 1) % num_sectors, i });
			// Top facet
			add_facet(builder, { num_vertices - 1, top_offset + (i % num_sectors), top_offset + (i + 1) % num_sectors });
		}
		builder.end_surface();
	}

private:
	const Sphere& sphere;
};

template<typename Polyhedron_3>
class Build_box : public CGAL::Modifier_base<typename Polyhedron_3::HalfedgeDS>
{
public:
	typedef typename Polyhedron_3::Point_3 Point_3;

	Build_box(const Box& box)
	 : box(box)
	{
	}

	void operator()(typename Polyhedron_3::HalfedgeDS& hds)
	{
		CGAL::Polyhedron_incremental_builder_3<typename Polyhedron_3::HalfedgeDS> builder(hds, true);
		builder.begin_surface(8, 12);

		const double x0 = std::min(box._x0, box._y0);
		const double y0 = std::max(box._x0, box._y0);

		const double x1 = std::min(box._x1, box._y1);
		const double y1 = std::max(box._x1, box._y1);

		const double x2 = std::min(box._x2, box._y2);
		const double y2 = std::max(box._x2, box._y2);

		add_vertex(builder, Point_3(y0, x1, x2));
		add_vertex(builder, Point_3(x0, x1, y2));
		add_vertex(builder, Point_3(x0, x1, x2));
		add_vertex(builder, Point_3(x0, y1, x2));
		add_vertex(builder, Point_3(y0, x1, y2));
		add_vertex(builder, Point_3(x0, y1, y2));
		add_vertex(builder, Point_3(y0, y1, x2));
		add_vertex(builder, Point_3(y0, y1, y2));

		add_facet(builder, { 1, 2, 3 });
		add_facet(builder, { 1, 3, 5 });
		add_facet(builder, { 1, 5, 4 });
		add_facet(builder, { 4, 5, 7 });
		add_facet(builder, { 4, 7, 0 });
		add_facet(builder, { 0, 7, 6 });
		add_facet(builder, { 0, 6, 2 });
		add_facet(builder, { 2, 6, 3 });
		add_facet(builder, { 7, 5, 6 });
		add_facet(builder, { 6, 5, 3 });
		add_facet(builder, { 1, 4, 2 });
		add_facet(builder, { 2, 4, 0 });

		builder.end_surface();
	}

	const Box& box;
};

template<typename Polyhedron_3>
class Build_cone : public CGAL::Modifier_base<typename Polyhedron_3::HalfedgeDS>
{
public:
	Build_cone(const Cone& cone)
	 : cone(cone)
	{
	}

	// Return some vector orthogonal to a
	static Point generate_orthogonal(const Point& a)
	{
		const Point b(0, 1, 0);
		const Point c(0, 0, 1);

		// Find a vector not parallel to a.
		const Point d = (fabs(a.dot(b)) < fabs(a.dot(c))) ? b : c;
		return a.cross(d);
	}

	void operator()(typename Polyhedron_3::HalfedgeDS& hds)
	{
		const Point axis = (cone.top - cone.bottom) / (cone.top - cone.bottom).norm();
		Point initial = generate_orthogonal(axis);

		const int num_sides = cone.slices;
		const bool top_degenerate = near(cone.top_radius, 0.0);
		const bool bottom_degenerate = near(cone.bottom_radius, 0.0);

		const int num_vertices = (top_degenerate || bottom_degenerate) ? num_sides + 2 : num_sides * 2 + 2;

		CGAL::Polyhedron_incremental_builder_3<typename Polyhedron_3::HalfedgeDS> builder(hds, true);
		builder.begin_surface(num_vertices, num_sides * 4);

		const double delta_theta = 2.0 * PI / num_sides;
		for (int i = 0; i != num_sides; ++i)
		{
			const double theta = i * delta_theta;
			const Point rotated = initial.rotate(axis, theta);

			if (!bottom_degenerate)
				add_vertex(builder, cone.bottom + rotated * cone.bottom_radius);

			if (!top_degenerate)
				add_vertex(builder, cone.top + rotated * cone.top_radius);
		}

		// The top and bottom vertices
		add_vertex(builder, cone.bottom);
		add_vertex(builder, cone.top);

		// bottom vertex has index num_vertices-2, top vertex has index num_vertices-1

		// Construct the facets on the side.
		// Vertices must be sorted counter clockwise seen from inside.
		for (int i = 0; i != num_sides; ++i)
		{
			if (top_degenerate)
			{
				add_facet(builder, { (i + 1) % num_sides, i, num_vertices - 1 });
			}
			else if (bottom_degenerate)
			{
				add_facet(builder, { i, (i + 1) % num_sides, num_vertices - 1 });
			}
			else
			{
				//Draw the sides as triangles.
				const int vertex_offset = i * 2;

				// First triangle
				add_facet(builder, { vertex_offset, vertex_offset + 1, (vertex_offset + 2) % (num_sides * 2) });

				// Second triangle
				add_facet(builder, { (vertex_offset + 3) % (num_sides * 2), (vertex_offset + 2) % (num_sides * 2), vertex_offset + 1 });
			}
		}

		// Construct the bottom facet.
		if (!bottom_degenerate)
		{
			for (int i = num_sides - 1; i >= 0; i -= 1)
			{
				if (!top_degenerate)
					add_facet(builder, { num_vertices - 2, i * 2, ((i + 1) * 2) % (num_sides * 2) });
				else
					add_facet(builder, { num_vertices - 2, i, (i + 1) % num_sides });
			}
		}

		// Construct the the top facet
		if (!top_degenerate)
		{
			for (int i = 0; i != num_sides; ++i)
			{
				if (!bottom_degenerate)
					add_facet(builder, { num_vertices - 1, ((i + 1) * 2) % (num_sides * 2) + 1, i * 2 + 1 });
				else
					add_facet(builder, { num_vertices - 2, (i + 1) % num_sides, i });
			}
		}

		builder.end_surface();
	}
private:
	const Cone& cone;
};

}

template<typename Polyhedron_3>
void make_sphere(const Sphere& s, Polyhedron_3& P)
{
	detail::Build_sphere<Polyhedron_3> builder(s);
	P.delegate(builder);
	assert(P.is_valid());
	assert(P.is_closed());
}

template<typename Polyhedron_3>
void make_box(const Box& b, Polyhedron_3& P)
{
	detail::Build_box<Polyhedron_3> builder(b);
	P.delegate(builder);
	assert(P.is_closed());
	assert(P.is_valid());
}

template<typename Polyhedron_3>
void make_tetrahedron(const Tetrahedron* b, Polyhedron_3& P)
{
	typedef typename Polyhedron_3::Point_3 Point_3;
	P.make_tetrahedron(
			Point_3(b->x0.x(), b->x0.y(), b->x0.z()),
			Point_3(b->x1.x(), b->x1.y(), b->x1.z()),
			Point_3(b->x2.x(), b->x2.y(), b->x2.z()),
			Point_3(b->x3.x(), b->x3.y(), b->x3.z()));
}

template<typename Polyhedron_3>
void make_cone(const Cone& c, Polyhedron_3& P)
{
	detail::Build_cone<Polyhedron_3> builder(c);
	P.delegate(builder);
	assert(P.is_closed());
	assert(P.is_valid());
}

}

