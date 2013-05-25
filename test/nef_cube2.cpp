#include "nef/polyhedron.h"
#include "nef/io.h"
#include "nef/primitives.h"
#include <iostream>

using namespace nef;

int main()
{
	// Not 2-manifold...
	try
	{
		auto x1 = make_box(0, 0, 0, 1, 1, 1);
		auto x2 = make_box(1, 1, 1, 2, 2, 2);
		auto x = x1 + x2;
		write_off(std::cout, x);
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << '\n';
		return 0;
	}
	return 1;
}

