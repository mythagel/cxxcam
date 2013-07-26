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
 * explode.cpp
 *
 *  Created on: 2013-06-25
 *      Author: nicholas
 */

#include "explode.h"
#include "polyhedron.h"
#include "cgal_explode.h"
#include "private.h"

namespace geom
{

std::vector<polyhedron_t> explode(const polyhedron_t& poly)
{
	auto P = to_Polyhedron_3(poly);

	std::vector<Polyhedron_3> polyhedra;

	typedef std::back_insert_iterator<std::vector<Polyhedron_3> > OutputIterator;
	Explode_polyhedron<Polyhedron_3, typename Polyhedron_3::Traits, OutputIterator> Explode;
	Explode.run(P, std::back_inserter(polyhedra));
	
	std::vector<polyhedron_t> exploded;
	for(auto& poly : polyhedra)
	{
		auto priv = std::make_shared<polyhedron_t::private_t>( poly );
		exploded.push_back( make_polyhedron( std::move(priv) ) );
	}
	return exploded;
}

}

