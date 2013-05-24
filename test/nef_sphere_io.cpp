#include "nef/polyhedron.h"
#include "nef/io.h"
#include "nef/primitives.h"
#include <iostream>

using namespace nef;

int main()
{
	auto x = make_sphere(0, 0, 0, 2, 16);
	auto o = to_object(x);
	return 0;
}
