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
 * Configuration.cpp
 *
 *  Created on: 2013-07-15
 *      Author: nicholas
 */

#include "translate.h"
#include "polyhedron.h"
#include "private.h"
#include <CGAL/Aff_transformation_3.h>
#include <algorithm>

typedef typename Nef_polyhedron_3::Aff_transformation_3 Aff_transformation_3;

namespace geom
{

polyhedron_t rotate(const polyhedron_t& polyhedron, double qw, double qx, double qy, double qz)
{
	// From http://www.cs.princeton.edu/~gewang/projects/darth/stuff/quat_faq.html#Q54
	auto xx = qx*qx;
	auto xy = qx*qy;
	auto xz = qx*qz;
	auto xw = qx*qw;

	auto yy = qy*qy;
	auto yz = qy*qz;
	auto yw = qy*qw;

	auto zz = qz*qz;
	auto zw = qz*qw;

	double mat[16];
	
	mat[0]  = 1 - 2 * ( yy + zz );
	mat[1]  =     2 * ( xy - zw );
	mat[2]  =     2 * ( xz + yw );

	mat[4]  =     2 * ( xy + zw );
	mat[5]  = 1 - 2 * ( xx + zz );
	mat[6]  =     2 * ( yz - xw );

	mat[8]  =     2 * ( xz - yw );
	mat[9]  =     2 * ( yz + xw );
	mat[10] = 1 - 2 * ( xx + yy );

	mat[3]  = mat[7] = mat[11] = mat[12] = mat[13] = mat[14] = 0;
	mat[15] = 1;

	Aff_transformation_3 rotate
		(
			mat[0], mat[4], mat[8],
			mat[1], mat[5], mat[9],
			mat[2], mat[6], mat[10],
			mat[15]
		);

	auto nef = get_priv(polyhedron)->nef;
	nef.transform(rotate);
	
	auto priv = std::make_shared<polyhedron_t::private_t>(nef);
	return make_polyhedron( std::move(priv) );
}

polyhedron_t translate(const polyhedron_t& polyhedron, double x, double y, double z)
{
	typedef typename Nef_polyhedron_3::Vector_3 Vector_3;
	Aff_transformation_3 translate(CGAL::TRANSLATION, Vector_3(x, y, z));

	auto nef = get_priv(polyhedron)->nef;
	nef.transform(translate);
	
	auto priv = std::make_shared<polyhedron_t::private_t>(nef);
	return make_polyhedron( std::move(priv) );
}

polyhedron_t scale(const polyhedron_t& polyhedron, double s)
{
	Aff_transformation_3 scale(CGAL::SCALING, s);

	auto nef = get_priv(polyhedron)->nef;
	nef.transform(scale);
	
	auto priv = std::make_shared<polyhedron_t::private_t>(nef);
	return make_polyhedron( std::move(priv) );
}

}

