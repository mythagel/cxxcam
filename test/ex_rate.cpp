#include <iostream>
#include "Units.h"

using namespace cxxcam;

static const double PI = 3.14159265358979323846;

// D = Cutter diameter mm
// z = Number of cutter teeth
// n = spindle rpm
// vf = feed per minute
// fz = feed per tooth
// vc = cutting speed

auto vc(double D, double n) -> units::velocity
{
	//vc=(π×D1×n)÷1000 mpm
	return units::velocity{ (PI*D*n) * units::millimeters_per_minute };
}

auto n(units::velocity vc, double D) -> double
{
	// n=(Vc)/(πxD)
	return units::velocity_mmpm(vc).value() / (PI * D);
}

auto fz(units::velocity vf, unsigned int z, double n) -> units::length
{
	//fz=vf÷(z×n)
	return units::length{ (units::velocity_mmpm(vf).value() / (static_cast<double>(z) * n)) * units::millimeters };
}

auto vf(units::length fz, unsigned int z, double n) -> units::velocity
{
	//vf=fz×z×n
	return units::velocity{ (units::length_mm(fz).value() * static_cast<double>(z) * n) * units::millimeters_per_minute };
}


int main()
{
	std::cout << vc(125, 350) << "\n";
	std::cout << n(vc(125, 350), 125) << "\n";
	std::cout << units::length_mm(fz(units::velocity{500 * units::millimeters_per_minute}, 10, 500)) << "\n";

	return 0;
}

