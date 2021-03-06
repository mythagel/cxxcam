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
 * Math.h
 *
 *  Created on: 2013-07-06
 *      Author: nicholas
 */

#ifndef CXXCAMMATH_H_
#define CXXCAMMATH_H_
#include "Units.h"
#include <boost/math/quaternion.hpp>
#include <iosfwd>

namespace cxxcam
{
namespace math
{

struct point_3
{
	units::length x;
	units::length y;
	units::length z;
	
	bool operator==(const point_3& p) const;
	bool operator!=(const point_3& p) const;
};
std::ostream& operator<<(std::ostream& os, const point_3&);

typedef boost::math::quaternion<double> quaternion_t;

struct vector_3
{
	double x;
	double y;
	double z;
	double a;
	
	vector_3();
	vector_3(double x, double y, double z, double a=0);
	explicit vector_3(const quaternion_t& q);
	bool operator==(const vector_3& v) const;
	bool operator!=(const vector_3& v) const;
};

std::ostream& operator<<(std::ostream& os, const vector_3&);

vector_3 normalise(const vector_3& v);
units::length distance(const point_3& p0, const point_3& p1);
bool equidistant(const point_3& p0, const point_3& p1, const point_3& ref, units::length tolerance);
quaternion_t::value_type dot(const quaternion_t& q1, const quaternion_t& q2);
quaternion_t normalise(const quaternion_t& q);
quaternion_t axis2quat(const vector_3 v);
quaternion_t axis2quat(double x, double y, double z, units::plane_angle theta);

}
}

#endif /* CXXCAMMATH_H_ */
