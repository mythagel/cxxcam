#include "Path.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, const cxxcam::math::vector_3& r)
{
	os << "(" << r.x << "," << r.y << "," << r.z << "," << r.a << ")";
	return os;
}

int main()
{
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
	
	auto steps = expand_linear(start, end, geometry, 1);
	
	for(const auto& step : steps)
	{
		std::cout << step << '\n';
		
		std::cout << vector_3(step.orientation) << '\n';
	}
	
	return 0;
}

