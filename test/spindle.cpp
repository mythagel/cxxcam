#include "Spindle.h"
#include "Error.h"
#include <iostream>
#include "Units.h"

using namespace cxxcam;

int main()
{
	{
		Spindle s;
		s.AddRange(1, 100);
		
		s.SetTorque(1, 1 * units::newton_meters);
		s.SetTorque(50, 25 * units::newton_meters);
		s.SetTorque(100, 100 * units::newton_meters);
		std::cerr << "Machine Range: " << s.str() << "\n\n";
		std::cout << "T1:   " << s.GetTorque(1) << "\n\n";
		std::cout << "T25:  " << s.GetTorque(25) << "\n\n";
		std::cout << "T50:  " << s.GetTorque(50) << "\n\n";
		std::cout << "T75:  " << s.GetTorque(75) << "\n\n";
		std::cout << "T100: " << s.GetTorque(100) << "\n\n";
	}

	{
		Spindle s;
		std::cerr << "Machine Range: " << s.str() << "\n";
		std::cerr << "500 RPM: " << s.Normalise(500) << ", 3100 RPM: " << s.Normalise(3100) << "\n";
	}
	{
		Spindle s;
		s.AddRange(0, 3000);
		std::cerr << "Machine Range: " << s.str() << "\n";
		std::cerr << "500 RPM: " << s.Normalise(500) << ", 3100 RPM: " << s.Normalise(3100) << "\n";
	}
	{
		Spindle s;
		s.AddDiscrete(300);
		s.AddDiscrete(500);
		s.AddDiscrete(1000);
		s.AddDiscrete(2500);
		s.AddDiscrete(3000);
		std::cerr << "Machine Range: " << s.str() << "\n";
		std::cerr << "500 RPM: " << s.Normalise(500) << ", 600 RPM: " << s.Normalise(600) << "\n";
	}
	{
		Spindle s;
		s.AddRange(0, 300);
		s.AddRange(500, 1000);
		s.AddRange(3000, 7000);
		s.AddDiscrete(10000);
		s.AddDiscrete(1500);
		std::cerr << "Machine Range: " << s.str() << "\n";
		std::cerr << "500 RPM: " << s.Normalise(500) << ", 1600 RPM: " << s.Normalise(1600) << ", 2900 RPM: " << s.Normalise(2900) << "\n";
	}
	
	Spindle s;
	s.AddRange(0, 3000);
	std::cerr << "Machine Range: " << s.str() << "\n";
	
	try
	{
		s.Normalise(4000);
	}
	catch(const error& ex)
	{
		// TODO exception expected!
		return 0;
	}
	return 1;
}

