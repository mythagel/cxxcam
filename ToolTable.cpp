/*
 * ToolTable.cpp
 *
 *  Created on: 27/04/2012
 *      Author: nicholas
 */

#include "ToolTable.h"

ToolTable::ToolTable()
{
}

bool ToolTable::AddTool(int id, const Tool& tool)
{
	std::map<int, Tool>::const_iterator it = m_Tools.find(id);
	if(it != m_Tools.end())
		return false;

	m_Tools.insert(std::make_pair(id, tool));
	return true;
}
bool ToolTable::Get(int id, Tool* tool)
{
	std::map<int, Tool>::const_iterator it = m_Tools.find(id);
	if(it == m_Tools.end())
		return false;

	*tool = it->second;
	return true;
}
bool ToolTable::RemoveTool(int id)
{
	std::map<int, Tool>::iterator it = m_Tools.find(id);
	if(it == m_Tools.end())
		return false;

	m_Tools.erase(it);
	return true;
}

ToolTable::~ToolTable()
{
}

