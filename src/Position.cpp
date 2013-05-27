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

namespace
{
auto to_tuple(const Position& pos) -> decltype(std::tie(pos.X, pos.Y, pos.Z, pos.A, pos.B, pos.C, pos.U, pos.V, pos.W))
{
	return std::tie(pos.X, pos.Y, pos.Z, pos.A, pos.B, pos.C, pos.U, pos.V, pos.W);
}
}

Position::Position()
 : X(),
   Y(),
   Z(),

   A(),
   B(),
   C(),

   U(),
   V(),
   W()
{
}

std::string Position::str() const
{
	std::stringstream s;

	if(X != 0.0 || Y != 0.0 || Z != 0.0)
	{
		s << "X: " << X << " ";
		s << "Y: " << Y << " ";
		s << "Z: " << Z << " \n";
	}

	if(A != 0.0 || B != 0.0 || C != 0.0)
	{
		s << "A: " << A << " ";
		s << "B: " << B << " ";
		s << "C: " << C << " \n";
	}

	if(U != 0.0 || V != 0.0 || W != 0.0)
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

