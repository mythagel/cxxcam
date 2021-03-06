#include "Math.h"
#include <iostream>
#include "die_if.h"
#include <boost/units/cmath.hpp>

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

	const auto x180v = vector_3(x180q);
	const auto y180v = vector_3(y180q);
	const auto z180v = vector_3(z180q);

	std::cout << "identity: quaternion"<< quaternion_t{1,0,0,0} << '\n';
	std::cout << "180deg around X: quaternion" << x180q << '\n';
	std::cout << "180deg around Y: quaternion" << y180q << '\n';
	std::cout << "180deg around Z: quaternion" << z180q << '\n';
	std::cout << '\n';
	
	std::cout << "identity: vector"<< vector_3(quaternion_t{1,0,0,0}) << '\n';
	std::cout << "180deg around X: vector" << x180v << vector_3(1, 0, 0, 180) << '\n';
	std::cout << "180deg around Y: vector" << y180v << '\n';
	std::cout << "180deg around Z: vector" << z180v << '\n';
	std::cout << '\n';

	// TODO these are meaningless without tolerance.
//	die_if(x180v != vector_3(1, 0, 0, 180), "x180v");
//	die_if(y180v != vector_3(0, 1, 0, 180), "y180v");
//	die_if(z180v != vector_3(0, 0, 1, 180), "z180v");
//	die_if(vector_3(quaternion_t{1,0,0,0}) != vector_3(0, 0, 0, 0), "identity");

	auto x = plane_angle{(180 * degrees) / 2};
	std::cout << "cos 180/2: " << cos(x.value()) << '\n';
	std::cout << "cos(90): " << cos(plane_angle{90 * degrees}) << '\n';
	std::cout << "cos(90): " << cos(90) << '\n';

	auto x180vq = axis2quat(x180v);
	auto y180vq = axis2quat(y180v);
	auto z180vq = axis2quat(z180v);
	
	std::cout << "180deg around X: quaternion" << x180vq << " xvqv: " << vector_3(x180vq) << '\n';
	std::cout << "180deg around Y: quaternion" << y180vq << " yvqv: " << vector_3(y180vq) << '\n';
	std::cout << "180deg around Z: quaternion" << z180vq << " zvqv: " << vector_3(z180vq) << '\n';
	std::cout << '\n';

	auto q = axis2quat(1, 0, 0, plane_angle{180 * degrees});
//	die_if(q != quaternion_t{0,1,0,0});
	
	auto v = vector_3(q);
	std::cout << "x180deg quat: " << q << " vec: " << v << '\n';
	return 0;
}

