#include "geom/polyhedron.h"
#include "geom/io.h"
#include "geom/primitives.h"
#include "geom/translate.h"
#include <iostream>

using namespace geom;

/*
 * TODO validate transformation are correct.
 */
int main()
{
	auto cyl = make_cone({0, 0, 0}, {0, 0, 40}, 10, 10, 16);
	write_off(std::cout, cyl);
	auto cyl10 = translate(cyl, 10, 10, 10);
	std::cerr << "\n";
	write_off(std::cout, cyl10);

	auto cyl180x = rotate(cyl, 0, 1, 0, 0);
	std::cerr << "\n";
	write_off(std::cout, cyl180x);
	return 0;
}
