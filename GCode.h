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

/*
 * Helper class owned by Machine that assists with the output of GCode.
 */
class GCode
{
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
private:
	Variant m_Variant;
	bool m_LineNumbers;
	int m_Precision;	// Number of digits after . in floating point numbers
	bool m_UpperCase;
	EndOfLine m_EndOfLine;

	std::vector<GCodeBlock> m_Blocks;
protected:
	const char* eol() const;
public:
	GCode(const std::string& variant);

	bool AddLine(const GCodeLine& line);
	void NewBlock(const std::string& name, const MachineState& initial_state);
	GCodeBlock& CurrentBlock();
	void EndBlock();

	// Debug output
	std::string str() const;

	~GCode() = default;
};

#endif /* GCODE_H_ */
