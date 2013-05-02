/*
 * Tool.h
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#ifndef TOOL_H_
#define TOOL_H_
#include <string>
#include "nef_polyhedron.h"

/*
 * Representation of the cutting tool used to remove material from the Stock.
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
	nef_polyhedron_t m_Nef;

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
