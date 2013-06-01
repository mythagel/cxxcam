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

namespace cxxcam
{

using units::millimeters;

namespace limits
{

void Travel::SetLimit(Axis::Type axis, millimeters<double> limit)
{
	m_Limits[axis] = limit;
}
void Travel::Validate(Axis::Type axis, millimeters<double> travel) const
{
	auto it = m_Limits.find(axis);
	if(it != m_Limits.end())
	{
		if(travel > it->second)
			throw error("Travel outside specified limit for axis");
	}
}
millimeters<double> Travel::MaxTravel(Axis::Type axis) const
{
	auto it = m_Limits.find(axis);
	if(it != m_Limits.end())
		return it->second;
	
	return millimeters<double>(0.0);
}

void Torque::SetTorque(unsigned long rpm, double torque_Nm)
{
	m_Samples.insert({rpm, torque_Nm});
}
double Torque::Get(unsigned long rpm) const
{
	if(m_Samples.empty())
		return 0.0;
	
	if(m_Samples.size() < 2)
		throw error("Need min & max torque samples at minimum");
	
	// TODO
	return 0.0;
}

void FeedRate::SetGlobal(units::millimeters_per_minute<double> limit_mmpm)
{
	m_Global = limit_mmpm;
}
void FeedRate::Set(Axis::Type axis, units::millimeters_per_minute<double> limit_mmpm)
{
	m_Limits[axis] = limit_mmpm;
}
void FeedRate::Validate(Axis::Type axis, units::millimeters_per_minute<double> rate_mmpm) const
{
	auto it = m_Limits.find(axis);
	if(it != m_Limits.end())
	{
		if(rate_mmpm > it->second)
			throw error("FeedRate outside specified limit for axis");
	}
	
	if(rate_mmpm > m_Global)
		throw error("FeedRate outside specified global limit");
}
units::millimeters_per_minute<double> FeedRate::Max(Axis::Type axis) const
{
	auto it = m_Limits.find(axis);
	if(it != m_Limits.end())
		return it->second;
	
	return m_Global;
}

void Rapids::SetGlobal(double limit)
{
	m_Global = limit;
}
void Rapids::Set(Axis::Type axis, double limit)
{
	m_Limits[axis] = limit;
}
void Rapids::Validate(Axis::Type axis, double rate) const
{
	auto it = m_Limits.find(axis);
	if(it != m_Limits.end())
	{
		if(rate > it->second)
			throw error("Rapid rate outside specified limit for axis");
	}
	
	if(rate > m_Global)
		throw error("Rapid rate outside specified global limit");
}
double Rapids::Max(Axis::Type axis) const
{
	auto it = m_Limits.find(axis);
	if(it != m_Limits.end())
		return it->second;
	
	return m_Global;
}

}
}

