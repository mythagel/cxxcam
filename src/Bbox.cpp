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
 * Bbox.cpp
 *
 *  Created on: 2013-08-01
 *      Author: nicholas
 */

#include "cxxcam/Bbox.h"
#include <algorithm>
#include <numeric>
#include <ostream>

namespace cxxcam
{

auto to_tuple(const Bbox& b) -> decltype(std::tie(b.min, b.max))
{
	return std::tie(b.min, b.max);
}

const Bbox Bbox::zero{};

Bbox::Bbox(const math::point_3& min, const math::point_3& max)
 : min(min), max(max)
{
}

bool Bbox::operator==(const Bbox& b) const
{
	return to_tuple(*this) == to_tuple(b);
}
bool Bbox::operator!=(const Bbox& b) const
{
	return to_tuple(*this) != to_tuple(b);
}

Bbox Bbox::operator+(const Bbox& b) const
{
	math::point_3 nmin { std::min(min.x, b.min.x), std::min(min.y, b.min.y), std::min(min.z, b.min.z) };
	math::point_3 nmax { std::max(max.x, b.max.x), std::max(max.y, b.max.y), std::max(max.z, b.max.z) };
	return { nmin, nmax };
}
Bbox& Bbox::operator+=(const Bbox& b)
{
	min.x = std::min(min.x, b.min.x);
	min.y = std::min(min.y, b.min.y);
	min.z = std::min(min.z, b.min.z);
	max.x = std::max(max.x, b.max.x);
	max.y = std::max(max.y, b.max.y);
	max.z = std::max(max.z, b.max.z);
	
	return *this;
}
Bbox Bbox::operator+(const math::point_3& p) const
{
	math::point_3 nmin { std::min(min.x, p.x), std::min(min.y, p.y), std::min(min.z, p.z) };
	math::point_3 nmax { std::max(max.x, p.x), std::max(max.y, p.y), std::max(max.z, p.z) };
	return { nmin, nmax };
}
Bbox& Bbox::operator+=(const math::point_3& p)
{
	min.x = std::min(min.x, p.x);
	min.y = std::min(min.y, p.y);
	min.z = std::min(min.z, p.z);
	max.x = std::max(max.x, p.x);
	max.y = std::max(max.y, p.y);
	max.z = std::max(max.z, p.z);
	
	return *this;
}

Bbox construct(const std::vector<math::point_3>& points)
{
	if(points.empty())
		return {};
		
	return std::accumulate(begin(points), end(points), Bbox { points.front(), points.front() });
}

std::ostream& operator<<(std::ostream& os, const Bbox& b)
{
	os << "min: {" << b.min << "} max: {" << b.max << "}";
	return os;
}

}

