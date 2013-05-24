#include "nef/polyhedron.h"
#include "nef/io.h"
#include "nef/primitives.h"
#include <iostream>

using namespace nef;

/*
Invalid memory accesses under valgrind exposed with this test.
*/

polyhedron_t make_tool()
{
	auto shank = make_cone(0, 0, 100, 0, 0, 40, 10, 10, 64);
	auto flutes = make_cone(0, 0, 40, 0, 0, 0, 10, 10, 64);
	return shank + flutes;
}

int main()
{
	auto p4 = make_tool();
	write_off(std::cout, p4);
	
	return 0;
}
