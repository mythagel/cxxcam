/*
 * nef_primitives.h
 *
 *  Created on: 06/05/2013
 *      Author: nicholas
 */

#ifndef NEF_PRIMITIVES_H_
#define NEF_PRIMITIVES_H_
#include "polyhedron.h"

namespace nef
{

polyhedron_t make_block(double x0, double y0, double z0, double x1, double y1, double z1);

}

#endif /* NEF_PRIMITIVES_H_ */
