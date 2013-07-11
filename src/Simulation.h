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
 * Simulation.h
 *
 *  Created on: 2013-07-11
 *      Author: nicholas
 */

#ifndef SIMULATION_H_
#define SIMULATION_H_
#include <vector>
#include "Path.h"
#include "Tool.h"
#include "Stock.h"

namespace cxxcam
{
namespace simulation
{

struct simulated_step
{
	path::step step;
};

std::vector<simulated_step> simulate_cut(const std::vector<path::step>& path, Stock& stock, Tool& tool);

/*
 * Check for stock intersection for Linear & Rapid movements
    - What to name this module?
       - Tasks
          - Performs analysis on a specified path.
          - Updates stock model.
          - Possibly updates tool object (wear etc.)
          - Provides information on cutting performance.
    - Volume of material removal
    - Cutting speed
    - Performance
       - Path for each flute is trochoidal. 
       - Calculate the path that flute tip passes through material.
       - Gives simulated chip load per tooth.
       - Compare with data (tables? calculated from Material hardness?) for MRR.

*/

}
}

#endif /* SIMULATION_H_ */
