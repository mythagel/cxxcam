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
 * Offset.cpp
 *
 *  Created on: 14/05/2013
 *      Author: nicholas
 */

#include "Offset.h"

Offset::Offset(Type type)
 : m_Type(type), m_Value()
{
}
Offset::Offset(Type type, double value)
 : m_Type(type), m_Value(value)
{
}

Offset::operator Offset::Type() const
{
	return m_Type;
}

Offset::operator double() const
{
	return m_Value;
}

I::I()
 : Offset(Type::I)
{
}
I::I(double value)
 : Offset(Type::I, value)
{
}

J::J()
 : Offset(Type::J)
{
}
J::J(double value)
 : Offset(Type::J, value)
{
}

K::K()
 : Offset(Type::K)
{
}
K::K(double value)
 : Offset(Type::K, value)
{
}
