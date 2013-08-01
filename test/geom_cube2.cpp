#include "geom/polyhedron.h"
#include "geom/io.h"
#include "geom/primitives.h"
#include <iostream>

using namespace geom;

int main()
{
	// Not 2-manifold...
	try
	{
		auto x1 = make_box({0, 0, 0}, {1, 1, 1});
		auto x2 = make_box({1, 1, 1}, {2, 2, 2});
		
		/*
		 * Although the shared point is an expected non-manifold situation,
		 * some solution to resolve this situation needs to be developed.
		 */
		auto x = x1 + x2;
		std::cout << format::off << x;
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << '\n';
		return 0;
	}
	return 1;
}

