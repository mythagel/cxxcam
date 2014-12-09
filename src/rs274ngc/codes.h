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
 * codes.h
 *
 *  Created on: 2013-08-27
 *      Author: nicholas
 */

#ifndef CODES_H_
#define CODES_H_

// G Codes are symbolic to be dialect-independent in source code
enum GCodes
{
	G_0 =       0,
	G_1 =      10,
	G_2 =      20,
	G_3 =      30,
	G_4 =      40,
	G_10 =    100,
	G_17 =    170,
	G_18 =    180,
	G_19 =    190,
	G_20 =    200,
	G_21 =    210,
	G_28 =    280,
	G_30 =    300,
	G_38_2 =  382,
	G_40 =    400,
	G_41 =    410,
	G_42 =    420,
	G_43 =    430,
	G_49 =    490,
	G_53 =    530,
	G_54 =    540,
	G_55 =    550,
	G_56 =    560,
	G_57 =    570,
	G_58 =    580,
	G_59 =    590,
	G_59_1 =  591,
	G_59_2 =  592,
	G_59_3 =  593,
	G_61 =    610,
	G_61_1 =  611,
	G_64 =    640,
	G_80 =    800,
	G_81 =    810,
	G_82 =    820,
	G_83 =    830,
	G_84 =    840,
	G_85 =    850,
	G_86 =    860,
	G_87 =    870,
	G_88 =    880,
	G_89 =    890,
	G_90 =    900,
	G_91 =    910,
	G_92 =    920,
	G_92_1 =  921,
	G_92_2 =  922,
	G_92_3 =  923,
	G_93 =    930,
	G_94 =    940,
	G_98 =    980,
	G_99 =    990
};

#endif /* CODES_H_ */
