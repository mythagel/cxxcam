/*
 * MachineState.cpp
 *
 *  Created on: 04/05/2012
 *      Author: nicholas
 */

#include "MachineState.h"

MachineState::MachineState()
 : m_Units(Machine::Units::Metric), m_Plane(Machine::Plane::XY), m_Motion(Machine::Motion::Absolute), m_ArcMotion(Machine::Motion::Incremental), m_FeedRateMode(Machine::FeedRateMode::UnitsPerMinute),
   m_SpindleRotation(Machine::Rotation::Stop), m_FeedRate(0.0), m_SpindleSpeed(),
   m_CurrentTool(0), m_Current()
{
}

bool MachineState::operator==(const MachineState& state) const
{
	return (m_Units == state.m_Units) &&
			(m_Plane == state.m_Plane) &&
			(m_Motion == state.m_Motion) &&
			(m_ArcMotion == state.m_ArcMotion) &&
			(m_FeedRateMode == state.m_FeedRateMode) &&
			(m_SpindleRotation == state.m_SpindleRotation) &&

			(m_FeedRate == state.m_FeedRate) &&
			(m_SpindleSpeed == state.m_SpindleSpeed) &&
			(m_CurrentTool == state.m_CurrentTool) &&
			(m_Current == state.m_Current);
}
bool MachineState::operator!=(const MachineState& state) const
{
	return (m_Units != state.m_Units) ||
			(m_Plane != state.m_Plane) ||
			(m_Motion != state.m_Motion) ||
			(m_ArcMotion != state.m_ArcMotion) ||
			(m_FeedRateMode != state.m_FeedRateMode) ||
			(m_SpindleRotation != state.m_SpindleRotation) ||

			(m_FeedRate != state.m_FeedRate) ||
			(m_SpindleSpeed != state.m_SpindleSpeed) ||
			(m_CurrentTool != state.m_CurrentTool) ||
			(m_Current != state.m_Current);
}
