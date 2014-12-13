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
 * GCode.h
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#ifndef GCODE_H_
#define GCODE_H_
#include "GCodeLine.h"
#include "GCodeBlock.h"
#include <vector>
#include <string>
#include <iosfwd>
#include <functional>

namespace cxxcam
{
namespace gcode
{

/*
 * Helper class owned by Machine that assists with the storage and output of GCode.
 *
 * TODO aligned output for comments within a block.
 */
class Code
{
friend std::ostream& operator<<(std::ostream& os, const Code& gcode);
public:
	enum Variant
	{
		variant_LinuxCNC
	};
	enum EndOfLine
	{
		CR,
		LF,
		CRLF
	};

	typedef std::vector<Block>::const_iterator const_iterator;

private:
	Variant m_Variant;
	bool m_LineNumbers;
	int m_Precision;	// Number of digits after . in floating point numbers
	bool m_UpperCase;
	EndOfLine m_EndOfLine;

	std::vector<Block> m_Blocks;
	std::function<void(const std::vector<Word>&, const std::string&)> m_Callback;
protected:
	const char* eol() const;
public:
	Code(const std::string& variant);

    void SetCallback(std::function<void(const std::vector<Word>&, const std::string&)> fn);

	const_iterator begin() const;
	const_iterator end() const;
	bool empty() const;

	bool AddLine(const Line& line);
	void NewBlock(const std::string& name, const MachineState& initial_state);
	const Block& CurrentBlock();
	void EndBlock();

	std::string debug_str() const;
};

std::ostream& operator<<(std::ostream& os, const Code& gcode);

}
}

#endif /* GCODE_H_ */
