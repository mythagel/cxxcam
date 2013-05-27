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
 * MachineState.h
 *
 *  Created on: 04/05/2012
 *      Author: nicholas
 */

#ifndef MACHINESTATE_H_
#define MACHINESTATE_H_
#include "Position.h"
#include "Machine.h"

namespace cxxcam
{

struct MachineState
{
	// Machine State
	Machine::Units m_Units;
	Machine::Plane m_Plane;
	Machine::CoordinateSystem m_CoordinateSystem;
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

}

#endif /* MACHINESTATE_H_ */
