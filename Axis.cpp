/*
 * Axis.cpp
 *
 *  Created on: 27/04/2012
 *      Author: nicholas
 */

#include "Axis.h"

Axis::Axis(Type type)
 : m_Type(type), m_Value()
{
}
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

Axis::~Axis()
{
}

X::X()
 : Axis(axis_X)
{
}
X::X(double value)
 : Axis(axis_X, value)
{
}
X::~X()
{
}

Y::Y()
 : Axis(axis_Y)
{
}
Y::Y(double value)
 : Axis(axis_Y, value)
{
}
Y::~Y()
{
}

Z::Z()
 : Axis(axis_Z)
{
}
Z::Z(double value)
 : Axis(axis_Z, value)
{
}
Z::~Z()
{
}

A::A()
 : Axis(axis_A)
{
}
A::A(double value)
 : Axis(axis_A, value)
{
}
A::~A()
{
}

B::B()
 : Axis(axis_B)
{
}
B::B(double value)
 : Axis(axis_B, value)
{
}
B::~B()
{
}

C::C()
 : Axis(axis_C)
{
}
C::C(double value)
 : Axis(axis_C, value)
{
}
C::~C()
{
}
