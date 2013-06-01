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

namespace detail
{
template<typename T>
class scalar_unit_base
{
protected:
	T val;
	
	explicit scalar_unit_base(T val = T{})
	 : val(val)
	{
	}
public:
	explicit operator T() const
	{
		return val;
	}
	
	bool operator<(const scalar_unit_base& o) const
	{
		return val < o.val;
	}
	bool operator>(const scalar_unit_base& o) const
	{
		return val > o.val;
	}
};

template<typename T>
class length_base : public scalar_unit_base<T>
{
protected:
	explicit length_base(T val = T{})
	 : scalar_unit_base<T>(val)
	{
	}
public:
};

template<typename T>
class velocity_base : public scalar_unit_base<T>
{
protected:
	explicit velocity_base(T val = T{})
	 : scalar_unit_base<T>(val)
	{
	}
public:
};

}

template <typename T>
class millimeters : public detail::length_base<T>
{
public:
	explicit millimeters(T val = T{})
	 : detail::length_base<T>(val)
	{
	}
};

template <typename T>
class millimeters_per_minute : public detail::velocity_base<T>
{
public:
	explicit millimeters_per_minute(T val = T{})
	 : detail::velocity_base<T>(val)
	{
	}
};

}
}

#endif /* UNITS_H_ */
