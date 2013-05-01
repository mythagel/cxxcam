/*
 * GCodeLine.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "GCodeLine.h"
#include <sstream>

namespace gcode
{

Line::Line()
{
}
Line::Line(const std::string& comment)
{
	m_Comment = comment;
}
Line::Line(const Word& word, const std::string& comment)
{
	m_Words.push_back(word);
	m_Comment = comment;
}

Line::const_iterator Line::begin() const
{
	return m_Words.begin();
}
Line::const_iterator Line::end() const
{
	return m_Words.end();
}
bool Line::empty() const
{
	return m_Words.empty();
}

void Line::Comment(const std::string& comment)
{
	m_Comment = comment;
}
std::string Line::Comment() const
{
	return m_Comment;
}

Line& Line::operator+=(const Word& word)
{
	m_Words.push_back(word);
	return *this;
}

std::string Line::debug_str() const
{
	std::stringstream s;

	auto word = begin();
	while(word != end())
	{
		s << *word++;
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

}
