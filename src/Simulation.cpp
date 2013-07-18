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

namespace cxxcam
{
namespace simulation
{

step simulate_cut(const path::step& s0, const path::step& s1, state& s)
{
	using units::length_mm;
	step sim_res;
	
	const auto& o0 = s0.orientation;
	const auto& p0 = s0.position;
	const auto& p1 = s1.position;
	
	auto tool = nef::rotate(s.tool.Model(), o0.R_component_1(), o0.R_component_2(), o0.R_component_3(), o0.R_component_4());

	nef::polyline_t path{ { {length_mm(p0.x).value(), length_mm(p0.y).value(), length_mm(p0.z).value()}, 
							{length_mm(p1.x).value(), length_mm(p1.y).value(), length_mm(p1.z).value()} } };
	
	auto tool_path = nef::glide(tool, path);
	//auto material_removed = s.stock.Model * tool_path;
	
	/*
	 * Takes far too long.
	 * Instead of calculating volume on each step (which is inaccurate and slow)
	 * accumulate the removed material and calculate it in one go at the end.
	 */
	//sim_res.swarf = units::volume( nef::volume(material_removed) * units::cubic_millimeters );
	
	s.stock.Model -= tool_path;
	
	return sim_res;
}

}
}

