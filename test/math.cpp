#include "Math.h"
#include <iostream>
#include <cassert>

std::ostream& operator<<(std::ostream& os, const cxxcam::math::vector_3& r)
{
	os << "(" << r.x << "," << r.y << "," << r.z << "," << r.a << ")";
	return os;
}

/*
w	x	y	z					Description
1	0	0	0					Identity quaternion, no rotation
0	1	0	0					180° turn around X axis
0	0	1	0					180° turn around Y axis
0	0	0	1					180° turn around Z axis
sqrt(0.5)	sqrt(0.5)	0	0	90° rotation around X axis
sqrt(0.5)	0	sqrt(0.5)	0	90° rotation around Y axis
sqrt(0.5)	0	0	sqrt(0.5)	90° rotation around Z axis
sqrt(0.5)	-sqrt(0.5)	0	0	-90° rotation around X axis
sqrt(0.5)	0	-sqrt(0.5)	0	-90° rotation around Y axis
sqrt(0.5)	0	0	-sqrt(0.5)	-90° rotation around Z axis

*/

int main()
{
	using namespace cxxcam::math;
	using namespace cxxcam::units;

	const quaternion_t x180{0,1,0,0};
	const quaternion_t y180{0,0,1,0};
	const quaternion_t z180{0,0,0,1};

	std::cout << "identity: vector"<< vector_3(quaternion_t{1,0,0,0}) << '\n';
	std::cout << "180deg around X: vector"<< vector_3(x180) << '\n';
	std::cout << "180deg around Y: vector"<< vector_3(y180) << '\n';
	std::cout << "180deg around Z: vector"<< vector_3(z180) << '\n';

	auto q = axis2quat(1, 0, 0, plane_angle{180 * degrees});
//	assert(q == quaternion_t{0,1,0,0});
	
	auto v = vector_3(q);
	std::cout << "x180deg quat: " << q << " vec: " << v << '\n';
	return 0;
}

