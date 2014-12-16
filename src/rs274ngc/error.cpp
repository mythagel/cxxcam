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
 * error.cpp
 *
 *  Created on: 2013-08-27
 *      Author: nicholas
 */

#include "error.h"
#include "rs274ngc_return.hh"

extern const char * _rs274ngc_errors[];

error::error(int code)
{
	if ((code >= RS274NGC_MIN_ERROR) and (code <= RS274NGC_MAX_ERROR))
		err = _rs274ngc_errors[code];
	else
		err = "Unknown error";
}
error::error(const std::string& err)
 : err(err)
{
}
const char* error::what() const noexcept
{
    return err.c_str();
}
error::~error() noexcept
{
}

