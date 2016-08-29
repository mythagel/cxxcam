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
 * Bbox.h
 *
 *  Created on: 2013-08-01
 *      Author: nicholas
 */

#ifndef BBOX_H_
#define BBOX_H_
#include "Math.h"
#include <iosfwd>
#include <vector>

namespace cxxcam
{

struct Bbox
{
	math::point_3 min;
	math::point_3 max;
	
	static const Bbox zero;
	
	Bbox() = default;
	Bbox(const math::point_3& min, const math::point_3& max);
	
	bool operator==(const Bbox& b) const;
	bool operator!=(const Bbox& b) const;
	
	Bbox operator+(const Bbox& b) const;
	Bbox& operator+=(const Bbox& b);
	Bbox operator+(const math::point_3& p) const;
	Bbox& operator+=(const math::point_3& p);
};

Bbox construct(const std::vector<math::point_3>& points);

std::ostream& operator<<(std::ostream& os, const Bbox&);

}

#endif /* BBOX_H_ */
