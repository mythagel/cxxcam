#include "Spindle.h"
#include <iostream>

int main()
{
	{
		Spindle s;
		s.AddRange(0, 3000);
		std::cerr << "Machine Range: " << s.str() << "\n";
		std::cerr << "500 RPM: " << s.Normalise(500) << ", 4000 RPM: " << s.Normalise(4000) << "\n";
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
		std::cerr << "500 RPM: " << s.Normalise(500) << ", 1600 RPM: " << s.Normalise(1600) << ", 2000 RPM: " << s.Normalise(2000) << "\n";
	}
	return 0;
}

