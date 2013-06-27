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
 * Cutter.h
 *
 *  Created on: 2013-06-27
 *      Author: nicholas
 */

#ifndef CUTTER_H_
#define CUTTER_H_

namespace cxxcam
{

class Cutter
{
private:
	double d;
	double r;
	double e;
	double f;
	double alpha;
	double beta;
	double h;
public:
	Cutter(double d);
	Cutter(double d, double r);
};

}

#endif /* CUTTER_H_ */
