/*
 * Simulation.cpp
 *
 *  Created on: 01/05/2012
 *      Author: nicholas
 */

#include "Simulation.h"
#include <CGAL/minkowski_sum_3.h>

Simulation::Simulation()
{
}

bool Simulation::ExpandToolPath(Nef_polyhedron tool, Nef_polyhedron tool_path, Nef_polyhedron& result)
{
	result = CGAL::minkowski_sum_3(tool, tool_path);

	return true;
}

Simulation::~Simulation()
{
}

