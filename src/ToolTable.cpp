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
 * ToolTable.cpp
 *
 *  Created on: 27/04/2012
 *      Author: nicholas
 */

#include "ToolTable.h"

namespace cxxcam
{

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

}

