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
#include "nef_polyhedron.h"

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
	nef_polyhedron_t m_Nef;
public:
	enum Format
	{
		format_OFF
	};
public:
	Stock();

	static Stock Rectangle(double length, double width, double height);
	static Stock Cylinder(double radius, double height);

	bool Write(const std::string& filename, Format format) const;

	~Stock();
};

#endif /* STOCK_H_ */
