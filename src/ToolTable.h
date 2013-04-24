/*
 * ToolTable.h
 *
 *  Created on: 27/04/2012
 *      Author: nicholas
 */

#ifndef TOOLTABLE_H_
#define TOOLTABLE_H_
#include <map>
#include "Tool.h"

class ToolTable
{
private:
	std::map<int, Tool> m_Tools;
public:
	ToolTable() = default;

	bool AddTool(int id, const Tool& tool);
	bool Get(int id, Tool* tool);
	bool RemoveTool(int id);

	~ToolTable() = default;
};

#endif /* TOOLTABLE_H_ */
