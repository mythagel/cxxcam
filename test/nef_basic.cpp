#include "nef/polyhedron.h"
#include "nef/io.h"
#include <iostream>

using namespace nef;

int main()
{
	polyhedron_t p1;
	polyhedron_t p2;
	
	auto p3 = p1 + p2;
	write_off(std::cout, p3);
	
	return 0;
}
