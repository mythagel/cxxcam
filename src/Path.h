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
 * Path.h
 *
 *  Created on: 2013-06-22
 *      Author: nicholas
 */

#ifndef PATH_H_
#define PATH_H_
#include "Position.h"
#include "Units.h"
#include "Limits.h"
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/math/quaternion.hpp>
#include <vector>

namespace cxxcam
{
namespace path
{

struct step
{
	typedef boost::geometry::model::point<units::length, 3, boost::geometry::cs::cartesian> point_3d;
	typedef boost::math::quaternion<double> quaternion_t;
	
	point_3d position;
	quaternion_t orientation;
};

std::vector<step> expand_linear(const Position& start, const Position& end, const limits::AvailableAxes& geometry, size_t steps_per_mm = 10);
std::vector<step> expand_arc(const Position& start, const Position& end, const limits::AvailableAxes& geometry, size_t steps_per_mm = 10);

}
}

#endif /* PATH_H_ */
