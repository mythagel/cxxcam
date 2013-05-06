/*
 * ops.h
 *
 *  Created on: 06/05/2013
 *      Author: nicholas
 */

#ifndef NEF_OPS_H_
#define NEF_OPS_H_
#include "polyhedron.h"
#include <vector>

namespace nef
{

struct polyline_t
{
	struct point
	{
		double x;
		double y;
		double z;
	};
	std::vector<point> line;
};

polyhedron_t glide(const polyhedron_t& polyhedron, const polyline_t& path);
double volume(const polyhedron_t& polyhedron);

}

#endif /* NEF_OPS_H_ */
