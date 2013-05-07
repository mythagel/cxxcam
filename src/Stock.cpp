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

Stock Stock::Rectangle(double length, double width, double height)
{
	return {};
}
Stock Stock::Cylinder(double radius, double height)
{
	return {};
}

bool Stock::Write(const std::string& filename, Format format) const
{
	switch(format)
	{
		case Format::OFF:
			return false;
		case Format::NEF:
		{
			std::ofstream out(filename);
			if(!out)
				return false;

			out << m_Nef;
			break;
		}
	}
	return false;
}
