/*
 * GCodeWord.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "GCodeWord.h"
#include <sstream>

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

std::ostream& operator<<(std::ostream& os, const Word& word)
{
	switch(word)
	{
		case Word::A:
			os << 'A';
			os << word.Value();
			break;
		case Word::B:
			os << 'B';
			os << word.Value();
			break;
		case Word::C:
			os << 'C';
			os << word.Value();
			break;
		case Word::D:
			os << 'D';
			os << word.Value();
			break;
		case Word::F:
			os << 'F';
			os << word.Value();
			break;
		case Word::G:
			os << 'G';
			os << word.Value();
			break;
		case Word::H:
			os << 'H';
			os << word.Value();
			break;
		case Word::I:
			os << 'I';
			os << word.Value();
			break;
		case Word::J:
			os << 'J';
			os << word.Value();
			break;
		case Word::K:
			os << 'K';
			os << word.Value();
			break;
		case Word::L:
			os << 'L';
			os << word.Value();
			break;
		case Word::M:
			os << 'M';
			os << word.Value();
			break;
		case Word::P:
			os << 'P';
			os << word.Value();
			break;
		case Word::Q:
			os << 'Q';
			os << word.Value();
			break;
		case Word::R:
			os << 'R';
			os << word.Value();
			break;
		case Word::S:
			os << 'S';
			os << word.Value();
			break;
		case Word::T:
			os << 'T';
			os << word.Value();
			break;
		case Word::U:
			os << 'U';
			os << word.Value();
			break;
		case Word::V:
			os << 'V';
			os << word.Value();
			break;
		case Word::W:
			os << 'W';
			os << word.Value();
			break;
		case Word::X:
			os << 'X';
			os << word.Value();
			break;
		case Word::Y:
			os << 'Y';
			os << word.Value();
			break;
		case Word::Z:
			os << 'Z';
			os << word.Value();
			break;
	}

	if(!word.Comment().empty())
		os << " (" << word.Comment() << ")";

	return os;
}

}
