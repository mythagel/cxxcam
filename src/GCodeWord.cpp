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
 * GCodeWord.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "cxxcam/GCodeWord.h"
#include <sstream>
#include <ostream>
#include <stdexcept>
#include <iomanip>

namespace cxxcam
{
namespace gcode
{

Word::Word(Code code, double value)
 : m_Code(code), m_Value(value)
{
}

Word::Word(Code code, double value, const std::string& comment)
 : m_Code(code), m_Value(value), m_Comment(comment)
{
}

Word::operator Word::Code() const
{
	return m_Code;
}

double Word::Value() const
{
	return m_Value;
}

void Word::Comment(const std::string& comment)
{
	m_Comment = comment;
}
std::string Word::Comment() const
{
	return m_Comment;
}

std::string to_string(Word::Code code)
{
	switch(code)
	{
		case Word::A:
			return "A";
		case Word::B:
			return "B";
		case Word::C:
			return "C";
		case Word::D:
			return "D";
		case Word::F:
			return "F";
		case Word::G:
			return "G";
		case Word::H:
			return "H";
		case Word::I:
			return "I";
		case Word::J:
			return "J";
		case Word::K:
			return "K";
		case Word::L:
			return "L";
		case Word::M:
			return "M";
		case Word::P:
			return "P";
		case Word::Q:
			return "Q";
		case Word::R:
			return "R";
		case Word::S:
			return "S";
		case Word::T:
			return "T";
		case Word::U:
			return "U";
		case Word::V:
			return "V";
		case Word::W:
			return "W";
		case Word::X:
			return "X";
		case Word::Y:
			return "Y";
		case Word::Z:
			return "Z";
	}

	throw std::logic_error("Unknown GCode word.");
}

std::ostream& operator<<(std::ostream& os, const Word& word)
{
	os << to_string(word);
	{
		std::ostringstream ss;
		ss << std::fixed << std::setprecision(6) << word.Value();
		auto s = ss.str();
		
		s.erase(s.find_last_not_of('0') + 1, std::string::npos);
		if(s.back() == '.')
			s.pop_back();
		os << s;
	}
	auto comment = word.Comment();

	if(!comment.empty())
		os << " (" << comment << ")";

	return os;
}

}
}

