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

/*
 * A block is a collective sequence of GCodeLines with a similar purpose.
 */
class GCodeBlock
{
private:
	std::string m_Name;
	std::vector<GCodeLine> m_Lines;

	MachineState m_InitialState;
public:

	typedef std::vector<GCodeLine>::const_iterator const_iterator;

	GCodeBlock(const std::string& name, const MachineState& initial_state);

	std::string Name() const;
	MachineState State() const;

	const_iterator begin() const;
	const_iterator end() const;
	bool empty() const;

	void append(const GCodeLine& line);
	void append(const GCodeWord& word);

	void NewLine();

	std::string debug_str() const;

	~GCodeBlock() = default;
};

#endif /* GCODEBLOCK_H_ */
