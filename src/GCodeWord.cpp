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

std::ostream& operator<<(std::ostream& os, const GCodeWord& word)
{
	switch(word)
	{
		case GCodeWord::A:
			os << 'A';
			os << word.Value();
			break;
		case GCodeWord::B:
			os << 'B';
			os << word.Value();
			break;
		case GCodeWord::C:
			os << 'C';
			os << word.Value();
			break;
		case GCodeWord::D:
			os << 'D';
			os << word.Value();
			break;
		case GCodeWord::F:
			os << 'F';
			os << word.Value();
			break;
		case GCodeWord::G:
			os << 'G';
			os << word.Value();
			break;
		case GCodeWord::H:
			os << 'H';
			os << word.Value();
			break;
		case GCodeWord::I:
			os << 'I';
			os << word.Value();
			break;
		case GCodeWord::J:
			os << 'J';
			os << word.Value();
			break;
		case GCodeWord::K:
			os << 'K';
			os << word.Value();
			break;
		case GCodeWord::L:
			os << 'L';
			os << word.Value();
			break;
		case GCodeWord::M:
			os << 'M';
			os << word.Value();
			break;
		case GCodeWord::P:
			os << 'P';
			os << word.Value();
			break;
		case GCodeWord::Q:
			os << 'Q';
			os << word.Value();
			break;
		case GCodeWord::R:
			os << 'R';
			os << word.Value();
			break;
		case GCodeWord::S:
			os << 'S';
			os << word.Value();
			break;
		case GCodeWord::T:
			os << 'T';
			os << word.Value();
			break;
		case GCodeWord::U:
			os << 'U';
			os << word.Value();
			break;
		case GCodeWord::V:
			os << 'V';
			os << word.Value();
			break;
		case GCodeWord::W:
			os << 'W';
			os << word.Value();
			break;
		case GCodeWord::X:
			os << 'X';
			os << word.Value();
			break;
		case GCodeWord::Y:
			os << 'Y';
			os << word.Value();
			break;
		case GCodeWord::Z:
			os << 'Z';
			os << word.Value();
			break;
	}

	if(!word.Comment().empty())
		os << " (" << word.Comment() << ")";

	return os;
}
