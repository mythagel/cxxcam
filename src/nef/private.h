/*
 * nef_polyhedron_private.h
 *
 *  Created on: 06/05/2013
 *      Author: nicholas
 */

#ifndef NEF_POLYHEDRON_PRIVATE_H_
#define NEF_POLYHEDRON_PRIVATE_H_
#include "polyhedron.h"
#include "cgal.h"

namespace nef
{

struct polyhedron_t::private_t
{
	private_t() = default;
	private_t(const Nef_polyhedron_3& nef)
	 : nef(nef)
	{
	}
	Nef_polyhedron_3 nef;
};

}

#endif /* NEF_POLYHEDRON_PRIVATE_H_ */
