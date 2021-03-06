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
#include "Math.h"
#include "Limits.h"
#include <vector>
#include <iosfwd>

namespace cxxcam
{
namespace path
{

struct step
{
	math::point_3 position;
	math::quaternion_t orientation;
	
	step();
	
	bool operator==(const step& o) const;
	bool operator!=(const step& o) const;
};

std::ostream& operator<<(std::ostream& os, const step& step);

struct path_t
{
	std::vector<step> path;
	units::length length;
	units::plane_angle angular_length;
};

/* if steps_per_mm < 0 then this will only expand linear motion IFF there is a corresponding
 * angular motion.
 * Otherwise the path will be left as the pure linear start and end steps.
 * */
path_t expand_linear(const Position& start, const Position& end, const limits::AvailableAxes& geometry, ssize_t steps_per_mm = 10);

path_t expand_rotary(const Position& start, const Position& end, const limits::AvailableAxes& geometry, size_t steps_per_degree = 10);

enum class ArcDirection
{
	Clockwise,
	CounterClockwise
};

path_t expand_arc(const Position& start, const Position& end, const Position_Cartesian& center, ArcDirection dir, const math::vector_3& plane, double turns, const limits::AvailableAxes& geometry, size_t steps_per_mm = 10);

units::length length_linear(const Position& start, const Position& end);
units::length length_arc(const Position& start, const Position& end, const Position_Cartesian& center, ArcDirection dir, const math::vector_3& plane, double turns);
}
}

#endif /* PATH_H_ */
