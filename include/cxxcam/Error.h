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
 * Error.h
 *
 *  Created on: 27/05/2013
 *      Author: nicholas
 */

#ifndef ERROR_H_
#define ERROR_H_
#include <stdexcept>

namespace cxxcam
{

class error : public std::runtime_error
{
public:
	error(const std::string& what)
	 : std::runtime_error(what)
	{
	}
};

}

#endif /* ERROR_H_ */
