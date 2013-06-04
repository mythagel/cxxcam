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

Axis::Axis(Type type)
 : m_Type(type)
{
}

Axis::operator Axis::Type() const
{
	return m_Type;
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

LinearAxis::LinearAxis(Type type)
 : Axis(type), m_Value()
{
	if(!is_linear(type))
		throw std::logic_error("Attempt to create Linear axis object for Rotary axis");
}
LinearAxis::LinearAxis(Type type, double value)
 : Axis(type), m_Value(value)
{
	if(!is_linear(type))
		throw std::logic_error("Attempt to create Linear axis object for Rotary axis");
}
LinearAxis::operator double() const
{
	return m_Value;
}

RotaryAxis::RotaryAxis(Type type)
 : Axis(type), m_Value()
{
	if(is_linear(type))
		throw std::logic_error("Attempt to create Rotary axis object for Linear axis");
}
RotaryAxis::RotaryAxis(Type type, double value)
 : Axis(type), m_Value(value)
{
	if(is_linear(type))
		throw std::logic_error("Attempt to create Rotary axis object for Linear axis");
}
RotaryAxis::operator double() const
{
	return m_Value;
}

X::X()
 : LinearAxis(Type::X)
{
}
X::X(double value)
 : LinearAxis(Type::X, value)
{
}

Y::Y()
 : LinearAxis(Type::Y)
{
}
Y::Y(double value)
 : LinearAxis(Type::Y, value)
{
}

Z::Z()
 : LinearAxis(Type::Z)
{
}
Z::Z(double value)
 : LinearAxis(Type::Z, value)
{
}

A::A()
 : RotaryAxis(Type::A)
{
}
A::A(double value)
 : RotaryAxis(Type::A, value)
{
}

B::B()
 : RotaryAxis(Type::B)
{
}
B::B(double value)
 : RotaryAxis(Type::B, value)
{
}

C::C()
 : RotaryAxis(Type::C)
{
}
C::C(double value)
 : RotaryAxis(Type::C, value)
{
}

U::U()
 : LinearAxis(Type::U)
{
}
U::U(double value)
 : LinearAxis(Type::U, value)
{
}

V::V()
 : LinearAxis(Type::V)
{
}
V::V(double value)
 : LinearAxis(Type::V, value)
{
}

W::W()
 : LinearAxis(Type::W)
{
}
W::W(double value)
 : LinearAxis(Type::W, value)
{
}

}

