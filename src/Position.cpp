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
 * Position.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "Position.h"
#include <sstream>
#include <tuple>

namespace cxxcam
{

const Position Position::zero;

namespace
{
auto to_tuple(const Position& pos) -> decltype(std::tie(pos.X, pos.Y, pos.Z, pos.A, pos.B, pos.C, pos.U, pos.V, pos.W))
{
	return std::tie(pos.X, pos.Y, pos.Z, pos.A, pos.B, pos.C, pos.U, pos.V, pos.W);
}
}

std::string Position::str() const
{
	if(*this == zero)
		return "Zero";	
	
	std::stringstream s;
	static const units::length zero;
	static const units::plane_angle angular_zero;

	if(X != zero || Y != zero || Z != zero)
	{
		s << "X: " << X << " ";
		s << "Y: " << Y << " ";
		s << "Z: " << Z << " \n";
	}

	if(A != angular_zero || B != angular_zero || C != angular_zero)
	{
		s << "A: " << A << " ";
		s << "B: " << B << " ";
		s << "C: " << C << " \n";
	}

	if(U != zero || V != zero || W != zero)
	{
		s << "U: " << U << " ";
		s << "V: " << V << " ";
		s << "W: " << W << " \n";
	}

	return s.str();
}

bool Position::operator==(const Position& pos) const
{
	return to_tuple(*this) == to_tuple(pos);
}
bool Position::operator!=(const Position& pos) const
{
	return to_tuple(*this) != to_tuple(pos);
}

}

