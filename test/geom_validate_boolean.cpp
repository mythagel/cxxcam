#include "geom/polyhedron.h"
#include "geom/io.h"
#include "geom/ops.h"
#include "geom/primitives.h"
#include <iostream>

using namespace geom;

/*
Invalid memory accesses under valgrind exposed with this test.
*/

int main()
{
	auto tool = make_cone({0, 0, 20}, {0, 0, 0}, 3, 3, 8);
	auto stock = make_box({x:0, y:0, z:0}, {x:50, y:50, z:10});
	
	{
		polyline_t path;
		path.line = { {10, 10, 10}, {10, 10, 9} };
	
		auto tool_path = glide(tool, path);
		stock -= tool_path;
	}
	
	{
		polyline_t path;
		path.line = { {10, 10, 9}, {10, 10, 8} };

		auto tool_path = glide(tool, path);
		stock -= tool_path;
	}
	
	std::cout << format::off << stock;
	return 0;
}
