#include "geom/polyhedron.h"
#include "geom/io.h"
#include "geom/primitives.h"
#include <iostream>

using namespace geom;

int main()
{
	auto empty = to_object(polyhedron_t{});
	
	auto x1 = make_box({0, 0, 0}, {1, 1, 1});
	auto x2 = make_box({0, 0, 1}, {2, 2, 2});
	auto x = x1 + x2;
	auto o = to_object(x);
	return 0;
}
