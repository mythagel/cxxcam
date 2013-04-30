/*
 * GCodeWord.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "GCodeWord.h"
#include <sstream>

GCodeWord::GCodeWord(Word word, double value)
 : m_Word(word), m_Value(value)
{
}

GCodeWord::GCodeWord(Word word, double value, const std::string& comment)
 : m_Word(word), m_Value(value), m_Comment(comment)
{
}

GCodeWord::operator GCodeWord::Word() const
{
	return m_Word;
}

double GCodeWord::Value() const
{
	return m_Value;
}

void GCodeWord::Comment(const std::string& comment)
{
	m_Comment = comment;
}
std::string GCodeWord::Comment() const
{
	return m_Comment;
}

std::string GCodeWord::str() const
{
	std::stringstream s;

	switch(m_Word)
	{
		case A:
			s << 'A';
			s << m_Value;
			break;
		case B:
			s << 'B';
			s << m_Value;
			break;
		case C:
			s << 'C';
			s << m_Value;
			break;
		case D:
			s << 'D';
			s << m_Value;
			break;
		case F:
			s << 'F';
			s << m_Value;
			break;
		case G:
			s << 'G';
			s << m_Value;
			break;
		case H:
			s << 'H';
			s << m_Value;
			break;
		case I:
			s << 'I';
			s << m_Value;
			break;
		case J:
			s << 'J';
			s << m_Value;
			break;
		case K:
			s << 'K';
			s << m_Value;
			break;
		case L:
			s << 'L';
			s << m_Value;
			break;
		case M:
			s << 'M';
			s << m_Value;
			break;
		case P:
			s << 'P';
			s << m_Value;
			break;
		case Q:
			s << 'Q';
			s << m_Value;
			break;
		case R:
			s << 'R';
			s << m_Value;
			break;
		case S:
			s << 'S';
			s << m_Value;
			break;
		case T:
			s << 'T';
			s << m_Value;
			break;
		case U:
			s << 'U';
			s << m_Value;
			break;
		case V:
			s << 'V';
			s << m_Value;
			break;
		case W:
			s << 'W';
			s << m_Value;
			break;
		case X:
			s << 'X';
			s << m_Value;
			break;
		case Y:
			s << 'Y';
			s << m_Value;
			break;
		case Z:
			s << 'Z';
			s << m_Value;
			break;
	}

	if(!Comment().empty())
	{
		s << " (" << Comment() << ")";
	}

	return s.str();
}
