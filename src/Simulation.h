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
#include "Path.h"

namespace cxxcam
{
namespace simulation
{

/*
 * Check for stock intersection for Linear & Rapid movements
    - What to name this module?
       - Tasks
          - Performs analysis on a specified path.
          - Updates stock model.
          - Possibly updates tool object (wear etc.)
          - Provides information on cutting performance.
       - Ideas
          - Simulation
          - Analysis
    - Take path, expand tool along path with rotation, subtract from stock.
    - Path is discretised, perform analysis at each step.
    - Analysis must be possible on multiple linear / angular segments
       - Non-plane aligned arcs / higher order curves will be represented as a collection of line segments.
       - Does the boundary between motions need to be preserved?
    - Volume of material removal
    - Cutting speed
    - Performance
       - Path for each flute is trochoidal. 
       - Calculate the path that flute tip passes through material.
       - Gives simulated chip load per tooth.
       - Compare with data (tables? calculated from Material hardness?) for MRR.
    - Interface for feedback?
       - Multiple moves must be able to be coalasced into one for analysis.
       - I.e. use stack push and pop of state to test different configurations to find optimal.
       - Return vector of stats for each step
          - volume, forces, engagement, etc.
          - New functions that augments existing path provided.

*/

}
}

#endif /* SIMULATION_H_ */
