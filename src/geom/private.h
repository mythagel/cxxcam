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
 * private.h
 *
 *  Created on: 06/05/2013
 *      Author: nicholas
 */

#ifndef GEOM_POLYHEDRON_PRIVATE_H_
#define GEOM_POLYHEDRON_PRIVATE_H_
#include "polyhedron.h"
#include "cgal.h"

namespace geom
{

struct polyhedron_t::private_t
{
	private_t();
	private_t(const Nef_polyhedron_3& nef);
	
	void regularise();
	
	Nef_polyhedron_3 nef;
};

polyhedron_t make_polyhedron(std::shared_ptr<polyhedron_t::private_t> priv);
std::shared_ptr<polyhedron_t::private_t> get_priv(polyhedron_t& polyhedron);
std::shared_ptr<const polyhedron_t::private_t> get_priv(const polyhedron_t& polyhedron);

Polyhedron_3 to_Polyhedron_3(const polyhedron_t& polyhedron);

}

#endif /* GEOM_POLYHEDRON_PRIVATE_H_ */
