#include "geom/polyhedron.h"
#include "geom/io.h"
#include "geom/primitives.h"
#include <iostream>

using namespace geom;

/*
Invalid memory accesses under valgrind exposed with this test.
*/

int main()
{
	auto shank = make_cone({0, 0, 100}, {0, 0, 40}, 10, 10, 8);
	auto flutes = make_cone({0, 0, 40}, {0, 0, 0}, 10, 10, 8);
	auto x = shank + flutes;
	std::cout << format::off << x;
	return 0;
}
