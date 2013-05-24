#include "nef/polyhedron.h"
#include "nef/io.h"
#include "nef/primitives.h"
#include <iostream>

using namespace nef;

/*
Invalid memory accesses under valgrind exposed with this test.
*/

int main()
{
	auto cyl = make_cone(0, 0, 0, 0, 0, 40, 10, 10, 16);
	write_off(std::cout, cyl);
	return 0;
}
