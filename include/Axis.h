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
 * Axis.h
 *
 *  Created on: 27/04/2012
 *      Author: nicholas
 */

#ifndef AXIS_H_
#define AXIS_H_

namespace cxxcam
{

/*
 * TODO UDL type?
 * move_to(15.5_X, 4.0_Y)
 */

class Axis
{
public:
	// Need better name than Type
	enum class Type
	{
		X, Y, Z,
		A, B, C,
		U, V, W,	// Unimplemented.
	};
protected:
	Type m_Type;
	double m_Value;

	explicit Axis(Type type, double value = {});
public:

	operator Type() const;
	operator double() const;
};

bool is_linear(Axis::Type axis);

class X : public Axis
{
public:
	X();
	explicit X(double value);
};

class Y : public Axis
{
public:
	Y();
	explicit Y(double value);
};

class Z : public Axis
{
public:
	Z();
	explicit Z(double value);
};

class A : public Axis
{
public:
	A();
	explicit A(double value);
};

class B : public Axis
{
public:
	B();
	explicit B(double value);
};

class C : public Axis
{
public:
	C();
	explicit C(double value);
};

class U : public Axis
{
public:
	U();
	explicit U(double value);
};

class V : public Axis
{
public:
	V();
	explicit V(double value);
};

class W : public Axis
{
public:
	W();
	explicit W(double value);
};

}

#endif /* AXIS_H_ */
