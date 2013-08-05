#include "Path.h"
#include <iostream>
#include "die_if.h"

void simple()
{
	std::cout << "simple\n";
	using namespace cxxcam;
	using namespace cxxcam::path;
	using namespace cxxcam::units;
	using namespace cxxcam::math;
	
	Position start;
	Position end;
	end.X = length{50 * millimeters};
	end.Y = length{100 * millimeters};
	end.A = plane_angle{180 * degrees};
	
	limits::AvailableAxes geometry;
	
	std::cout << start << " -> " << end << '\n';
	
	path::info_t info;
	auto steps = expand_linear(start, end, geometry, info, 1);
	
	for(const auto& step : steps)
	{
		std::cout << step << '\n';
		std::cout << normalise(vector_3(step.orientation)) << '\n';
	}
}

void check_start_end()
{
	std::cout << "check_start_end\n";
	using namespace cxxcam;
	using namespace cxxcam::path;
	using namespace cxxcam::units;
	using namespace cxxcam::math;
	
	Position start;
	start.Z = length{90 * millimeters};

	Position end;
	end.X = length{50 * millimeters};
	end.Z = length{90 * millimeters};
	
	limits::AvailableAxes geometry;
	
	std::cout << start << " -> " << end << '\n';
	
	path::info_t info;
	auto steps = expand_linear(start, end, geometry, info, 1);
	
	step s0;
	s0.position.z = length{90 * millimeters};
	
	step s1;
	s1.position.x = length{50 * millimeters};
	s1.position.z = length{90 * millimeters};
	
	std::cout << "Expected: " << s0 << " Actual: " << steps.front() << '\n';
	std::cout << "Expected: " << s1 << " Actual: " << steps.back() << '\n';
	
	step as0 = steps.front();
	step as1 = steps.back();
	die_if(as0 != s0, "Start position is incorrect");
	die_if(as1 != s1, "End position is incorrect");
}

void nintydegrees()
{
	std::cout << "nintydegrees\n";
	using namespace cxxcam;
	using namespace cxxcam::path;
	using namespace cxxcam::units;
	using namespace cxxcam::math;
	
	Position start;
	start.Z = length{90 * millimeters};

	Position end;
	end.X = length{50 * millimeters};
	end.Z = length{90 * millimeters};
	end.A = plane_angle{90 * degrees};
	
	limits::AvailableAxes geometry;
	
	std::cout << start << " -> " << end << '\n';
	
	path::info_t info;
	auto steps = expand_linear(start, end, geometry, info, 1);
	
	for(const auto& step : steps)
	{
		std::cout << step << '\n';
		std::cout << normalise(vector_3(step.orientation)) << '\n';
	}
}

void nintyonedegrees()
{
	std::cout << "nintyonedegrees\n";
	using namespace cxxcam;
	using namespace cxxcam::path;
	using namespace cxxcam::units;
	using namespace cxxcam::math;
	
	Position start;
	start.Z = length{90 * millimeters};

	Position end;
	end.X = length{50 * millimeters};
	end.Z = length{90 * millimeters};
	end.A = plane_angle{91 * degrees};
	
	limits::AvailableAxes geometry;
	
	std::cout << start << " -> " << end << '\n';
	
	path::info_t info;
	auto steps = expand_linear(start, end, geometry, info, 1);
	
	for(const auto& step : steps)
	{
		std::cout << step << '\n';
		std::cout << normalise(vector_3(step.orientation)) << '\n';
	}
}

void z8()
{
	std::cout << "z8\n";
	using namespace cxxcam;
	using namespace cxxcam::path;
	using namespace cxxcam::units;
	using namespace cxxcam::math;
	
	Position start;
	start.X = length{10 * millimeters};
	start.Y = length{10 * millimeters};
	start.Z = length{9 * millimeters};

	Position end;
	end.X = length{10 * millimeters};
	end.Y = length{10 * millimeters};
	end.Z = length{8 * millimeters};
	
	limits::AvailableAxes geometry;
	
	std::cout << start << " -> " << end << '\n';
	
	path::info_t info;
	auto steps = expand_linear(start, end, geometry, info, 1);
	
	die_if(steps[steps.size()-1] == steps[steps.size()-2], "Duplicate step");
	
	for(const auto& step : steps)
	{
		std::cout << step << '\n';
		std::cout << normalise(vector_3(step.orientation)) << '\n';
	}
}

int main()
{
	simple();
	check_start_end();
	nintydegrees();
	nintyonedegrees();
	z8();
	
	return 0;
}

