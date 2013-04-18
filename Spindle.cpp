/*
 * Spindle.cpp
 *
 *  Created on: 08/05/2012
 *      Author: nicholas
 */

#include "Spindle.h"
#include <sstream>
#include <cmath>
#include <limits>

Spindle::Entry::Entry(unsigned long range_start, unsigned long range_end)
 : m_Type(type_Range), m_RangeStart(range_start), m_RangeEnd(range_end)
{
}
Spindle::Entry::Entry(unsigned long discrete_value)
 : m_Type(type_Discrete), m_Discrete(discrete_value)
{
}

bool Spindle::Entry::Contains(unsigned long speed) const
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

long Spindle::Entry::Distance(unsigned long speed) const
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

bool Spindle::Entry::operator<(const Entry& other) const
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

Spindle::Spindle()
{
}

unsigned long Spindle::Normalise(unsigned long requested_speed)
{
	for(std::set<Entry>::const_iterator it = m_Entries.begin(); it != m_Entries.end(); ++it)
		if(it->Contains(requested_speed))
			return requested_speed;

	unsigned long real_speed(requested_speed);

	long min_distance(std::numeric_limits<long>::max());
	for(std::set<Entry>::const_iterator it = m_Entries.begin(); it != m_Entries.end(); ++it)
	{
		long distance = it->Distance(requested_speed);
		if(std::abs(distance) < std::abs(min_distance))
		{
			min_distance = distance;
			real_speed = requested_speed + distance;
		}
	}

	return real_speed;
}

void Spindle::AddRange(unsigned long range_start, unsigned long range_end)
{
	m_Entries.insert(Entry(range_start, range_end));
}
void Spindle::AddDiscrete(unsigned long discrete_value)
{
	m_Entries.insert(Entry(discrete_value));
}

std::string Spindle::str() const
{
	std::stringstream s;

	std::set<Entry>::const_iterator it = m_Entries.begin();
	while(it != m_Entries.end())
	{
		switch(it->m_Type)
		{
			case Entry::type_Range:
				s << it->m_RangeStart << " - " << it->m_RangeEnd;
				break;
			case Entry::type_Discrete:
				s << it->m_Discrete;
				break;
		}

		++it;
		if(it != m_Entries.end())
			s << ", ";
	}

	return s.str();
}

Spindle::~Spindle()
{
}

