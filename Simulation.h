/*
 * Simulation.h
 *
 *  Created on: 01/05/2012
 *      Author: nicholas
 */

#ifndef SIMULATION_H_
#define SIMULATION_H_
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Nef_polyhedron_3.h>

/*
 * TODO move CGAL out of interface decl.
 */
class Simulation
{
public:
	Simulation();

	typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
	typedef CGAL::Nef_polyhedron_3<Kernel> Nef_polyhedron;

	// Won't let me make this const.
	static bool ExpandToolPath(Nef_polyhedron tool, Nef_polyhedron tool_path, Nef_polyhedron& result);

	~Simulation();
};

#endif /* SIMULATION_H_ */
