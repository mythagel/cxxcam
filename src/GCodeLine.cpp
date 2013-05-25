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

auto Line::begin() const -> const_iterator
{
	return m_Words.begin();
}
auto Line::end() const -> const_iterator
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
