#include "geom/polyhedron.h"
#include "geom/io.h"
#include "geom/primitives.h"
#include "geom/translate.h"
#include <iostream>
#include "die_if.h"

using namespace geom;

/*
 * TODO validate transformation are correct.
 */
int main()
{
	auto cyl = make_cone({0, 0, 0}, {0, 0, 40}, 10, 10, 16);
	std::cout << format::off << cyl;
	auto cyl10 = translate(cyl, 10, 10, 10);
	std::cerr << "\n";
	std::cout << format::off << cyl10;

	auto cyl180x = rotate(cyl, 0, 1, 0, 0);
	std::cerr << "\n";
	std::cout << format::off << cyl180x;
	
	auto box = make_box({0, 0, 0}, {1, 1, 1});
	auto box10 = make_box({0, 0, 0}, {10, 10, 10});
	
	auto boxx10 = scale(box, 10);
	die_if(boxx10 != box10);
	return 0;
}
