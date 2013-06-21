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
 * Position.h
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#ifndef POSITION_H_
#define POSITION_H_
#include <string>
#include "Units.h"

namespace cxxcam
{

struct Position
{
	static const Position zero;	
	
	units::length X;
	units::length Y;
	units::length Z;

	units::plane_angle A;
	units::plane_angle B;
	units::plane_angle C;

	units::length U;
	units::length V;
	units::length W;

	std::string str() const;

	bool operator==(const Position& pos) const;
	bool operator!=(const Position& pos) const;
};

}

#endif /* POSITION_H_ */
