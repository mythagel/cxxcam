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
	auto cyl = make_cone( {0, 0, 0}, {0, 0, 40}, 10, 10, 16);
	std::cout << format::off << cyl;
	return 0;
}
