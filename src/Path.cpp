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
 * Path.cpp
 *
 *  Created on: 2013-06-22
 *      Author: nicholas
 */

#include "Path.h"
#include <boost/units/cmath.hpp>

namespace cxxcam
{
namespace path
{

step::quaternion_t::value_type dot(const step::quaternion_t& q1, const step::quaternion_t& q2)
{
	return
		q1.R_component_1() * q2.R_component_1() +
		q1.R_component_2() * q2.R_component_2() +
		q1.R_component_3() * q2.R_component_3() +
		q1.R_component_4() * q2.R_component_4();
}
step::quaternion_t normalise(const step::quaternion_t& q)
{
	return q / dot(q, q);
}

step::quaternion_t rot_A(const units::plane_angle& theta)
{
    return normalise( step::quaternion_t{sin(theta/2.0), 0, 0, cos(theta/2.0)} );
}
step::quaternion_t rot_B(const units::plane_angle& theta)
{
	return normalise( step::quaternion_t{0, sin(theta/2.0), 0, cos(theta/2.0)} );
}
step::quaternion_t rot_C(const units::plane_angle& theta)
{

	return normalise( step::quaternion_t{0, 0, sin(theta/2.0), cos(theta/2.0)} );
}

std::vector<step> expand_linear(const Position& start, const Position& end, const limits::AvailableAxes& geometry, size_t steps_per_mm)
{
	std::vector<step> path;
	
	auto pos2step = [&geometry](const Position& pos) -> step
	{
		step s;
		for(auto axis : geometry)
		{
			switch(axis)
			{
				case Axis::Type::X:
					s.position.set<0>(units::length_mm(pos.X).value());
					break;
				case Axis::Type::Y:
					s.position.set<1>(units::length_mm(pos.Y).value());
					break;
				case Axis::Type::Z:
					s.position.set<2>(units::length_mm(pos.Z).value());
					break;
				case Axis::Type::A:
					s.orientation *= rot_A(pos.A);
					break;
				case Axis::Type::B:
					s.orientation *= rot_B(pos.B);
					break;
				case Axis::Type::C:
					s.orientation *= rot_C(pos.C);
					break;
				case Axis::Type::U:
				case Axis::Type::V:
				case Axis::Type::W:
					// TODO how are uvw mapped into the cartesian space
					break;
			}
		}
		return s;
	};
	
	path.push_back(pos2step(start));
	// TODO interpolate between start and end!
	path.push_back(pos2step(end));

	/*
	 * TODO
	 * find the path between the start and end positions given.
	 * Includes movement in rotary axes
	 * Rotations in rotary axes must be applied in order specified in geometry.
	 * have to interpolate at steps_per_mm for generated step output.
	 */
	return path;
}

std::vector<step> expand_arc(const Position&, const Position&, const limits::AvailableAxes&, size_t)
{
	std::vector<step> path;
	/*
	 * TODO
	 * find the path between the start and end positions given.
	 * Includes movement in rotary axes.
	 * Interface for arc paths WILL change.
	 */
	return path;
}

}
}

