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
 * cgal.h
 *
 *  Created on: 06/05/2013
 *      Author: nicholas
 */

#ifndef GEOM_CGAL_H_
#define GEOM_CGAL_H_

#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Polyhedron_3.h>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Lazy_exact_nt.h>
#include <CGAL/Gmpq.h>
typedef CGAL::Simple_cartesian<CGAL::Lazy_exact_nt<CGAL::Gmpq> > Nef_Kernel;

typedef CGAL::Nef_polyhedron_3<Nef_Kernel> Nef_polyhedron_3;
typedef CGAL::Polyhedron_3<Nef_Kernel> Polyhedron_3;
typedef Nef_Kernel::Point_3 Point_3;

#endif /* GEOM_CGAL_H_ */
