#include "Path.h"
#include <iostream>

int main()
{
	using namespace cxxcam;
	using namespace cxxcam::path;
	using namespace cxxcam::units;
	
	Position start;
	Position end;
	end.X = length{50 * millimeters};
	end.Y = length{100 * millimeters};
	end.A = plane_angle{90 * degrees};
	
	limits::AvailableAxes geometry;
	
	auto steps = expand_linear(start, end, geometry);
	
	for(const auto& step : steps)
		std::cout << step << '\n';
	
	return 0;
}

