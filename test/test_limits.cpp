#include "Limits.h"
#include "Position.h"
#include <iostream>
#include <stdexcept>
#include "die_if.h"

using namespace cxxcam;
using namespace cxxcam::limits;

void test_rapids()
{
	Rapids r;
	
	r.SetGlobal(units::velocity{500 * units::millimeters_per_minute});
	r.Set(Axis::Type::Z, units::velocity{200 * units::millimeters_per_minute});
	r.Set(Axis::Type::A, units::angular_velocity{50 * units::degrees_per_second});
	
	Position begin;
	begin.X = units::length{500 * units::millimeters};	// 60 seconds
	begin.A = units::plane_angle{50 * units::degrees};	// 1 second
	
	Position end;
	
	std::cout << "Begin: " << begin.str() << "\n";
	std::cout << "End  : " << end.str() << "\n";
	auto duration = r.Duration(begin, end);
	std::cout << "Rapid time: " << duration << "\n";
	
	die_if(duration != units::time{60 * units::second}, "Incorrect duration");
}

void test_feedrate()
{
	FeedRate r;
	r.SetGlobal(units::velocity{100 * units::millimeters_per_minute});
	r.Set(Axis::Type::Z, units::velocity{50 * units::millimeters_per_minute});
	r.Set(Axis::Type::A, units::angular_velocity{5 * units::degrees_per_second});
	
	r.Validate(Axis::Type::X, units::velocity{100 * units::millimeters_per_minute});
	
	r.Validate(Axis::Type::Z, units::velocity{20 * units::millimeters_per_minute});
	r.Validate(Axis::Type::A, units::angular_velocity{5 * units::degrees_per_second});
}

int main()
{
	test_rapids();
	test_feedrate();
	return 0;
}
