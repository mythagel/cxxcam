/*
 * MachineState.h
 *
 *  Created on: 04/05/2012
 *      Author: nicholas
 */

#ifndef MACHINESTATE_H_
#define MACHINESTATE_H_
#include "Position.h"
#include "Machine.h"

struct MachineState
{
	// Machine State
	Machine::Units m_Units;
	Machine::Plane m_Plane;
	Machine::Motion m_Motion;
	Machine::Motion m_ArcMotion;
	Machine::FeedRateMode m_FeedRateMode;
	Machine::Rotation m_SpindleRotation;

	// Volatile State
	double m_FeedRate;
	unsigned long m_SpindleSpeed;
	int m_CurrentTool;
	Position m_Current;

	MachineState();

	bool operator==(const MachineState& state) const;
	bool operator!=(const MachineState& state) const;
};

#endif /* MACHINESTATE_H_ */
