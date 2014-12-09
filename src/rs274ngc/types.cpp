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
 * types.cpp
 *
 *  Created on: 2013-08-27
 *      Author: nicholas
 */

#include "types.h"

Tool::Tool()
 : id(), length(), diameter()
{
}

Position::Position()
 : x(), y(), z(), a(), b(), c()
{
}
Position::Position(double x, double y, double z, double a, double b, double c)
 : x(x), y(y), z(z), a(a), b(b), c(c)
{
}
Position::Position(double x, double y, double z)
 : x(x), y(y), z(z), a(), b(), c()
{
}

Position Position::operator+(const Position& p) const
{
	return {x+p.x, y+p.y, z+p.z, a+p.a, b+p.b, c+p.c};
}
Position Position::operator-(const Position& p) const
{
	return {x-p.x, y-p.y, z-p.z, a-p.a, b-p.b, c-p.c};
}

