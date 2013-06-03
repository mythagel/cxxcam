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
auto to_tuple(const Position_Metric& pos) -> decltype(std::tie(pos.X, pos.Y, pos.Z, pos.A, pos.B, pos.C, pos.U, pos.V, pos.W))
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

std::string Position_Metric::str() const
{
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

bool Position_Metric::operator==(const Position_Metric& pos) const
{
	return to_tuple(*this) == to_tuple(pos);
}
bool Position_Metric::operator!=(const Position_Metric& pos) const
{
	return to_tuple(*this) != to_tuple(pos);
}

Position_Metric to_millimeters(const Position& pos)
{
	return
	{
		units::length{pos.X * units::millimeters},
		units::length{pos.Y * units::millimeters},
		units::length{pos.Z * units::millimeters},
		units::plane_angle{pos.A * units::degrees},
		units::plane_angle{pos.B * units::degrees},
		units::plane_angle{pos.C * units::degrees},
		units::length{pos.U * units::millimeters},
		units::length{pos.V * units::millimeters},
		units::length{pos.W * units::millimeters},
	};
}
Position_Metric to_inches(const Position& pos)
{
	return
	{
		units::length{pos.X * units::inches},
		units::length{pos.Y * units::inches},
		units::length{pos.Z * units::inches},
		units::plane_angle{pos.A * units::degrees},
		units::plane_angle{pos.B * units::degrees},
		units::plane_angle{pos.C * units::degrees},
		units::length{pos.U * units::inches},
		units::length{pos.V * units::inches},
		units::length{pos.W * units::inches},
	};
}

}

