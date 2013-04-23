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
