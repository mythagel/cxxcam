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
 * Stock.h
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#ifndef STOCK_H_
#define STOCK_H_
#include <string>
#include <memory>
#include "Material.h"
#include "nef/polyhedron.h"

/*
 * Stores a description and model of the stock from which material will be removed.
 *
 * No virtual classes, stock will be a single object which can be initialised with multiple
 * physical shapes:
 * Rectangle
 * Cylinder
 * ... (others added on demand)
 *
 * Also should reference properties on the material the stock is made out of.
 *
 * Has the ability to write the stock model out so it can be inspected.
 * File formats to be defined later.
 */
class Stock
{
private:
	std::shared_ptr<Material_t> Material;
	nef::polyhedron_t m_Nef;
public:
	enum class Format
	{
		NEF,
		OFF
	};
public:
	static Stock Rectangle(double length, double width, double height);
	static Stock Cylinder(double radius, double height);

	bool Write(const std::string& filename, Format format = Format::NEF) const;
};

#endif /* STOCK_H_ */
