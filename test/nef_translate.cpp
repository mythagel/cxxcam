#include "nef/polyhedron.h"
#include "nef/io.h"
#include "nef/primitives.h"
#include "nef/translate.h"
#include <iostream>

using namespace nef;

/*
 * TODO validate transformation are correct.
 */
int main()
{
	auto cyl = make_cone(0, 0, 0, 0, 0, 40, 10, 10, 16);
	write_off(std::cout, cyl);
	auto cyl10 = translate(cyl, 10, 10, 10);
	std::cerr << "\n";
	write_off(std::cout, cyl10);

	auto cyl180x = rotate(cyl, 0, 1, 0, 0);
	std::cerr << "\n";
	write_off(std::cout, cyl180x);
	return 0;
}
