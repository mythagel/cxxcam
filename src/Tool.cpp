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
 * Tool.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "Tool.h"

Tool::Tool()
 : m_Name("Invalid"), m_Type(Type::Mill)
{
}
Tool::Tool(const std::string& name, Type type)
 : m_Name(name), m_Type(type)
{
}

std::string Tool::Name() const
{
	return m_Name;
}

Tool::Type Tool::ToolType() const
{
	return m_Type;
}
