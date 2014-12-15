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
 * numberlist.h
 *
 *  Created on: 2013-12-18
 *      Author: nicholas
 */

#ifndef PARSERS_NUMBERLIST_H_
#define PARSERS_NUMBERLIST_H_
#include "basic.h"
#include <array>
#include <stdexcept>

namespace svg
{
namespace types
{
namespace parsers
{

namespace detail
{
inline bool throw_if(bool cond, const std::string& what)
{
    if(cond) throw std::runtime_error(what);
    return cond;
}
}

template <size_t N>
std::array<float, N> parse_numberlist(const char* c, const char* const end)
{
    using namespace detail;
    std::array<float, N> list;
    for(size_t i = 0; i < N; ++i)
    {
        throw_if(!parse_number(c, end, list[i]), "expected number");

        if(i != N-1)
        {
            throw_if(c == end, "unexpected eof");
            throw_if(!parse_comma_wsp(c, end), "expected comma-wsp");
            throw_if(c == end, "unexpected eof");
        }
    }
    return list;
}

}
}
}

#endif /* PARSERS_NUMBERLIST_H_ */
