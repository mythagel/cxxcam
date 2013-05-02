/*
 * MachineState.cpp
 *
 *  Created on: 04/05/2012
 *      Author: nicholas
 */

#include "MachineState.h"
#include <tuple>

auto to_tuple(const MachineState& state) -> decltype(std::tie(state.m_Units, state.m_Plane, state.m_Motion, state.m_ArcMotion, state.m_FeedRateMode, state.m_SpindleRotation, state.m_FeedRate, state.m_SpindleSpeed, state.m_CurrentTool, state.m_Current))
{
	return std::tie(state.m_Units, state.m_Plane, state.m_Motion, state.m_ArcMotion, state.m_FeedRateMode, state.m_SpindleRotation, state.m_FeedRate, state.m_SpindleSpeed, state.m_CurrentTool, state.m_Current);
}

MachineState::MachineState()
 : m_Units(Machine::Units::Metric), m_Plane(Machine::Plane::XY), m_Motion(Machine::Motion::Absolute), m_ArcMotion(Machine::Motion::Incremental), m_FeedRateMode(Machine::FeedRateMode::UnitsPerMinute),
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
