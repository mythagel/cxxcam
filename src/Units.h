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
 * Units.h
 *
 *  Created on: 2013-05-31
 *      Author: nicholas
 */

#ifndef UNITS_H_
#define UNITS_H_

namespace cxxcam
{
namespace units
{

template <typename T>
class millimeters
{
private:
	T value;
public:
	explicit millimeters(T value = T{})
	 : value(value)
	{
	}
	operator T() const
	{
		return value;
	}
};

}
}

#endif /* UNITS_H_ */
