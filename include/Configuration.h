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
 * Configuration.h
 *
 *  Created on: 2013-06-05
 *      Author: nicholas
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include "Machine.h"
#include "Tool.h"
#include "Stock.h"
#include "Axis.h"

namespace cxxcam
{

struct Configuration
{
	struct spindle_speed
	{
		const enum
		{
			type_Range,
			type_Discrete
		} Type;

		union
		{
			struct
			{
				const unsigned long RangeStart;
				const unsigned long RangeEnd;
			};
			const unsigned long Discrete;
		};
		
		union
		{
			struct
			{
				const double TorqueStart;
				const double TorqueEnd;
			};
			const double DiscreteTorque;
		};
		
		spindle_speed(unsigned long range_start, unsigned long range_end, double torque_start, double torque_end)
		 : Type(type_Range), RangeStart(range_start), RangeEnd(range_end),
		   TorqueStart(torque_start), TorqueEnd(torque_end)
		{
		}
		spindle_speed(unsigned long discrete_value, double torque)
		 : Type(type_Discrete), Discrete(discrete_value), DiscreteTorque(torque)
		{
		}
	};

	Machine::Type type = Machine::Type::Mill;
	Machine::Units units = Machine::Units::Metric;
	std::string gcode_variant = "Generic";
	std::string axes = "XYZABCUVW";
    std::function<void(const std::vector<gcode::Word>&, const std::string&)> gcode_callback;
	
	std::map<int, Tool> tools;
	std::vector<spindle_speed> spindle_speeds;
	
	double max_feed_rate = 0.0;
	std::map<Axis::Type, double> axis_max_feed_rates;
	
	double rapid_rate = 0.0;
	std::map<Axis::Type, double> axis_rapid_rates;
	
	Stock stock;
	
	std::unique_ptr<Machine> Construct() const;
};

}

#endif /* CONFIGURATION_H_ */
