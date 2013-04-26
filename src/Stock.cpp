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
