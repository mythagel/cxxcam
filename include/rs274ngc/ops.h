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
 * ops.h
 *
 *  Created on: 2013-08-27
 *      Author: nicholas
 */

#ifndef OPS_H_
#define OPS_H_

// unary operations
enum class UnaryOperation
{
	ABS = 1,
	ACOS = 2,
	ASIN = 3,
	ATAN = 4,
	COS = 5,
	EXP = 6,
	FIX = 7,
	FUP = 8,
	LN = 9,
	ROUND = 10,
	SIN = 11,
	SQRT = 12,
	TAN = 13
};

// binary operations
enum class BinaryOperation
{
	DIVIDED_BY = 1,
	MODULO = 2,
	POWER = 3,
	TIMES = 4,
	AND2 = 5,
	EXCLUSIVE_OR = 6,
	MINUS = 7,
	NON_EXCLUSIVE_OR = 8,
	PLUS = 9,
	RIGHT_BRACKET = 10
};

void execute_binary(double* left, BinaryOperation operation, double* right);
void execute_unary(double* double_ptr, UnaryOperation operation);

int precedence(BinaryOperation an_operator);


#endif /* OPS_H_ */
