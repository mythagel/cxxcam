/*
 * GCodeWord.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "GCodeWord.h"
#include <sstream>
#include <ostream>
#include <stdexcept>

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
	os << to_string(word) << word.m_Value;
	auto comment = word.Comment();

	if(!comment.empty())
		os << " (" << comment << ")";

	return os;
}

}
