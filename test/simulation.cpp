#include "Simulation.h"
#include "Units.h"
#include "nef/primitives.h"
#include <iostream>

int main()
{
	using namespace cxxcam;
	using namespace cxxcam::simulation;
	using namespace cxxcam::units;
	
	path::step s0;
	path::step s1;
	s1.position.x = length{50 * millimeters};
	
	state s;
	s.stock.Model = nef::make_box(0, 0, 0, 100, 100, 100);
	
	{
		auto end_mill = Tool::Mill{};
		end_mill.type = Tool::Mill::Type::End;
		end_mill.center_cutting = false;
		end_mill.flutes = 4;
		end_mill.flute_length = 30;
		end_mill.core_diameter = 5;
		end_mill.cutting_length = 28;
		end_mill.mill_diameter = 10;
		end_mill.shank_diameter = 10;
		end_mill.length = 60;
		s.tool = Tool("10mm End Mill", end_mill);
	}
	
	auto step = simulate_cut(s0, s1, s);
	
	return 0;
}

