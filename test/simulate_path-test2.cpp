#include "Path.h"
#include "Simulation.h"

#include "Units.h"
#include "nef/primitives.h"
#include "nef/io.h"
#include <iostream>

#include <vector>
#include <fstream>
#include <sstream>
#include "fold_adjacent.h"

int main()
{
	using namespace cxxcam;
	using namespace cxxcam::simulation;
	
	std::vector<path::step> steps;
	{
		using namespace cxxcam::path;
		using namespace cxxcam::units;
		
		// Define Motion
		Position start;
		start.Z = length{90 * millimeters};
	
		Position end;
		end.X = length{50 * millimeters};
		end.Z = length{90 * millimeters};
		end.A = plane_angle{91 * degrees};
	
		// Expand path
		limits::AvailableAxes geometry;
		steps = expand_linear(start, end, geometry, 1);
	}
	
	for(auto step : steps)
		std::cout << step << '\n';
	
	// Configure simulation
	state s;
	s.stock.Model = nef::make_box(0, 0, 0, 50, 50, 100);
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
	
	std::vector<simulation::step> sim_res;
	fold_adjacent(begin(steps), end(steps), std::back_inserter(sim_res), 
	[&s](const path::step& s0, const path::step& s1) -> simulation::step
	{
		std::cerr << s0 << " -> " << s1 << '\n';
		return simulate_cut(s0, s1, s);
	});

//	Material removal volume disabled.
//	units::volume total;
//	for(auto step : sim_res)
//	{
//		std::cout << step.swarf << "\n";
//		total += step.swarf;
//	}
//	std::cout << "Total: " << total << "\n";

	std::ofstream os("simulate_path-test2.off");
	nef::write_off(os, s.stock.Model);
	return 0;
}

