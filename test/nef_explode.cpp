#include "nef/polyhedron.h"
#include "nef/io.h"
#include "nef/primitives.h"
#include "nef/explode.h"
#include <iostream>
#include <sstream>
#include <fstream>

using namespace nef;

void simple()
{
	auto x1 = make_box({0, 0, 0}, {1, 1, 1});
	auto x2 = make_box({1.5, 1.5, 1.5}, {2, 2, 2});
	auto x = x1 + x2;
	
	auto parts = explode(x);
	for(size_t i = 0; i < parts.size(); ++i)
	{
		std::ostringstream name;
		name << "nef_explode-simple-" << i << ".off";
		std::ofstream os(name.str());
		write_off(os, parts[i]);
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
		name << "nef_explode-nonmanifold-" << i << ".off";
		std::ofstream os(name.str());
		write_off(os, parts[i]);
	}
}

int main()
{
	simple();
	// TODO
	//nonmanifold();
	return 0;
}

