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

#include "cxxcam/Path.h"
#include <tuple>
#include <boost/units/cmath.hpp>
#include "cxxcam/Math.h"

namespace cxxcam
{
namespace path
{

auto to_tuple(const step& s) -> decltype(std::tie(s.position, s.orientation))
{
	return std::tie(s.position, s.orientation);
}

static const math::quaternion_t identity{1,0,0,0};
static const units::plane_angle angular_zero;

step::step()
 : position(), orientation(identity)
{
}

bool step::operator==(const step& o) const
{
	return to_tuple(*this) == to_tuple(o);
}
bool step::operator!=(const step& o) const
{
	return to_tuple(*this) != to_tuple(o);
}

std::ostream& operator<<(std::ostream& os, const step& step)
{
	os << "position: (" << units::length_mm(step.position.x) << ", " << units::length_mm(step.position.y) << ", " << units::length_mm(step.position.z) << ") orientation: " << step.orientation;
	return os;
}

/*
 * TODO
 * Rotations need to be validated.
 */
step position2step(const Position& pos, const limits::AvailableAxes& geometry)
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
					s.orientation *= math::normalise(math::axis2quat(1, 0, 0, pos.A));
				break;
			case Axis::Type::B:
				if(pos.B != angular_zero)
					s.orientation *= math::normalise(math::axis2quat(0, 1, 0, pos.B));
				break;
			case Axis::Type::C:
				if(pos.C != angular_zero)
					s.orientation *= math::normalise(math::axis2quat(0, 0, 1, pos.C));
				break;
			case Axis::Type::U:
			case Axis::Type::V:
			case Axis::Type::W:
				// TODO how are uvw mapped into the cartesian space
				break;
		}
	}
	s.orientation = math::normalise(s.orientation);
	return s;
};

units::plane_angle pseudo_cartesian_distance(const Position& start, const Position& end)
{
	return units::plane_angle{sqrt((start.A-end.A)*(start.A-end.A) + (start.B-end.B)*(start.B-end.B) + (start.C-end.C)*(start.C-end.C))};
}

path_t expand_linear(const Position& start, const Position& end, const limits::AvailableAxes& geometry, ssize_t steps_per_mm)
{
	auto pos2step = [&geometry](const Position& pos) -> step
	{
		return position2step(pos, geometry);
	};
	
	auto s0 = pos2step(start);
	auto sn = pos2step(end);
	auto length = units::length_mm(distance(s0.position, sn.position)).value();
	auto pseudo_cartesian_length = units::plane_angle_deg{pseudo_cartesian_distance(start, end)}.value();

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

	path_t path;
	path.length = units::length{length * units::millimeters};
	path.angular_length = units::plane_angle{pseudo_cartesian_length * units::degrees};

    auto is_pure_linear = [pseudo_cartesian_length]() -> bool
    {
        return pseudo_cartesian_length < 1e6;
    };

    if(is_pure_linear() && steps_per_mm < 0)
    {
        path.path.push_back(s0);
        path.path.push_back(sn);
    }
    else
    {
        steps_per_mm = std::abs(steps_per_mm);
        auto total_steps = length * steps_per_mm;

        /*
        If the length of the movement is less than the degrees travelled in the pseudo cartesian ABC coordinate system
        then trade oversampling for undersampling by treating the degrees travelled as length units and reinterpret
        steps_per_mm as steps_per_degree and use that to sample the path.
        */
        if(length < pseudo_cartesian_length)
            total_steps = pseudo_cartesian_length * steps_per_mm;

        for(size_t s = 0; s < total_steps; ++s)
        {
            auto scale = s / static_cast<double>(total_steps);
            
            // starting at `start` move a scaled amount towards `end`
            Position p = start;
            
            p.X += axis_movement.X * scale;
            p.Y += axis_movement.Y * scale;
            p.Z += axis_movement.Z * scale;

            p.A += axis_movement.A * scale;
            p.B += axis_movement.B * scale;
            p.C += axis_movement.C * scale;

            p.U += axis_movement.U * scale;
            p.V += axis_movement.V * scale;
            p.W += axis_movement.W * scale;
            
            path.path.push_back(pos2step(p));
        }
        
        if(path.path.empty() || path.path.back() != sn)
            path.path.push_back(sn);
    }
	
	return path;
}

