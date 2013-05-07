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
 * GCodeWord.h
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#ifndef GCODEWORD_H_
#define GCODEWORD_H_
#include <string>
#include <iosfwd>

namespace gcode
{

class Word
{
	friend std::ostream& operator<<(std::ostream& os, const Word& word);
public:
	enum Code
	{
		A,	// A axis of machine
		B,	// B axis of machine
		C,	// C axis of machine
		D,	// Tool radius compensation number
		F,	// Feed rate
		G,	// General function (See table Modal Groups)
		H,	// Tool length offset index
		I,	// X offset for arcs and G87 canned cycles
		J,	// Y offset for arcs and G87 canned cycles
		K,	// Z offset for arcs and G87 canned cycles. / Spindle-Motion Ratio for G33 synchronized movements.
		L,	// generic parameter word for G10, M66 and others
		M,	// Miscellaneous function (See table Modal Groups)
		P,	// Dwell time in canned cycles and with G4. / Key used with G10.
		Q,	// Feed increment in G73, G83 canned cycles
		R,	// Arc radius or canned cycle plane
		S,	// Spindle speed
		T,	// Tool selection
		U,	// U axis of machine
		V,	// V axis of machine
		W,	// W axis of machine
		X,	// X axis of machine
		Y,	// Y axis of machine
		Z	// Z axis of machine
	};
private:
	Code m_Code;
	double m_Value;
	std::string m_Comment;
public:
	Word(Code code, double value);
	Word(Code code, double value, const std::string& comment);

	operator Code() const;

	void Comment(const std::string& comment);
	std::string Comment() const;
};

std::string to_string(Word::Code code);
std::ostream& operator<<(std::ostream& os, const Word& word);

}

#endif /* GCODEWORD_H_ */
