#include "geom/polyhedron.h"
#include "geom/primitives.h"
#include "geom/io.h"
#include "geom/translate.h"
#include "geom/ops.h"
#include <iostream>
#include <fstream>

int main()
{
	using namespace geom;
	
	auto shank = make_cone( {0, 0, 60}, {0, 0, 28}, 10, 10, 8);
	auto flutes = make_cone( {0, 0, 28}, {0, 0, 0}, 10, 10, 8);
	const auto tool = shank + flutes;
	
	const auto stock = make_box( {0, 0, 0}, {50, 50, 100} );
	
	{
		auto t0 = rotate(tool, 0.707107, 0.707107, 0, 0);
		auto t00 = translate(t0, 50, 0, 90);

		auto s0 = stock - t00;
		
		std::ofstream os("geom_nonmanifold-s0.off");
		os << format::off << s0;
	}
	
	{
		auto t1 = rotate(tool, 0.718126, 0.695913, 0, 0);
		auto t01 = translate(t1, 50, 0, 90);
		
		auto s1 = stock - t01;

		std::ofstream os("geom_nonmanifold-s1.off");
		os << format::off << s1;
	}
	
	/*
	 * Unable to reproduce non-manifold error caused by broken fold
	 * operation. Will need to expand this test if it occurs again.
	 */
	{
		auto s2 = stock;
		{
			auto t0 = rotate(tool, 0.718126, 0.695913, 0, 0);
			polyline_t path{ { {49, 0, 90}, {48, 0, 90} } };
			auto tool_path = glide(t0, path);
			
			s2 -= tool_path;
		}
	
		auto t0 = rotate(tool, 0.707107, 0.707107, 0, 0);
		polyline_t path{ { {50, 0, 90}, {49, 0, 90} } };
		auto tool_path = glide(t0, path);
		
		s2 -= tool_path;
		
		std::ofstream os("geom_nonmanifold-s2.off");
		os << format::off << s2;
	}
}