path_t expand_rotary(const Position& start, const Position& end, const limits::AvailableAxes& geometry, size_t steps_per_degree)
{
	auto pos2step = [&geometry](const Position& pos) -> step
	{
		return position2step(pos, geometry);
	};
	
	auto pseudo_cartesian_length = units::plane_angle_deg{pseudo_cartesian_distance(start, end)}.value();
	
	Position axis_movement;
	axis_movement.A = end.A - start.A;
	axis_movement.B = end.B - start.B;
	axis_movement.C = end.C - start.C;
	
	path_t path;
	path.angular_length = units::plane_angle{pseudo_cartesian_length * units::degrees};
	auto total_steps = pseudo_cartesian_length * steps_per_degree;
	
	for(size_t s = 0; s < total_steps; ++s)
	{
		auto scale = s / static_cast<double>(total_steps);
		
		// starting at `start` move a scaled amount towards `end`
		Position p = start;
		
		p.A += axis_movement.A * scale;
		p.B += axis_movement.B * scale;
		p.C += axis_movement.C * scale;
		
		path.path.push_back(pos2step(p));
	}
	
	auto sn = pos2step(end);
	if(path.path.empty() || path.path.back() != sn)
		path.path.push_back(sn);
	
	return path;
}

path_t expand_arc(const Position& start, const Position& end, const Position_Cartesian& center, ArcDirection dir, const math::vector_3& plane, double turns, const limits::AvailableAxes& geometry, size_t steps_per_mm)
{
	auto pos2step = [&geometry](const Position& pos) -> step
	{
		return position2step(pos, geometry);
	};
	
	auto pseudo_cartesian_length = units::plane_angle_deg{pseudo_cartesian_distance(start, end)}.value();
	
	static const double PI = 3.14159265358979323846;
	static const units::plane_angle PI2_r( 2 * PI * units::radians );

	auto helix_length = [](double r, double h, double p) -> double
	{
		double c = h / (2*PI);
		auto l = (2*PI*p) * sqrt((r*r) + (c*c));
		return l;
	};
	
	math::point_3 arc_start;
	math::point_3 arc_end;
	units::length helix;
	math::point_3 arc_center;
	
	if(plane.z == 1)
	{
		arc_start = math::point_3{start.X, start.Y, 0};
		arc_end = math::point_3{end.X, end.Y, 0};
		helix = units::length(end.Z - start.Z);
		arc_center = math::point_3{center.X, center.Y, 0};
	}
	else if(plane.y == 1)
	{
		arc_start = math::point_3{start.X, start.Z, 0};
		arc_end = math::point_3{end.X, end.Z, 0};
		helix = units::length(end.Y - start.Y);
		arc_center = math::point_3{center.X, center.Z, 0};
	}
	else if(plane.x == 1)
	{
		arc_start = math::point_3{start.Z, start.Y, 0};
		arc_end = math::point_3{end.Z, end.Y, 0};
		helix = units::length(end.X - start.X);
		arc_center = math::point_3{center.Z, center.Z, 0};
	}
	else
		throw std::runtime_error("Unsupported Arc Plane");
	
	if(!equidistant(arc_start, arc_end, arc_center, units::length{1e-6 * units::millimeters}))
		throw std::runtime_error("Arc center not equidistant from start and end points.");

	auto r = distance(arc_start, arc_center);
	auto start_theta = atan2(arc_start.y - arc_center.y, arc_start.x - arc_center.x);
	auto end_theta = atan2(arc_end.y - arc_center.y, arc_end.x - arc_center.x);
	auto turn_theta = PI2_r * (turns-1);
	auto delta_theta = end_theta - start_theta;
	switch(dir)
	{
		case ArcDirection::Clockwise:
		{
			if(delta_theta > units::plane_angle(0))
				delta_theta -= PI2_r;
			else if(delta_theta == units::plane_angle(0))
				delta_theta = -PI2_r;
			break;
		}
		case ArcDirection::CounterClockwise:
		{
			if(delta_theta < units::plane_angle(0))
				delta_theta += PI2_r;
			else if(delta_theta == units::plane_angle(0))
				delta_theta = PI2_r;
			break;
		}
	}
	
	turn_theta += fabs(delta_theta);
	
	auto length = helix_length(units::length_mm(r).value(), units::length_mm{helix / units::plane_angle_rads{turn_theta}.value()}.value(), units::plane_angle_rads(turn_theta / (2*PI)).value());
	size_t total_steps = length * steps_per_mm;
	
	/*
	As for linear movement, if the length of the movement is less than the degrees travelled in the pseudo cartesian ABC coordinate system
	then trade oversampling for undersampling by treating the degrees travelled as length units and reinterpret
	steps_per_mm as steps_per_degree and use that to sample the path.
	*/
	if(length < pseudo_cartesian_length)
		total_steps = pseudo_cartesian_length * steps_per_mm;
	
	auto rads_per_step = turn_theta / static_cast<double>(total_steps);
	
	auto step_dt = delta_theta < units::plane_angle(0) ? -rads_per_step : rads_per_step;
	
	Position axis_movement;
	axis_movement.A = end.A - start.A;
	axis_movement.B = end.B - start.B;
	axis_movement.C = end.C - start.C;

	axis_movement.U = end.U - start.U;
	axis_movement.V = end.V - start.V;
	axis_movement.W = end.W - start.W;
	
	path_t path;
	path.length = units::length{length * units::millimeters};
	path.angular_length = units::plane_angle{pseudo_cartesian_length * units::degrees};
	auto t = start_theta;
	auto hdt = helix / static_cast<double>(total_steps);
	for(size_t s = 0; s < total_steps; ++s, t += step_dt)
	{
		Position p = start;
		auto sd = static_cast<double>(s);
		
		if(plane.z)
		{
			p.X = (cos(t)*r)+arc_center.x;
			p.Y = (sin(t)*r)+arc_center.y;
			p.Z += (hdt*sd);
		}
		else if(plane.y)
		{
			p.X = (cos(t)*r)+arc_center.x;
			p.Y += (hdt*sd);
			p.Z = (sin(t)*r)+arc_center.z;
		}
		else if(plane.x)
		{
			p.X += (hdt*sd);
			p.Y = (sin(t)*r)+arc_center.y;
			p.Z = (cos(t)*r)+arc_center.z;
		}
		
		auto scale = s / static_cast<double>(total_steps);
		
		p.A += axis_movement.A * scale;
		p.B += axis_movement.B * scale;
		p.C += axis_movement.C * scale;

		p.U += axis_movement.U * scale;
		p.V += axis_movement.V * scale;
		p.W += axis_movement.W * scale;
		
		path.path.push_back(pos2step(p));
	}
	
	auto sn = pos2step(end);
	if(path.path.empty() || path.path.back() != sn)
		path.path.push_back(sn);
	
	return path;
}

