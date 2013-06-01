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
namespace limits
{

/*
 * Records maximum travel per axis.
 */
class Travel
{
private:
	std::map<Axis::Type, units::millimeters<double>> m_Limits;
public:
	
	void SetLimit(Axis::Type axis, units::millimeters<double> limit);
	
	// Throws cxxcam::error if out of limits
	void Validate(Axis::Type axis, units::millimeters<double> travel) const;
	
	// returns 0.0 for unspecified limit
	// likely to be changed
	units::millimeters<double> MaxTravel(Axis::Type axis) const;
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
		double torque_Nm;
		bool operator<(const sample& o) const
		{
			return rpm < o.rpm;
		}
	};
	std::set<sample> m_Samples;
public:
	void SetTorque(unsigned long rpm, double torque_Nm);
	
	// TODO
	double Get(unsigned long rpm) const;
};

/*
 * Max FeedRate per axis.
 * Query if the given feedrate is valid or the max per axis
 * All units are mm / minute
 * TODO units class for mm/m and ipm
 */
class FeedRate
{
private:
	std::map<Axis::Type, units::millimeters_per_minute<double>> m_Limits;
	units::millimeters_per_minute<double> m_Global;
public:
	void SetGlobal(units::millimeters_per_minute<double> limit_mmpm);
	void Set(Axis::Type axis, units::millimeters_per_minute<double> limit_mmpm);
	
	// Throws cxxcam::error if out of limits
	void Validate(Axis::Type axis, units::millimeters_per_minute<double> rate_mmpm) const;
	
	// returns global for unspecified limit
	// likely to be changed
	units::millimeters_per_minute<double> Max(Axis::Type axis) const;
};

/*
 * Max rapids per axis.
 * Query if the given rapid rate is valid or the max per axis
 */
class Rapids
{
private:
	std::map<Axis::Type, double> m_Limits;
	double m_Global = 0.0;
public:
	void SetGlobal(double limit);
	void Set(Axis::Type axis, double limit);
	
	// Throws cxxcam::error if out of limits
	void Validate(Axis::Type axis, double rate) const;
	
	// returns global for unspecified limit
	// likely to be changed
	double Max(Axis::Type axis) const;
};

}
}

#endif /* LIMITS_H_ */
