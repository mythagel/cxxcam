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
 * ops.cpp
 *
 *  Created on: 2013-08-27
 *      Author: nicholas
 */

#include "ops.h"
#include "error.h"
#include "rs274ngc_return.hh"
#include <cmath>
#include "types.h"

/****************************************************************************/

/* execute binary

Returned value: int
If execute_binary1 or execute_binary2 returns an error code, this
returns that code.
Otherwise, it returns RS274NGC_OK.

Side effects: The value of left is set to the result of applying
the operation to left and right.

Called by: read_real_expression

*/

void execute_binary(double * left, BinaryOperation operation, double * right)
{
    switch (operation)
    {
        case BinaryOperation::DIVIDED_BY:
            error_if(*right == 0.0, NCE_ATTEMPT_TO_DIVIDE_BY_ZERO);
            *left = (*left / *right);
            break;
        case BinaryOperation::MODULO:                          /* always calculates a positive answer */
            *left = fmod(*left, *right);
            if (*left < 0.0)
                *left = (*left + fabs(*right));
            break;
        case BinaryOperation::POWER:
            error_if(((*left < 0.0) and (floor(*right) != *right)), NCE_ATTEMPT_TO_RAISE_NEGATIVE_TO_NON_INTEGER_POWER);
            *left = pow(*left, *right);
            break;
        case BinaryOperation::TIMES:
            *left = (*left * *right);
            break;
        case BinaryOperation::AND2:
            *left = ((*left == 0.0) or (*right == 0.0)) ? 0.0 : 1.0;
            break;
        case BinaryOperation::EXCLUSIVE_OR:
            *left = (((*left == 0.0) and (*right != 0.0)) or
                ((*left != 0.0) and (*right == 0.0))) ? 1.0 : 0.0;
            break;
        case BinaryOperation::MINUS:
            *left = (*left - *right);
            break;
        case BinaryOperation::NON_EXCLUSIVE_OR:
            *left = ((*left != 0.0) or (*right != 0.0)) ? 1.0 : 0.0;
            break;
        case BinaryOperation::PLUS:
            *left = (*left + *right);
            break;
        default:
            throw error(NCE_BUG_UNKNOWN_OPERATION);
    }
}

/****************************************************************************/

/* execute_unary

Returned Value: int
If any of the following errors occur, this returns the error code shown.
Otherwise, it returns RS274NGC_OK.
1. the operation is unknown: NCE_BUG_UNKNOWN_OPERATION
2. the argument to acos is not between minus and plus one:
NCE_ARGUMENT_TO_ACOS_OUT_RANGE
3. the argument to asin is not between minus and plus one:
NCE_ARGUMENT_TO_ASIN_OUT_RANGE
4. the argument to the natural logarithm is not positive:
NCE_ZERO_OR_NEGATIVE_ARGUMENT_TO_LN
5. the argument to square root is negative:
NCE_NEGATIVE_ARGUMENT_TO_SQRT

Side effects:
The result from performing the operation on the value in double_ptr
is put into what double_ptr points at.

Called by: read_unary.

This executes the operations: ABS, ACOS, ASIN, COS, EXP, FIX, FUP, LN
ROUND, SIN, SQRT, TAN

All angle measures in the input or output are in degrees.

*/

void execute_unary(double * double_ptr, UnaryOperation operation)
{
    switch (operation)
    {
        case UnaryOperation::ABS:
            if (*double_ptr < 0.0)
                *double_ptr = (-1.0 * *double_ptr);
            break;
        case UnaryOperation::ACOS:
            error_if(((*double_ptr < -1.0) or (*double_ptr > 1.0)), NCE_ARGUMENT_TO_ACOS_OUT_OF_RANGE);
            *double_ptr = acos(*double_ptr);
            *double_ptr = ((*double_ptr * 180.0)/ PI);
            break;
        case UnaryOperation::ASIN:
            error_if(((*double_ptr < -1.0) or (*double_ptr > 1.0)), NCE_ARGUMENT_TO_ASIN_OUT_OF_RANGE);
            *double_ptr = asin(*double_ptr);
            *double_ptr = ((*double_ptr * 180.0)/ PI);
            break;
        case UnaryOperation::COS:
            *double_ptr = cos((*double_ptr * PI)/180.0);
            break;
        case UnaryOperation::EXP:
            *double_ptr = exp(*double_ptr);
            break;
        case UnaryOperation::FIX:
            *double_ptr = floor(*double_ptr);
            break;
        case UnaryOperation::FUP:
            *double_ptr = ceil(*double_ptr);
            break;
        case UnaryOperation::LN:
            error_if((*double_ptr <= 0.0), NCE_ZERO_OR_NEGATIVE_ARGUMENT_TO_LN);
            *double_ptr = log(*double_ptr);
            break;
        case UnaryOperation::ROUND:
            *double_ptr = (double)
                ((int) (*double_ptr + ((*double_ptr < 0.0) ? -0.5 : 0.5)));
            break;
        case UnaryOperation::SIN:
            *double_ptr = sin((*double_ptr * PI)/180.0);
            break;
        case UnaryOperation::SQRT:
            error_if((*double_ptr < 0.0), NCE_NEGATIVE_ARGUMENT_TO_SQRT);
            *double_ptr = sqrt(*double_ptr);
            break;
        case UnaryOperation::TAN:
            *double_ptr = tan((*double_ptr * PI)/180.0);
            break;
        default:
            throw error(NCE_BUG_UNKNOWN_OPERATION);
    }
}

/****************************************************************************/

/* precedence

Returned Value: int
This returns an integer representing the precedence level of an_operator

Side Effects: None

Called by: read_real_expression

To add additional levels of operator precedence, edit this function.

*/

int precedence(BinaryOperation an_operator)
{
    if (an_operator == BinaryOperation::RIGHT_BRACKET)
        return 1;
    else if (an_operator == BinaryOperation::POWER)
        return 4;
    else if (an_operator >= BinaryOperation::AND2)
        return 2;
    else
        return 3;
}

