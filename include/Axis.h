/*
 * Axis.h
 *
 *  Created on: 27/04/2012
 *      Author: nicholas
 */

#ifndef AXIS_H_
#define AXIS_H_

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
		U, V, W,
	};
protected:
	Type m_Type;
	double m_Value;

	explicit Axis(Type type);
	Axis(Type type, double value);
public:

	operator Type() const;
	operator double() const;
};

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

#endif /* AXIS_H_ */
