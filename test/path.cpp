#include "Path.h"
#include <iostream>

struct euler_rot
{
	double x;
	double y;
	double z;
	double a;
};

euler_rot aa(const cxxcam::path::step::quaternion_t& q)
{
	euler_rot r;
	auto scale = sqrt(q.R_component_1() * q.R_component_1() + q.R_component_2() * q.R_component_2() + q.R_component_3() * q.R_component_3());
	r.x = q.R_component_1() / scale;
	r.y = q.R_component_2() / scale;
	r.z = q.R_component_3() / scale;
	r.a = acos(q.R_component_4()) * 2.0;
	
	return r;
}

std::ostream& operator<<(std::ostream& os, const euler_rot& r)
{
	os << "(" << r.x << "," << r.y << "," << r.z << "," << r.a << ")";
	return os;
}

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
	{
		std::cout << step << '\n';
		
		std::cout << aa(step.orientation) << '\n';
	}
	
	return 0;
}

