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
