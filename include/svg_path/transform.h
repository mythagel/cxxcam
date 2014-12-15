/* 
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
 * transform.h
 *
 *  Created on: 2013-12-09
 *      Author: nicholas
 */

#ifndef PARSERS_TRANSFORM_H_
#define PARSERS_TRANSFORM_H_
#include <array>
#include <stdexcept>

namespace svg
{
namespace types
{
namespace parsers
{
namespace transform
{

struct parse_error : std::runtime_error
{
    parse_error(const std::string& what);
    virtual ~parse_error() noexcept;
};

/*
Parser that reduces all transformation lists down to a matrix

[a b c d e f] ->

[a c e]
[b d f]
[0 0 1]
*/
std::array<float, 6> parse_transforms(const char* c, const char* const end);

}
}
}
}

#endif /* PARSERS_TRANSFORM_H_ */
