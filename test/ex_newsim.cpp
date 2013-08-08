#include "Path.h"
#include "Simulation.h"

#include "Units.h"
#include "geom/primitives.h"
#include "geom/io.h"
#include "geom/translate.h"
#include "geom/ops.h"
#include "geom/query.h"
#include <iostream>

#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "fold_adjacent.h"

using namespace cxxcam;

int main()
{
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
		end.Y = length{50 * millimeters};
		end.Z = length{90 * millimeters};
		end.A = plane_angle(45 * degrees);
	
		Position_Cartesian center;
		center.X = length{50 * millimeters};
		center.Z = length{90 * millimeters};
	
		// Expand path
		limits::AvailableAxes geometry;
		steps = expand_arc(start, end, center, ArcDirection::Clockwise, {0, 0, 1}, 1, geometry, 1).path;
	}
	
	for(auto step : steps)
		std::cout << step << '\n';
	
	// Configure simulation
	auto stock = geom::make_box({0, 0, 0}, {50, 50, 100});
	geom::polyhedron_t tool;
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
		
		tool = Tool("10mm End Mill", end_mill).Model();
	}
	
	
	std::vector<geom::polyhedron_t> tool_motion;
	fold_adjacent(begin(steps), end(steps), std::back_inserter(tool_motion), 
	[&tool](const path::step& s0, const path::step& s1) -> geom::polyhedron_t
	{
		std::cout << s0 << " -> " << s1 << '\n';
		return sweep_tool(tool, s0, s1);
	});
	
	auto tool_path = geom::merge(tool_motion);
	stock -= tool_path;

	{
		std::ofstream os("ex_newsim-tool_path.off");
		os << geom::format::off << tool_path;
	}
	{
		std::ofstream os("ex_newsim-stock.off");
		os << geom::format::off << stock;
	}
	return 0;
}

