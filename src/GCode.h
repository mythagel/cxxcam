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
protected:
	const char* eol() const;
public:
	Code(const std::string& variant);

	const_iterator begin() const;
	const_iterator end() const;
	bool empty() const;

	bool AddLine(const Line& line);
	void NewBlock(const std::string& name, const MachineState& initial_state);
	Block& CurrentBlock();
	void EndBlock();

	std::string debug_str() const;

	~Code() = default;
};

std::ostream& operator<<(std::ostream& os, const Code& gcode);

}

#endif /* GCODE_H_ */
