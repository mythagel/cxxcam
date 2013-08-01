#include "geom/polyhedron.h"
#include "geom/io.h"
#include "geom/primitives.h"
#include "geom/explode.h"
#include <iostream>
#include <sstream>
#include <fstream>

using namespace geom;

void simple()
{
	auto x1 = make_box({0, 0, 0}, {1, 1, 1});
	auto x2 = make_box({1.5, 1.5, 1.5}, {2, 2, 2});
	auto x = x1 + x2;
	
	auto parts = explode(x);
	for(size_t i = 0; i < parts.size(); ++i)
	{
		std::ostringstream name;
		name << "geom_explode-simple-" << i << ".off";
		std::ofstream os(name.str());
		os << format::off << parts[i];
	}
}

void nonmanifold()
{
	auto x1 = make_box({0, 0, 0}, {1, 1, 1});
	auto x2 = make_box({1, 1, 1}, {2, 2, 2});
	auto x = x1 + x2;
	
	auto parts = explode(x);
	for(size_t i = 0; i < parts.size(); ++i)
	{
		std::ostringstream name;
		name << "geom_explode-nonmanifold-" << i << ".off";
		std::ofstream os(name.str());
		os << format::off << parts[i];
	}
}

int main()
{
	simple();
	// TODO
	//nonmanifold();
	return 0;
}

