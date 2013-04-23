/*
 * GCodeLine.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "GCodeLine.h"
#include <sstream>

GCodeLine::GCodeLine()
{
}
GCodeLine::GCodeLine(const std::string& comment)
{
	m_Comment = comment;
}
GCodeLine::GCodeLine(const GCodeWord& word, const std::string& comment)
{
	m_Words.push_back(word);
	m_Comment = comment;
}

GCodeLine::const_iterator GCodeLine::begin() const
{
	return m_Words.begin();
}
GCodeLine::const_iterator GCodeLine::end() const
{
	return m_Words.end();
}
bool GCodeLine::empty() const
{
	return m_Words.empty();
}

void GCodeLine::Comment(const std::string& comment)
{
	m_Comment = comment;
}
std::string GCodeLine::Comment() const
{
	return m_Comment;
}

GCodeLine& GCodeLine::operator+=(const GCodeWord& word)
{
	m_Words.push_back(word);
	return *this;
}

std::string GCodeLine::str() const
{
	std::stringstream s;

	GCodeLine::const_iterator word = begin();
	while(word != end())
	{
		s << word->str();

		++word;

		if(word != end())
			s << " ";
	}

	if(!Comment().empty())
	{
		if(!empty())
			s << " ";
		s << "; " << Comment();
	}

	s << '\n';

	return s.str();
}
