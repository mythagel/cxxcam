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
 * GCodeLine.h
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#ifndef GCODELINE_H_
#define GCODELINE_H_
#include <vector>
#include <string>
#include "GCodeWord.h"

namespace gcode
{

class Line
{
private:
	std::vector<Word> m_Words;
	std::string m_Comment;
public:

	typedef std::vector<Word>::const_iterator const_iterator;

	Line();
	explicit Line(const std::string& comment);
	explicit Line(const Word& word, const std::string& comment = {});

	const_iterator begin() const;
	const_iterator end() const;
	bool empty() const;

	void Comment(const std::string& comment);
	std::string Comment() const;

	Line& operator+=(const Word& word);

	std::string debug_str() const;
};

}

#endif /* GCODELINE_H_ */
