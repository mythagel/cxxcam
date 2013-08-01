#include "geom/polyhedron.h"
#include "geom/io.h"
#include "geom/primitives.h"
#include "geom/translate.h"
#include <iostream>
#include <fstream>
#include <limits>
#include <cmath>

using namespace geom;

/*
w	x	y	z	Description
1	0	0	0	Identity quaternion, no rotation
0	1	0	0	180° turn around X axis
0	0	1	0	180° turn around Y axis
0	0	0	1	180° turn around Z axis
sqrt(0.5)	sqrt(0.5)	0	0	90° rotation around X axis
sqrt(0.5)	0	sqrt(0.5)	0	90° rotation around Y axis
sqrt(0.5)	0	0	sqrt(0.5)	90° rotation around Z axis
sqrt(0.5)	-sqrt(0.5)	0	0	-90° rotation around X axis
sqrt(0.5)	0	-sqrt(0.5)	0	-90° rotation around Y axis
sqrt(0.5)	0	0	-sqrt(0.5)	-90° rotation around Z axis
*/

int main()
{
	static const auto gr = 1.61803398875;
	auto cone = make_cone({0, 0, 2*gr}, {0, 0, 0}, 1.61803398875, 2*std::numeric_limits<double>::epsilon(), 16);

	{
		std::ofstream os("geom_validate_rotations.off");
		os << format::off << cone;
	}

	{
		auto x = rotate(cone, 0, 1, 0, 0);
		x += cone;
		std::ofstream os("geom_validate_rotations-180x.off");
		os << format::off << x;
	}
	{
		auto x = rotate(cone, 0, 0, 1, 0);
		x += cone;
		std::ofstream os("geom_validate_rotations-180y.off");
		os << format::off << x;
	}
	{
		auto x = rotate(cone, 0, 0, 0, 1);
		x += cone;
		std::ofstream os("geom_validate_rotations-180z.off");
		os << format::off << x;
	}
	
	{
		auto x = rotate(cone, sqrt(0.5), sqrt(0.5), 0, 0);
		x += cone;
		std::ofstream os("geom_validate_rotations-90x.off");
		os << format::off << x;
	}
	
	return 0;
}

