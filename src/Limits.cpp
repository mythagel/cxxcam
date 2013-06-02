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
 * Limits.cpp
 *
 *  Created on: 2013-05-31
 *      Author: nicholas
 */

#include "Limits.h"
#include "Error.h"
#include "Position.h"
#include <boost/units/cmath.hpp>

namespace cxxcam
{

namespace limits
{

void Travel::SetLimit(Axis::Type axis, units::length limit)
{
	m_Limits[axis] = limit;
}
void Travel::Validate(Axis::Type axis, units::length travel) const
{
	auto it = m_Limits.find(axis);
	if(it != m_Limits.end())
	{
		if(travel > it->second)
			throw error("Travel outside specified limit for axis");
	}
}
units::length Travel::MaxTravel(Axis::Type axis) const
{
	auto it = m_Limits.find(axis);
	if(it != m_Limits.end())
		return it->second;
	
	return {};
}

void Torque::SetTorque(unsigned long rpm, units::torque torque)
{
	m_Samples.insert({rpm, torque});
}
units::torque Torque::Get(unsigned long rpm) const
{
	if(m_Samples.empty())
		return {};
	
	if(m_Samples.size() < 2)
		throw error("Need min & max torque samples at minimum");
	
	// TODO
	return {};
}

void FeedRate::SetGlobal(units::velocity limit)
{
	m_Global = limit;
}
void FeedRate::Set(Axis::Type axis, units::velocity limit)
{
	m_Limits[axis] = limit;
}
void FeedRate::Validate(Axis::Type axis, units::velocity rate) const
{
	auto it = m_Limits.find(axis);
	if(it != m_Limits.end())
	{
		if(rate > it->second)
			throw error("FeedRate outside specified limit for axis");
	}
	
	if(rate > m_Global)
		throw error("FeedRate outside specified global limit");
}
units::velocity FeedRate::Max(Axis::Type axis) const
{
	auto it = m_Limits.find(axis);
	if(it != m_Limits.end())
		return it->second;
	
	return m_Global;
}

void Rapids::SetGlobal(units::velocity limit)
{
	m_Global = limit;
}
void Rapids::Set(Axis::Type axis, units::velocity limit)
{
	m_Limits[axis] = limit;
}
double Rapids::Duration(const Position_Metric& begin, const Position_Metric& end) const
{
	auto axis_time = [this](units::length begin, units::length end, Axis::Type axis) -> units::time
	{
		auto distance = abs(end - begin);
		auto velocity = Velocity(axis);
		return distance / velocity;
	};
	
	units::time duration;
	duration += axis_time(begin.X, end.X, Axis::Type::X);
	duration += axis_time(begin.Y, end.Y, Axis::Type::Y);
	duration += axis_time(begin.Z, end.Z, Axis::Type::Z);
	duration += axis_time(begin.A, end.A, Axis::Type::A);
	duration += axis_time(begin.B, end.B, Axis::Type::B);
	duration += axis_time(begin.C, end.C, Axis::Type::C);
	duration += axis_time(begin.U, end.U, Axis::Type::U);
	duration += axis_time(begin.V, end.V, Axis::Type::V);
	duration += axis_time(begin.W, end.W, Axis::Type::W);
	
	return duration.value();
}
units::velocity Rapids::Velocity(Axis::Type axis) const
{
	auto it = m_Limits.find(axis);
	if(it != m_Limits.end())
		return it->second;
	
	return m_Global;
}

}
}

