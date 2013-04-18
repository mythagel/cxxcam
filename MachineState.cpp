/*
 * MachineState.cpp
 *
 *  Created on: 04/05/2012
 *      Author: nicholas
 */

#include "MachineState.h"

MachineState::MachineState()
 : m_Units(Machine::units_Metric), m_Plane(Machine::plane_XY), m_Motion(Machine::motion_Absolute), m_ArcMotion(Machine::motion_Incremental), m_FeedRateMode(Machine::feedMode_UnitsPerMinute),
   m_SpindleRotation(Machine::rotation_Stop), m_FeedRate(0.0), m_SpindleSpeed(),
   m_CurrentTool(0), m_Current()
{
}

bool MachineState::operator==(const MachineState& state) const
{
	return  (m_Units == state.m_Units) &&
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
	return  (m_Units != state.m_Units) ||
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

MachineState::~MachineState()
{
}

