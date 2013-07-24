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
	auto shank = make_cone({0, 0, 100}, {0, 0, 40}, 10, 10, 64);
	auto flutes = make_cone({0, 0, 40}, {0, 0, 0}, 10, 10, 64);
	auto x = shank + flutes;
	write_off(std::cout, x);
	return 0;
}
