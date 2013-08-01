#include "geom/polyhedron.h"
#include "geom/io.h"
#include "geom/primitives.h"
#include <iostream>

using namespace geom;

int main()
{
	auto x1 = make_sphere({0, 0, 0}, 2, 16);
	auto x2 = make_sphere({1, 1, 1}, 1, 16);
	auto x = x1 + x2;
	std::cout << format::off << x;
	return 0;
}
