#include "geom/cgal.h"
#include <iostream>
#include <sstream>


// Meshing (for volume)
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/Polyhedral_mesh_domain_3.h>
#include <CGAL/Polyhedral_mesh_domain_with_features_3.h>
#include <CGAL/make_mesh_3.h>
#include <CGAL/refine_mesh_3.h>

// Meshing
typedef CGAL::Exact_predicates_inexact_constructions_kernel Inexact_Kernel;
typedef CGAL::Mesh_polyhedron_3<Inexact_Kernel>::type Mesh_polyhedron_3;

typedef CGAL::Polyhedral_mesh_domain_with_features_3<Inexact_Kernel, Mesh_polyhedron_3> Mesh_domain;
typedef CGAL::Mesh_triangulation_3<Mesh_domain>::type Tr;
typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;
typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;

int main()
{
    std::stringstream in(R"(OFF
8 12 0

1 0 0
0 0 0
1 1 0
0 1 0
0 1 1
0 0 1
1 0 1
1 1 1
3  1 0 6
3  5 1 6
3  0 2 7
3  6 0 7
3  3 2 1
3  1 2 0
3  5 4 1
3  1 4 3
3  5 6 7
3  4 5 7
3  4 7 3
3  3 7 2

)");

	Mesh_polyhedron_3 PK;
    in >> PK;
	assert(PK.is_valid());

	Mesh_domain domain(PK);
	domain.detect_features();

	using CGAL::parameters::facet_angle;
	using CGAL::parameters::facet_size;
	using CGAL::parameters::facet_distance;
	using CGAL::parameters::cell_radius_edge_ratio;
	// facet_size = 0.15, facet_distance = 0.008
	// TODO these parameters need to be tweaked.
	Mesh_criteria criteria(facet_angle = 25, facet_size = 1, facet_distance = 0.8, cell_radius_edge_ratio = 3);

	using CGAL::parameters::no_perturb;
	using CGAL::parameters::no_exude;
	auto c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria, no_perturb(), no_exude());

	auto triangulation = c3t3.triangulation();
	return 0;
}
