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
 * block.cpp
 *
 *  Created on: 2013-08-23
 *      Author: nicholas
 */

#include "block.h"

block_t::block_t()
{
	comment[0] = 0;
	for (int n = 0; n < 14; n++)
		g_modes[n] = -1;
	motion_to_be = -1;
	m_count = 0;
	for (int n = 0; n < 10; n++)
		m_modes[n] = -1;
	parameter_occurrence = 0;
}

