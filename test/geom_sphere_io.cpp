#include "geom/polyhedron.h"
#include "geom/io.h"
#include "geom/primitives.h"
#include <iostream>

using namespace geom;

int main()
{
	auto x = make_sphere({0, 0, 0}, 2, 16);
	auto o = to_object(x);
	return 0;
}
