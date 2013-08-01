#include "Bbox.h"
#include <iostream>
#include "die_if.h"

int main()
{
	using namespace cxxcam;
	using namespace cxxcam::math;
	
	auto mm = units::millimeters;
	
	std::vector<point_3> points;
	points.push_back( { units::length{1*mm}, units::length{1*mm}, units::length{1*mm} } );
	points.push_back( { units::length{0*mm}, units::length{0*mm}, units::length{0*mm} } );
	auto b = construct(points);
	std::cout << b << '\n';
	
	die_if(b != Bbox{ {units::length{0*mm}, units::length{0*mm}, units::length{0*mm}}, {units::length{1*mm}, units::length{1*mm}, units::length{1*mm}}});
	
	return 0;
}

