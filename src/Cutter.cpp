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
 * Cutter.cpp
 *
 *  Created on: 2013-06-27
 *      Author: nicholas
 */

#include "Cutter.h"

namespace cxxcam
{

Cutter::Cutter(double d)
 : d(d), r(0), e(d/2-r), f(r), alpha(0), beta(0), h(r < 5 ? 5 : r)
{
}
Cutter::Cutter(double d, double r)
 : d(d), r(r), e(d/2-r), f(r), alpha(0), beta(0), h(r < 5 ? 5 : r)
{
}

}

