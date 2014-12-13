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
 * Configuration.cpp
 *
 *  Created on: 2013-06-05
 *      Author: nicholas
 */

#include "Configuration.h"
#include "make_unique.h"

namespace cxxcam
{

std::unique_ptr<Machine> Configuration::Construct() const
{
	auto machine = make_unique<Machine>(type, units, gcode_variant, gcode_callback);
	machine->SetMachineAxes(axes);
	
	for(auto& tool : tools)
		machine->AddTool(tool.first, tool.second);
	
	for(auto& speed : spindle_speeds)
	{
		switch(speed.Type)
		{
			case spindle_speed::type_Range:
				machine->AddSpindleRange(speed.RangeStart, speed.RangeEnd);
				
				machine->SetSpindleTorque(speed.RangeStart, speed.TorqueStart);
				machine->SetSpindleTorque(speed.RangeEnd, speed.TorqueEnd);
				break;
			case spindle_speed::type_Discrete:
				machine->AddSpindleDiscrete(speed.Discrete);
				
				machine->SetSpindleTorque(speed.Discrete, speed.DiscreteTorque);
				break;
		}
	}
	
	machine->SetGlobalMaxFeedrate(max_feed_rate);
	for(auto& rate : axis_max_feed_rates)
	{
		auto axis = Axis::Construct(rate.first);
		machine->SetMaxFeedrate(axis, rate.second);
	}

	machine->SetGlobalRapidRate(rapid_rate);
	for(auto& rate : axis_rapid_rates)
	{
		auto axis = Axis::Construct(rate.first);
		machine->SetRapidRate(axis, rate.second);
	}

	machine->SetStock(stock);

	return machine;
}

}

