#include "Path.h"
#include "Simulation.h"

#include "Units.h"
#include "geom/primitives.h"
#include "geom/io.h"
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
	
		// Expand path
		limits::AvailableAxes geometry;
		steps = expand_linear(start, end, geometry, 1).path;
	}
	
	for(auto step : steps)
		std::cout << step << '\n';
	
	// Configure simulation
	simulation::simulation_t sim;
	sim.steps.path = steps;
	sim.stock.Model = geom::make_box({0, 0, 0}, {50, 50, 100});
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
		sim.tool = Tool("10mm End Mill", end_mill);
	}
	
	auto result = run(sim);
	
	std::cout << "Bbox: " << result.bounding_box << '\n';

	std::ofstream os("simulate_path.off");
	os << geom::format::off << result.stock.Model;
	return 0;
}

