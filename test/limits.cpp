#include "Limits.h"
#include "Position.h"
#include <iostream>

using namespace cxxcam;
using namespace cxxcam::limits;

int main()
{
	Rapids r;
	
	r.SetGlobal(units::velocity{500 * units::millimeters_per_minute});
	r.Set(Axis::Type::Z, units::velocity{200 * units::millimeters_per_minute});
	//r.Set(Axis::Type axis, units::angular_velocity limit);
	
	Position_Metric begin;
	begin.X = units::length{500 * units::millimeters};
	
	Position_Metric end;
	auto duration = r.Duration(begin, end);
	std::cout << "Rapid time: " << duration << "s\n";

	return 0;
}
