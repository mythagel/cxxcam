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
 * MachineState.cpp
 *
 *  Created on: 04/05/2012
 *      Author: nicholas
 */

#include "MachineState.h"
#include <tuple>

namespace cxxcam
{

namespace
{
auto to_tuple(const MachineState& state) -> decltype(std::tie(state.m_Units, state.m_Plane, state.m_CoordinateSystem, state.m_Motion, state.m_ArcMotion, state.m_FeedRateMode, state.m_SpindleRotation, state.m_FeedRate, state.m_SpindleSpeed, state.m_CurrentTool, state.m_Current))
{
	return std::tie(state.m_Units, state.m_Plane, state.m_CoordinateSystem, state.m_Motion, state.m_ArcMotion, state.m_FeedRateMode, state.m_SpindleRotation, state.m_FeedRate, state.m_SpindleSpeed, state.m_CurrentTool, state.m_Current);
}
}

MachineState::MachineState()
 : m_Units(Machine::Units::Metric), m_Plane(Machine::Plane::XY), 
   m_CoordinateSystem(Machine::CoordinateSystem::P1),
   m_Motion(Machine::Motion::Absolute), m_ArcMotion(Machine::Motion::Incremental), 
   m_FeedRateMode(Machine::FeedRateMode::UnitsPerMinute),
   m_SpindleRotation(Machine::Rotation::Stop), m_FeedRate(0.0), m_SpindleSpeed(),
   m_CurrentTool(0), m_Current()
{
}

bool MachineState::operator==(const MachineState& state) const
{
	return to_tuple(*this) == to_tuple(state);
}
bool MachineState::operator!=(const MachineState& state) const
{
	return to_tuple(*this) != to_tuple(state);
}

}

