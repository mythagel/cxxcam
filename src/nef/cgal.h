/*
 * cgal.h
 *
 *  Created on: 06/05/2013
 *      Author: nicholas
 */

#ifndef NEF_CGAL_H_
#define NEF_CGAL_H_

#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Nef_polyhedron_iostream_3.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel Exact_Kernel;
typedef CGAL::Nef_polyhedron_3<Exact_Kernel> Nef_polyhedron_3;
typedef CGAL::Polyhedron_3<Exact_Kernel> Polyhedron_3;
typedef Exact_Kernel::Point_3 Point_3;


#endif /* NEF_CGAL_H_ */
