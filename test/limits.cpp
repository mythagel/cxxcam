#include "Limits.h"
#include "Position.h"
#include <iostream>

#include <boost/units/cmath.hpp>

using namespace cxxcam;
using namespace cxxcam::limits;

int main()
{
	Rapids r;
	
	r.SetGlobal(units::velocity{500 * units::millimeters_per_minute});
	r.Set(Axis::Type::Z, units::velocity{200 * units::millimeters_per_minute});
	r.Set(Axis::Type::A, units::angular_velocity{50 * units::degrees_per_second});
	
	Position_Metric begin;
	begin.X = units::length{500 * units::millimeters};	// 60 seconds
	begin.A = units::plane_angle{50 * units::degrees};	// 1 second
	
	Position_Metric end;
	auto duration = r.Duration(begin, end);
	std::cout << "Rapid time: " << duration << "\n";

	return 0;
}
