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
 * Spindle.cpp
 *
 *  Created on: 08/05/2012
 *      Author: nicholas
 */

#include "cxxcam/Spindle.h"
#include <sstream>
#include <cmath>
#include <limits>
#include "cxxcam/Error.h"

namespace cxxcam
{

Spindle::Speed::Speed(unsigned long range_start, unsigned long range_end)
 : m_Type(type_Range), m_RangeStart(range_start), m_RangeEnd(range_end)
{
}
Spindle::Speed::Speed(unsigned long discrete_value)
 : m_Type(type_Discrete), m_Discrete(discrete_value)
{
}

bool Spindle::Speed::Contains(unsigned long speed) const
{
	switch(m_Type)
	{
		case type_Range:
			return (speed >= m_RangeStart) && (speed <= m_RangeEnd);
		case type_Discrete:
			return m_Discrete == speed;
	}

	return false;
}

long Spindle::Speed::Distance(unsigned long speed) const
{
	switch(m_Type)
	{
		case type_Range:
			if(speed < m_RangeStart)
				return m_RangeStart - speed;
			if(speed > m_RangeEnd)
				return m_RangeEnd - speed;
			break;
		case type_Discrete:
			if(speed < m_Discrete)
				return m_Discrete - speed;
			if(speed > m_Discrete)
				return m_Discrete - speed;
			break;
	}

	return 0;
}

bool Spindle::Speed::operator<(const Speed& other) const
{
	switch(m_Type)
	{
		case type_Range:
			switch(other.m_Type)
			{
				case type_Range:
					return m_RangeStart < other.m_RangeStart;
				case type_Discrete:
					return m_RangeEnd < other.m_Discrete;
			}
			break;
		case type_Discrete:
			switch(other.m_Type)
			{
				case type_Range:
					return m_Discrete < other.m_RangeStart;
				case type_Discrete:
					return m_Discrete < other.m_Discrete;
			}
			break;
	}

	return false;
}

Spindle::Spindle(unsigned long tolerance)
 : m_Tolerance(tolerance)
{
}

unsigned long Spindle::Normalise(unsigned long requested_speed) const
{
	if(m_Speed.empty())
		return requested_speed;
	
	for(auto& speed : m_Speed)
		if(speed.Contains(requested_speed))
			return requested_speed;

	auto real_speed = requested_speed;

	auto min_distance = std::numeric_limits<long>::max();
	for(auto& speed : m_Speed)
	{
		auto distance = speed.Distance(requested_speed);
		if(std::abs(distance) < std::abs(min_distance))
		{
			min_distance = distance;
			real_speed = requested_speed + distance;
		}
	}

	if(static_cast<unsigned long>(std::abs(min_distance)) > m_Tolerance)
	{
		std::ostringstream s;
		s << "Requested speed " << requested_speed << " outside of active tolerance (limit: " << m_Tolerance << "rpm; min: " << std::abs(min_distance) << ").";
		throw error(s.str());
	}

	return real_speed;
}

units::torque Spindle::GetTorque(unsigned long speed) const
{
	if(m_Torque.empty())
		return {};
	
	if(m_Torque.size() < 2)
		throw error("Need min & max torque samples at minimum");
	
	auto range = m_Torque.equal_range({speed, {}});
	auto& low = range.first;
	auto& high = range.second;
	
	if(low != m_Torque.end() && low->rpm == speed)
		return low->torque;
	
	if(low != m_Torque.begin())
		--low;

	auto x = static_cast<double>(speed);
	auto x0 = low->rpm;
	auto y0 = units::torque_nm(low->torque).value();
	auto x1 = high->rpm;
	auto y1 = units::torque_nm(high->torque).value();
	auto torque_nm = y0 + (y1 - y0) * ((x - x0) / (x1 - x0));
	
	return torque_nm * units::newton_meters;
}

void Spindle::AddRange(unsigned long range_start, unsigned long range_end)
{
	m_Speed.insert({range_start, range_end});
}
void Spindle::AddDiscrete(unsigned long discrete_value)
{
	m_Speed.insert(Speed(discrete_value));
}
void Spindle::SetTorque(unsigned long rpm, units::torque torque)
{
	m_Torque.insert({rpm, torque});
}

std::string Spindle::str() const
{
	std::stringstream s;

	for(auto it = begin(m_Speed); it != end(m_Speed); )
	{
		switch(it->m_Type)
		{
			case Speed::type_Range:
				s << it->m_RangeStart << "-" << it->m_RangeEnd;
				break;
			case Speed::type_Discrete:
				s << it->m_Discrete;
				break;
		}

		++it;
		if(it != end(m_Speed))
			s << ", ";
	}

	return s.str();
}

}

