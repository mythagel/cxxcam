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
 * error.h
 *
 *  Created on: 2013-08-27
 *      Author: nicholas
 */

#ifndef ERROR_H_
#define ERROR_H_
#include <exception>
#include <string>

struct error : std::exception
{
	std::string err;
	
	explicit error(int code);
	explicit error(const std::string& err);
	virtual const char* what() const noexcept;
	virtual ~error() noexcept;
};

template <typename T>
void error_if(bool bad, T err)
{
	if(bad) throw error(err);
}

#endif /* ERROR_H_ */
