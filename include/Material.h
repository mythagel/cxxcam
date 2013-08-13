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
 * Material.h
 *
 *  Created on: 28/04/2012
 *      Author: nicholas
 */

#ifndef MATERIAL_H_
#define MATERIAL_H_
#include <string>
#include <map>

namespace cxxcam
{
namespace material
{

/*
 * A representation of the material the stock is made out of.
 *
 * Stores properties on the material, that in conjunction with the tool,
 * machine and cutter engagement will be used to calculate the optimum
 * speeds and feeds
 *

From Wikipedia:

Steel (tough)										15–18
Mild steel											30–38
Cast iron (medium)									18–24
Alloy steels (1320–9262)							20-37
Carbon steels (C1008-C1095)							21-40
Free cutting steels (B1111-B1113 & C1108-C1213)		35-69
Stainless steels (300 & 400 series)					23-40
Bronzes												24–45
Leaded steel (Leadloy 12L14)						91
Aluminium											75–105
Brass												90-210
 */
struct Material
{
	std::string name;
	std::string grade;
	
	template <typename T>
	struct range_t
	{
		T low;
		T high;
		
		range_t()
		 : low(), high()
		{
		}
		explicit range_t(T v)
		 : low(v), high(v)
		{
		}
		range_t(T low, T high)
		 : low(low), high(high)
		{
		}
	};
	
	range_t<double> hardness;
	range_t<double> machinability;
	
	range_t<double> mmpm_hss;
	range_t<double> mmpm_carbide;
};

class MaterialTable
{
private:
	std::map<std::string, Material> table;
public:
	
};

}
}

#endif /* MATERIAL_H_ */
