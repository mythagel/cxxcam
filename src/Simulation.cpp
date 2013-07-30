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
#include "geom/translate.h"
#include "geom/ops.h"

namespace cxxcam
{
namespace simulation
{

step simulate_cut(const path::step& s0, const path::step& s1, state& s)
{
	if(s.stock.Model.empty())
		return {s0, s1, {}};
	
	using units::length_mm;
	step sim_res;
	sim_res.s0 = s0;
	sim_res.s1 = s1;
	
	const auto& o0 = s0.orientation;
	const auto& p0 = s0.position;
	const auto& p1 = s1.position;
	
	auto tool = geom::rotate(s.tool.Model(), o0.R_component_1(), o0.R_component_2(), o0.R_component_3(), o0.R_component_4());

	geom::polyhedron_t tool_path;
	if(distance(p0, p1) > units::length{0.000001 * units::millimeters})
	{
		geom::polyline_t path{ { {length_mm(p0.x).value(), length_mm(p0.y).value(), length_mm(p0.z).value()}, 
								{length_mm(p1.x).value(), length_mm(p1.y).value(), length_mm(p1.z).value()} } };
		tool_path = geom::glide(tool, path);
	}
	else
	{
		tool_path = translate(tool, length_mm(p0.x).value(), length_mm(p0.y).value(), length_mm(p0.z).value());
	}
	//auto material_removed = s.stock.Model * tool_path;
	
	/*
	 * Takes far too long.
	 * Instead of calculating volume on each step (which is inaccurate and slow)
	 * accumulate the removed material and calculate it in one go at the end.
	 */
	//sim_res.swarf = units::volume( geom::volume(material_removed) * units::cubic_millimeters );
	
	s.stock.Model -= tool_path;
	
	return sim_res;
}

}
}

