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
 * GCodeModal.h
 *
 *  Created on: 2013-06-05
 *      Author: nicholas
 */

#ifndef GCODEMODAL_H_
#define GCODEMODAL_H_
#include "GCodeWord.h"

namespace cxxcam
{
namespace gcode
{

/*
TODO Implement modal codes tracking. Need to be able to pass gcode through
and this will allow querying for active modal codes.


Non-modal codes (Group 0)
G4, G10 G28, G30, G53 G92, G92.1, G92.2, G92.3,

Motion (Group 1)
G0, G1, G2, G3, G33, G38.x, G73, G76, G80, G81, G82, G83, G84, G85, G86, G87, G88, G89

Plane selection (Group 2)
G17, G18, G19, G17.1, G18.1, G19.1

Distance Mode (Group 3)
G90, G91

Arc IJK Distance Mode (Group 4)
G90.1, G91.1

Feed Rate Mode (Group 5)
G93, G94, G95

Units (Group 6)
G20, G21

Cutter Diameter Compensation (Group 7)
G40, G41, G42, G41.1, G42.1

Tool Length Offset (Group 8)
G43, G43.1, G49

Canned Cycles Return Mode (Group 10)
G98, G99

Coordinate System (Group 12)
G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3

Control Mode (Group 13)
G61, G61.1, G64

Spindle Speed Mode (Group 14)
G96, G97

Lathe Diameter Mode (Group 15)
G7, G8


Stopping (Group 4)
M0, M1, M2, M30, M60

I/O on/off (Group 5)
M6 Tn

Tool Change (Group 6)
M6 Tn

Spindle (Group 7)
M3, M4, M5

Coolant (Group 8)
(M7 M8 can both be on), M9

Override Switches (Group 9)
M48, M49

User Defined (Group 10)
M100-M199

*/

class Modal
{
public:
	enum class G
	{
		Non_modal = 0,
		Motion = 1,
		Plane_selection = 2,
		Distance_Mode = 3,
		Arc_Distance_Mode = 4,
		Feed_Rate_Mode = 5,
		Units = 6,
		Cutter_Compensation = 7,
		Tool_Length_Offset = 8,
		Canned_Cycles_Return_Mode = 10,
		Coordinate_System = 12,
		Control_Mode = 13,
		Spindle_Speed_Mode = 14,
		Lathe_Diameter_Mode = 15
	};
	enum class M
	{
		Stopping = 4,
		IO = 5,
		Tool_Change = 6,
		Spindle = 7,
		Coolant = 8,
		Override_Switches = 9,
		User_Defined = 10
	};
};


}
}

#endif /* GCODEMODAL_H_ */
