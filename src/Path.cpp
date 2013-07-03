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
 * Path.cpp
 *
 *  Created on: 2013-06-22
 *      Author: nicholas
 */

#include "Path.h"

namespace cxxcam
{
namespace path
{

std::vector<pose> expand_linear(const Position&, const Position&)
{
	std::vector<pose> path;
	/*
	 * TODO
	 * find the path between the start and end positions given.
	 * Includes movement in rotary axes
	 */
	return path;
}

std::vector<pose> expand_arc(const Position&, const Position&)
{
	std::vector<pose> path;
	/*
	 * TODO
	 * find the path between the start and end positions given.
	 * Includes movement in rotary axes.
	 * Interface for arc paths WILL change.
	 */
	return path;
}

}
}

