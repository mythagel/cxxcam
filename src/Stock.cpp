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
 * Stock.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "Stock.h"
#include <cassert>
#include <fstream>
#include "nef/io.h"
#include <ostream>

Stock::Stock(const nef::polyhedron_t& nef)
 : Model(nef)
{
}

bool Stock::Write(std::ostream& os, Format format) const
{
	if(!os)
		return false;
				
	switch(format)
	{
		case Format::OFF:
		{
			write_off(os, Model);
			return true;
		}
		case Format::NEF:
		{
			os << Model;
			return true;
		}
	}
	
	return false;
}
