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
 * block.h
 *
 *  Created on: 2013-08-23
 *      Author: nicholas
 */

#ifndef BLOCK_H_
#define BLOCK_H_
#include <cstddef>
#include "maybe.h"

struct block_t
{
	block_t();
    maybe<double>   x;
    maybe<double>   y;
    maybe<double>   z;
    maybe<double>   a;
    maybe<double>   b;
    maybe<double>   c;
    
    char     comment[256];
    maybe<unsigned int>      d;
    maybe<double>   f;
    int      g_modes[14];
    maybe<unsigned int>      h;
    
    maybe<double>   i;
    maybe<double>   j;
    maybe<double>   k;
    
    maybe<unsigned int>      l;
    maybe<unsigned int>      line_number;
    int      motion_to_be;
    unsigned int      m_count;
    int      m_modes[10];
    maybe<double>   p;
    maybe<double>   q;
    maybe<double>   r;
    maybe<double>   s;
    maybe<unsigned int>      t;
    
    
    std::size_t parameter_occurrence;                     // parameter buffer index
    int parameter_numbers[50];                    // parameter number buffer
    double parameter_values[50];                  // parameter value buffer
};

#endif /* BLOCK_H_ */