units::length length_linear(const Position& start, const Position& end)
{
    return math::distance({start.X, start.Y, start.Z}, {end.X, end.Y, end.Z});
}

units::length length_arc(const Position& start, const Position& end, const Position_Cartesian& center, ArcDirection dir, const math::vector_3& plane, double turns)
{
	static const double PI = 3.14159265358979323846;
	static const units::plane_angle PI2_r( 2 * PI * units::radians );

	auto helix_length = [](double r, double h, double p) -> double
	{
		double c = h / (2*PI);
		auto l = (2*PI*p) * sqrt((r*r) + (c*c));
		return l;
	};
	
	math::point_3 arc_start;
	math::point_3 arc_end;
	units::length helix;
	math::point_3 arc_center;
	
	if(plane.z == 1)
	{
		arc_start = math::point_3{start.X, start.Y, 0};
		arc_end = math::point_3{end.X, end.Y, 0};
		helix = units::length(end.Z - start.Z);
		arc_center = math::point_3{center.X, center.Y, 0};
	}
	else if(plane.y == 1)
	{
		arc_start = math::point_3{start.X, start.Z, 0};
		arc_end = math::point_3{end.X, end.Z, 0};
		helix = units::length(end.Y - start.Y);
		arc_center = math::point_3{center.X, center.Z, 0};
	}
	else if(plane.x == 1)
	{
		arc_start = math::point_3{start.Z, start.Y, 0};
		arc_end = math::point_3{end.Z, end.Y, 0};
		helix = units::length(end.X - start.X);
		arc_center = math::point_3{center.Z, center.Z, 0};
	}
	else
		throw std::runtime_error("Unsupported Arc Plane");
	
	if(!equidistant(arc_start, arc_end, arc_center, units::length{1e-6 * units::millimeters}))
		throw std::runtime_error("Arc center not equidistant from start and end points.");

	auto r = distance(arc_start, arc_center);
	auto start_theta = atan2(arc_start.y - arc_center.y, arc_start.x - arc_center.x);
	auto end_theta = atan2(arc_end.y - arc_center.y, arc_end.x - arc_center.x);
	auto turn_theta = PI2_r * (turns-1);
	auto delta_theta = end_theta - start_theta;
	switch(dir)
	{
		case ArcDirection::Clockwise:
		{
			if(delta_theta > units::plane_angle(0))
				delta_theta -= PI2_r;
			else if(delta_theta == units::plane_angle(0))
				delta_theta = -PI2_r;
			break;
		}
		case ArcDirection::CounterClockwise:
		{
			if(delta_theta < units::plane_angle(0))
				delta_theta += PI2_r;
			else if(delta_theta == units::plane_angle(0))
				delta_theta = PI2_r;
			break;
		}
	}
	
	turn_theta += fabs(delta_theta);
	
	return units::length(helix_length(units::length_mm(r).value(), units::length_mm{helix / units::plane_angle_rads{turn_theta}.value()}.value(), units::plane_angle_rads(turn_theta / (2*PI)).value()) * units::millimeters);
}

}
}

