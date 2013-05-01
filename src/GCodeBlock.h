/*
 * GCodeBlock.h
 *
 *  Created on: 28/04/2012
 *      Author: nicholas
 */

#ifndef GCODEBLOCK_H_
#define GCODEBLOCK_H_
#include <string>
#include <vector>
#include "GCodeLine.h"
#include "MachineState.h"

namespace gcode
{

/*
 * A block is a collective sequence of Lines with a similar purpose.
 * Not to be confused with a GCode 'block' which is a synonym for line
 */
class Block
{
private:
	std::string m_Name;
	std::vector<Line> m_Lines;

	MachineState m_InitialState;
public:

	typedef std::vector<Line>::const_iterator const_iterator;

	Block(const std::string& name, const MachineState& initial_state);

	std::string Name() const;
	MachineState State() const;

	const_iterator begin() const;
	const_iterator end() const;
	bool empty() const;

	void append(const Line& line);
	void append(const Word& word);

	void NewLine();

	std::string debug_str() const;

	~Block() = default;
};

}

#endif /* GCODEBLOCK_H_ */
