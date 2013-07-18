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
#include "Units.h"
#include "Limits.h"

namespace cxxcam
{
namespace simulation
{

/*
 * Need a better solution for storing simulation results.
 * cxxcam will run simulation for each input step.
 * jscam will provide ability to analyse cutting performance (and use this as feedback for generation)
 * Tool changes change meaning of simulation results. But because of volume of data it is unwise to store
 * tool objects for each step.
 * Tool id is not stable enough.
 * Seems acceptable to push this responsibility onto caller.
 * simulation results are stored - tool etc are not.
 */
struct step
{
	path::step s0;
	path::step s1;
	
	units::volume swarf;
};

struct state
{
	Stock stock;
	Tool tool;
	double FeedRate;	// TODO normalised.
	unsigned long SpindleSpeed;	// RPM
	limits::FeedRate FeedRateLimit;
};

/*
 * Simulate a single time step
 * Simulation of a path will involve multiple calls to this function to analyse each step.
 * standard algorithms + lambda used to fold path sequence.
 */
step simulate_cut(const path::step& s0, const path::step& s1, state& s);

}
}

#endif /* SIMULATION_H_ */
