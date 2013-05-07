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
 * Tool.h
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#ifndef TOOL_H_
#define TOOL_H_
#include <string>
#include "nef/polyhedron.h"

/*
 * Representation of the cutting tool used to remove material from the Stock.
 * TODO create nef object for the tool object.
 */
class Tool
{
public:
	enum class Type
	{
		Mill,
		Lathe
	};
private:
	std::string m_Name;
	Type m_Type;
	nef::polyhedron_t m_Nef;

	struct Lathe
	{

	};

	struct Mill
	{
		// Whether the tool is suitable for plunge cuts.
		bool center_cutting;

		int flutes;
		double flute_length;

		double cutting_length;
		double mill_diameter;
		double shank_diameter;
		double core_diameter;

		double length;
	};
public:
	Tool();
	Tool(const std::string& name, Type type);

	std::string Name() const;
	Type ToolType() const;
};

#endif /* TOOL_H_ */
