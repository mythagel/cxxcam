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
 * maybe.h
 *
 *  Created on: 2013-08-23
 *      Author: nicholas
 */

#ifndef MAYBE_H_
#define MAYBE_H_
#include <type_traits>
#include <stdexcept>

template<typename T>
struct maybe
{
	bool valid;
	typename std::enable_if<std::is_trivial<T>::value, T>::type value;

	maybe()
	 : valid(false)
	{
	}
	maybe(T v)
	 : valid(true), value(v)
	{
	}

	explicit operator bool() const
	{
		return valid;
	}

	maybe& operator= (T v)
	{
		valid = true;
		value = v;
		return *this;
	}

	const T& operator*() const
	{
		if(!valid)
			throw std::logic_error("Value not valid.");
		return value;
	}
};

#endif /* MAYBE_H_ */
