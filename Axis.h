/*
 * Axis.h
 *
 *  Created on: 27/04/2012
 *      Author: nicholas
 */

#ifndef AXIS_H_
#define AXIS_H_

class Axis
{
public:
	enum Type
	{
		axis_X,
		axis_Y,
		axis_Z,

		axis_A,
		axis_B,
		axis_C,

		axis_U,
		axis_V,
		axis_W,
	};
protected:
	Type m_Type;
	double m_Value;

	explicit Axis(Type type);
	Axis(Type type, double value);
public:

	operator Type() const;
	operator double() const;

	~Axis();
};

class X : public Axis
{
public:
	X();
	explicit X(double value);
	~X();
};

class Y : public Axis
{
public:
	Y();
	explicit Y(double value);
	~Y();
};

class Z : public Axis
{
public:
	Z();
	explicit Z(double value);
	~Z();
};

class A : public Axis
{
public:
	A();
	explicit A(double value);
	~A();
};

class B : public Axis
{
public:
	B();
	explicit B(double value);
	~B();
};

class C : public Axis
{
public:
	C();
	explicit C(double value);
	~C();
};

#endif /* AXIS_H_ */
