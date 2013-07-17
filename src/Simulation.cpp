/* cxxcam - C++ CAD/CAM driver library.
 * Copyright (C) 2013  Nicholas Gill
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Simulation.cpp
 *
 *  Created on: 2013-07-11
 *      Author: nicholas
 */

#include "Simulation.h"
#include "nef/translate.h"
#include "nef/ops.h"

// TESTING CRAP
#include "nef/io.h"
#include <iostream>

namespace cxxcam
{
namespace simulation
{

step simulate_cut(const path::step& s0, const path::step& s1, state& s)
{
	using units::length_mm;
	
	auto tool = s.tool.Model();
	const auto& o0 = s0.orientation;
	const auto& p0 = s0.position;
	
	const auto& o1 = s1.orientation;
	const auto& p1 = s1.position;
	
	auto t0 = nef::translate(nef::rotate(tool, o0.R_component_1(), o0.R_component_2(), o0.R_component_3(), o0.R_component_4()), length_mm(p0.x).value(), length_mm(p0.y).value(), length_mm(p0.z).value());
//	auto t1 = nef::translate(nef::rotate(tool, o1.R_component_1(), o1.R_component_2(), o1.R_component_3(), o1.R_component_4()), p1.x, p1.y, p1.z);

	nef::polyline_t path{ { {length_mm(p0.x).value(), length_mm(p0.y).value(), length_mm(p0.z).value()}, {length_mm(p1.x).value(), length_mm(p1.y).value(), length_mm(p1.z).value()} } };
	auto tool_path = nef::glide(t0, path);
	
	// TESTING CRAP
	nef::write_off(std::cout, tool_path);
	
	return {};
}

}
}

