#include "Path.h"
#include "Simulation.h"

#include "Units.h"
#include "nef/primitives.h"
#include "nef/io.h"
#include "nef/explode.h"
#include <iostream>

#include <algorithm>
#include <vector>
#include <fstream>
#include <sstream>

template<class InputIt, class OutputIt, class BinaryOperation>
OutputIt broken_binary_fold(InputIt first, InputIt last, OutputIt d_first, BinaryOperation op)
{
    if (first == last)
    	return d_first;
 
    auto acc = *first++;
    while (++first != last)
    {
        auto val = *first;
        *++d_first = op(val, acc);	// should be acc, val
        acc = std::move(val);
    }
    return ++d_first;
}

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
		end.A = plane_angle{90 * degrees};
	
		// Expand path
		limits::AvailableAxes geometry;
		steps = expand_linear(start, end, geometry, 1);
	}
	
//	for(auto step : steps)
//		std::cout << step << '\n';
	
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
	broken_binary_fold(begin(steps), end(steps), std::back_inserter(sim_res), 
	[&s](const path::step& s0, const path::step& s1) -> simulation::step
	{
		std::cout << s0 << " -> " << s1 << '\n';
		return simulate_cut(s0, s1, s);
	});

	units::volume total;
	for(auto step : sim_res)
	{
		std::cout << step.swarf << "\n";
		total += step.swarf;
	}
	std::cout << "Total: " << total << "\n";

	auto parts = nef::explode(s.stock.Model);
	for(size_t i = 0; i < parts.size(); ++i)
	{
		std::ostringstream name;
		name << "simulate_path-nintynonmanifold" << i << ".off";
		std::ofstream os(name.str());
		nef::write_off(os, parts[i]);
	}
	return 0;
}

