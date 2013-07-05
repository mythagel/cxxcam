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

static const step::quaternion_t identity{1,0,0,0};
static const units::plane_angle angular_zero;

step::step()
 : position(), orientation(identity)
{
}

std::ostream& operator<<(std::ostream& os, const step& step)
{
	os << "position: (" << units::length_mm(step.position.x) << ", " << units::length_mm(step.position.y) << ", " << units::length_mm(step.position.z) << ") orientation: " << step.orientation;
	return os;
}

units::length distance(const point_3& p0, const point_3& p1)
{
	return units::length{sqrt((p0.x-p1.x)*(p0.x-p1.x) + (p0.y-p1.y)*(p0.y-p1.y) + (p0.z-p1.z)*(p0.z-p1.z))};
}

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
	/*
	 * TODO
	 * Rotations are not correct.
	 */
	auto pos2step = [&geometry](const Position& pos) -> step
	{
		step s;
		for(auto axis : geometry)
		{
			switch(axis)
			{
				case Axis::Type::X:
					s.position.x = pos.X;
					break;
				case Axis::Type::Y:
					s.position.y = pos.Y;
					break;
				case Axis::Type::Z:
					s.position.z = pos.Z;
					break;
				case Axis::Type::A:
					if(pos.A != angular_zero)
						s.orientation *= rot_A(pos.A);
					break;
				case Axis::Type::B:
					if(pos.B != angular_zero)
						s.orientation *= rot_B(pos.B);
					break;
				case Axis::Type::C:
					if(pos.C != angular_zero)
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
	
	auto s0 = pos2step(start);
	auto sn = pos2step(end);
	auto length = units::length_mm(distance(s0.position, sn.position)).value();
	
	Position axis_movement;
	axis_movement.X = end.X - start.X;
	axis_movement.Y = end.Y - start.Y;
	axis_movement.Z = end.Z - start.Z;

	axis_movement.A = end.A - start.A;
	axis_movement.B = end.B - start.B;
	axis_movement.C = end.C - start.C;

	axis_movement.U = end.U - start.U;
	axis_movement.V = end.V - start.V;
	axis_movement.W = end.W - start.W;
	
	std::vector<step> path;
	auto total_steps = length * steps_per_mm;
	for(size_t s = 0; s < total_steps; ++s)
	{
		auto scale = s / static_cast<double>(total_steps);
	
		Position p;
		
		p.X = axis_movement.X * scale;
		p.Y = axis_movement.Y * scale;
		p.Z = axis_movement.Z * scale;

		p.A = axis_movement.A * scale;
		p.B = axis_movement.B * scale;
		p.C = axis_movement.C * scale;

		p.U = axis_movement.U * scale;
		p.V = axis_movement.V * scale;
		p.W = axis_movement.W * scale;
		
		path.push_back(pos2step(p));
	}
	path.push_back(sn);
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

