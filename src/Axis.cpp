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
 * Axis.cpp
 *
 *  Created on: 27/04/2012
 *      Author: nicholas
 */

#include "Axis.h"
#include <stdexcept>

namespace cxxcam
{

Axis::Axis(Type type, double value)
 : m_Type(type), m_Value(value)
{
}

Axis::operator Axis::Type() const
{
	return m_Type;
}
Axis::operator double() const
{
	return m_Value;
}

bool is_linear(Axis::Type axis)
{
	switch(axis)
	{
		case Axis::Type::A:
		case Axis::Type::B:
		case Axis::Type::C:
			return false;
		default:
			return true;
	}
}

X::X()
 : Axis(Type::X)
{
}
X::X(double value)
 : Axis(Type::X, value)
{
}

Y::Y()
 : Axis(Type::Y)
{
}
Y::Y(double value)
 : Axis(Type::Y, value)
{
}

Z::Z()
 : Axis(Type::Z)
{
}
Z::Z(double value)
 : Axis(Type::Z, value)
{
}

A::A()
 : Axis(Type::A)
{
}
A::A(double value)
 : Axis(Type::A, value)
{
}

B::B()
 : Axis(Type::B)
{
}
B::B(double value)
 : Axis(Type::B, value)
{
}

C::C()
 : Axis(Type::C)
{
}
C::C(double value)
 : Axis(Type::C, value)
{
}

U::U()
 : Axis(Type::U)
{
}
U::U(double value)
 : Axis(Type::U, value)
{
}

V::V()
 : Axis(Type::V)
{
}
V::V(double value)
 : Axis(Type::V, value)
{
}

W::W()
 : Axis(Type::W)
{
}
W::W(double value)
 : Axis(Type::W, value)
{
}

}

