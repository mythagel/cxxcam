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

	const quaternion_t x180q{0,1,0,0};
	const quaternion_t y180q{0,0,1,0};
	const quaternion_t z180q{0,0,0,1};

	auto x180v = vector_3(x180q);
	auto y180v = vector_3(y180q);
	auto z180v = vector_3(z180q);
	
	std::cout << "identity: vector"<< vector_3(quaternion_t{1,0,0,0}) << '\n';
	std::cout << "180deg around X: vector"<< x180v << '\n';
	std::cout << "180deg around Y: vector"<< y180v << '\n';
	std::cout << "180deg around Z: vector"<< z180v << '\n';

	auto x180vq = axis2quat(x180v);
	auto y180vq = axis2quat(y180v);
	auto z180vq = axis2quat(z180v);
	
	std::cout << "180deg around X: quaternion"<< x180vq << '\n';
	std::cout << "180deg around Y: quaternion"<< y180vq << '\n';
	std::cout << "180deg around Z: quaternion"<< z180vq << '\n';

	auto q = axis2quat(1, 0, 0, plane_angle{180 * degrees});
//	assert(q == quaternion_t{0,1,0,0});
	
	auto v = vector_3(q);
	std::cout << "x180deg quat: " << q << " vec: " << v << '\n';
	return 0;
}

