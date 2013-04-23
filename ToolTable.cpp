/*
 * ToolTable.cpp
 *
 *  Created on: 27/04/2012
 *      Author: nicholas
 */

#include "ToolTable.h"

bool ToolTable::AddTool(int id, const Tool& tool)
{
	auto it = m_Tools.find(id);
	if(it != m_Tools.end())
		return false;

	m_Tools.insert({id, tool});
	return true;
}
bool ToolTable::Get(int id, Tool* tool)
{
	auto it = m_Tools.find(id);
	if(it == m_Tools.end())
		return false;

	*tool = it->second;
	return true;
}
bool ToolTable::RemoveTool(int id)
{
	auto it = m_Tools.find(id);
	if(it == m_Tools.end())
		return false;

	m_Tools.erase(it);
	return true;
}
