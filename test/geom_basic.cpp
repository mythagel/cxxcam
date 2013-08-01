#include "geom/polyhedron.h"
#include "geom/io.h"
#include <iostream>

using namespace geom;

int main()
{
	polyhedron_t p1;
	polyhedron_t p2;
	
	auto p3 = p1 + p2;
	std::cout << format::off << p3;
	
	return 0;
}
