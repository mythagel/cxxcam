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
 * Offset.h
 *
 *  Created on: 14/05/2013
 *      Author: nicholas
 */

#ifndef OFFSET_H_
#define OFFSET_H_

/*
 * TODO UDL type?
 */

class Offset
{
public:
	// Need better name than Type
	enum class Type
	{
		I, J, K,
	};
protected:
	Type m_Type;
	double m_Value;

	explicit Offset(Type type);
	Offset(Type type, double value);
public:

	operator Type() const;
	operator double() const;
};

class I : public Offset
{
public:
	I();
	explicit I(double value);
};

class J : public Offset
{
public:
	J();
	explicit J(double value);
};

class K : public Offset
{
public:
	K();
	explicit K(double value);
};

#endif /* OFFSET_H_ */
