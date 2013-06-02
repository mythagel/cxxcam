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
 * Limits.h
 *
 *  Created on: 2013-05-31
 *      Author: nicholas
 */

#ifndef LIMITS_H_
#define LIMITS_H_
#include "Axis.h"
#include "Units.h"
#include <map>
#include <set>

namespace cxxcam
{

struct Position_Metric;

namespace limits
{

class Travel
{
private:
	std::map<Axis::Type, units::length> m_Limits;
public:
	
	void SetLimit(Axis::Type axis, units::length limit);
	
	// Throws cxxcam::error if out of limits
	void Validate(Axis::Type axis, units::length travel) const;
	
	// returns 0.0 for unspecified limit
	// likely to be changed
	units::length MaxTravel(Axis::Type axis) const;
};

/*
 * Torque at various RPMs.
 * Can be queried for the power at a given rpm
 * which if not explicitly in the samples will be 
 * generated via simple linear interpolation.
 */
class Torque
{
private:
	struct sample
	{
		unsigned long rpm;
		units::torque torque;
		bool operator<(const sample& o) const
		{
			return rpm < o.rpm;
		}
	};
	std::set<sample> m_Samples;
public:
	void SetTorque(unsigned long rpm, units::torque torque);
	
	// TODO
	units::torque Get(unsigned long rpm) const;
};

class FeedRate
{
private:
	std::map<Axis::Type, units::velocity> m_Limits;
	units::velocity m_Global;
public:
	void SetGlobal(units::velocity limit);
	void Set(Axis::Type axis, units::velocity limit);
	
	// Throws cxxcam::error if out of limits
	void Validate(Axis::Type axis, units::velocity rate) const;
	
	// returns global for unspecified limit
	// likely to be changed
	units::velocity Max(Axis::Type axis) const;
};

class Rapids
{
private:
	std::map<Axis::Type, units::velocity> m_Limits;
	units::velocity m_Global;
public:
	void SetGlobal(units::velocity limit);
	void Set(Axis::Type axis, units::velocity limit);
	
	double Duration(const Position_Metric& begin, const Position_Metric& end) const;
	
	// returns global for unspecified rate
	units::velocity Velocity(Axis::Type axis) const;
};

}
}

#endif /* LIMITS_H_ */
