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
 * Math.cpp
 *
 *  Created on: 2013-07-06
 *      Author: nicholas
 */

#include "Math.h"
#include <boost/units/cmath.hpp>
#include <tuple>
#include <ostream>

namespace cxxcam
{
namespace math
{

auto to_tuple(const point_3& p) -> decltype(std::tie(p.x, p.y, p.z))
{
	return std::tie(p.x, p.y, p.z);
}
auto to_tuple(const vector_3& v) -> decltype(std::tie(v.x, v.y, v.z, v.a))
{
	return std::tie(v.x, v.y, v.z, v.a);
}

// TODO these are meaningless without tolerance.
bool point_3::operator==(const point_3& p) const
{
	return to_tuple(*this) == to_tuple(p);
}
bool point_3::operator!=(const point_3& p) const
{
	return to_tuple(*this) != to_tuple(p);
}

std::ostream& operator<<(std::ostream& os, const point_3& p)
{
	using units::length_mm;
	os << length_mm(p.x) << ", " << length_mm(p.y) << ", " << length_mm(p.z);
	return os;
}

vector_3::vector_3()
 : x(), y(), z(), a()
{
}
vector_3::vector_3(double x, double y, double z, double a)
 : x(x), y(y), z(z), a(a)
{
}
vector_3::vector_3(const quaternion_t& q)
{
	auto scale = sqrt(q.R_component_1() * q.R_component_1() + q.R_component_2() * q.R_component_2() + q.R_component_3() * q.R_component_3());
	if(scale == 0.0)
	{
		x = q.R_component_2();
		y = q.R_component_3();
		z = q.R_component_4();
	}
	else
	{
		x = q.R_component_2() / scale;
		y = q.R_component_3() / scale;
		z = q.R_component_4() / scale;
	}
	a = (acos(q.R_component_1()) * 2.0) * 57.2957795;
}

// TODO these are meaningless without tolerance.
bool vector_3::operator==(const vector_3& v) const
{
	return to_tuple(*this) == to_tuple(v);
}
bool vector_3::operator!=(const vector_3& v) const
{
	return to_tuple(*this) != to_tuple(v);
}

vector_3 normalise(const vector_3& v)
{
	auto scale = sqrt((v.x*v.x)+(v.y*v.y)+(v.z*v.z));
	if(scale == 0.0)
		return {0, 0, 0, v.a};
	
	return {v.x*(1.0/scale), v.y*(1.0/scale), v.z*(1.0/scale), v.a};
}

units::length distance(const point_3& p0, const point_3& p1)
{
	return units::length{sqrt((p0.x-p1.x)*(p0.x-p1.x) + (p0.y-p1.y)*(p0.y-p1.y) + (p0.z-p1.z)*(p0.z-p1.z))};
}

quaternion_t::value_type dot(const quaternion_t& q1, const quaternion_t& q2)
{
	return
		q1.R_component_1() * q2.R_component_1() +
		q1.R_component_2() * q2.R_component_2() +
		q1.R_component_3() * q2.R_component_3() +
		q1.R_component_4() * q2.R_component_4();
}
quaternion_t normalise(const quaternion_t& q)
{
	return q / dot(q, q);
}

quaternion_t axis2quat(const vector_3 v)
{
	return axis2quat(v.x, v.y, v.z, units::plane_angle{v.a * units::degrees});
}
quaternion_t axis2quat(double x, double y, double z, units::plane_angle theta)
{
	theta /= 2;
    auto sint = sin(theta);
	return quaternion_t{cos(theta), sint*x, sint*y, sint*z};
}

}
}

