/*
 * Stock.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "Stock.h"
#include <cassert>
#include <fstream>

Stock::Stock()
{
}

Stock Stock::Rectangle(double length, double width, double height)
{
	Stock stock;
	return stock;
}
Stock Stock::Cylinder(double radius, double height)
{
	return Stock();
}

bool Stock::Write(const std::string& filename, Format format) const
{
	switch(format)
	{
		case format_OFF:
		{
//			std::ofstream out(filename.c_str());
//			if(!out)
//				return false;
//
//			Nef_polyhedron_3 Model = m_Private->Model;
//			assert(Model.is_simple());
//
//			Exact_Polyhedron_3 Polyhedron;
//			Model.convert_to_polyhedron(Polyhedron);
//			out << Polyhedron;
			return true;
			break;
		}
	}
	return false;
}

Stock::~Stock()
{
}

