#include "nef/polyhedron.h"
#include "nef/io.h"
#include "nef/primitives.h"
#include <iostream>

using namespace nef;

int main()
{
	auto x1 = make_sphere(0, 0, 0, 2, 32);
	auto x2 = make_sphere(1, 1, 1, 1, 32);
	auto x = x1 + x2;
	write_off(std::cout, x);
	return 0;
}
