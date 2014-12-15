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
 * transform.cpp
 *
 *  Created on: 2013-12-09
 *      Author: nicholas
 */

#include "transform.h"
#include "basic.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <algorithm>
#include <cmath>

namespace svg
{
namespace types
{
namespace parsers
{
namespace transform
{

static const auto DEG_TO_RAD = 0.0174532925;

namespace ublas = boost::numeric::ublas;

typedef ublas::matrix<float, ublas::row_major, ublas::bounded_array<float, 9>> matrix;
typedef ublas::identity_matrix<float> identity_matrix;

namespace
{

inline bool throw_if(bool cond, const std::string& what)
{
    if(cond) throw parse_error(what);
    return cond;
}

/*
matrix ::=
    "matrix" wsp* "(" wsp*
       number comma-wsp
       number comma-wsp
       number comma-wsp
       number comma-wsp
       number comma-wsp
       number wsp* ")" */
bool parse_matrix(const char*& c, const char* const end, matrix& t)
{
    char tag[] = {'m', 'a', 't', 'r', 'i', 'x'};
    auto it = std::search(c, end, std::begin(tag), std::end(tag));
    if(it != c) return false;
    c += sizeof(tag);
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(*c++ != '(', "expected '('");
    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    matrix m = identity_matrix{3};
    throw_if(!parse_number(c, end, m(0,0)), "expected number");
    throw_if(c == end, "unexpected eof");

    throw_if(!parse_comma_wsp(c, end), "expected comma-wsp");
    throw_if(c == end, "unexpected eof");

    throw_if(!parse_number(c, end, m(1,0)), "expected number");
    throw_if(c == end, "unexpected eof");

    throw_if(!parse_comma_wsp(c, end), "expected comma-wsp");
    throw_if(c == end, "unexpected eof");

    throw_if(!parse_number(c, end, m(0,1)), "expected number");
    throw_if(c == end, "unexpected eof");

    throw_if(!parse_comma_wsp(c, end), "expected comma-wsp");
    throw_if(c == end, "unexpected eof");

    throw_if(!parse_number(c, end, m(1,1)), "expected number");
    throw_if(c == end, "unexpected eof");

    throw_if(!parse_comma_wsp(c, end), "expected comma-wsp");
    throw_if(c == end, "unexpected eof");

    throw_if(!parse_number(c, end, m(0,2)), "expected number");
    throw_if(c == end, "unexpected eof");

    throw_if(!parse_comma_wsp(c, end), "expected comma-wsp");
    throw_if(c == end, "unexpected eof");

    throw_if(!parse_number(c, end, m(1,2)), "expected number");
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(*c++ != ')', "expected ')'");
    
    t = prod(t, m);
    return true;
}

// translate ::= "translate" wsp* "(" wsp* number ( comma-wsp number )? wsp* ")"
bool parse_translate(const char*& c, const char* const end, matrix& t)
{
    char tag[] = {'t', 'r', 'a', 'n', 's', 'l', 'a', 't', 'e'};
    auto it = std::search(c, end, std::begin(tag), std::end(tag));
    if(it != c) return false;
    c += sizeof(tag);
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(*c++ != '(', "expected '('");
    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    matrix m = identity_matrix{3};
    throw_if(!parse_number(c, end, m(0,2)), "expected number");
    throw_if(c == end, "unexpected eof");

    if(parse_comma_wsp(c, end))
    {
        throw_if(c == end, "unexpected eof");
        parse_number(c, end, m(1, 2)) && throw_if(c == end, "unexpected eof");
    }

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(*c++ != ')', "expected ')'");
    
    t = prod(t, m);
    return true;
}

// scale ::= "scale" wsp* "(" wsp* number ( comma-wsp number )? wsp* ")"
bool parse_scale(const char*& c, const char* const end, matrix& t)
{
    char tag[] = {'s', 'c', 'a', 'l', 'e'};
    auto it = std::search(c, end, std::begin(tag), std::end(tag));
    if(it != c) return false;
    c += sizeof(tag);
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(*c++ != '(', "expected '('");
    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    matrix m = identity_matrix{3};
    throw_if(!parse_number(c, end, m(0,0)), "expected number");
    throw_if(c == end, "unexpected eof");

    m(1,1) = m(0,0);
    if(parse_comma_wsp(c, end))
    {
        throw_if(c == end, "unexpected eof");
        parse_number(c, end, m(1,1)) && throw_if(c == end, "unexpected eof");
    }

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(*c++ != ')', "expected ')'");
    
    t = prod(t, m);
    return true;
}

// rotate ::= "rotate" wsp* "(" wsp* number ( comma-wsp number comma-wsp number )? wsp* ")"
bool parse_rotate(const char*& c, const char* const end, matrix& t)
{
    char tag[] = {'r', 'o', 't', 'a', 't', 'e'};
    auto it = std::search(c, end, std::begin(tag), std::end(tag));
    if(it != c) return false;
    c += sizeof(tag);
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(*c++ != '(', "expected '('");
    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    float a;
    throw_if(!parse_number(c, end, a), "expected number");
    throw_if(c == end, "unexpected eof");

    float x = 0;
    float y = 0;
    if(parse_comma_wsp(c, end))
    {
        throw_if(c == end, "unexpected eof");

        if(parse_number(c, end, x))
        {
            throw_if(c == end, "unexpected eof");
            
            throw_if(!parse_comma_wsp(c, end), "expected comma-wsp");
            throw_if(c == end, "unexpected eof");

            throw_if(!parse_number(c, end, y), "expected number");
            throw_if(c == end, "unexpected eof");
        }
    }

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(*c++ != ')', "expected ')'");

    matrix m = identity_matrix{3};
    m(0,0) = std::cos(a*DEG_TO_RAD);
    m(0,1) = -std::sin(a*DEG_TO_RAD);
    m(1,0) = std::sin(a*DEG_TO_RAD);
    m(1,1) = std::cos(a*DEG_TO_RAD);

    // translate(<cx>, <cy>) rotate(<rotate-angle>) translate(-<cx>, -<cy>)
    matrix tr = identity_matrix{3};
    tr(0,2) = x;
    tr(1,2) = y;
    t = prod(t, tr);

    t = prod(t, m);

    tr(0,2) = -x;
    tr(1,2) = -y;

    t = prod(t, tr);
    return true;
}

// skewX ::= "skewX" wsp* "(" wsp* number wsp* ")"
bool parse_skewX(const char*& c, const char* const end, matrix& t)
{
    char tag[] = {'s', 'k', 'e', 'w', 'X'};
    auto it = std::search(c, end, std::begin(tag), std::end(tag));
    if(it != c) return false;
    c += sizeof(tag);
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(*c++ != '(', "expected '('");
    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    matrix m = identity_matrix{3};
    throw_if(!parse_number(c, end, m(0,1)), "expected number");
    throw_if(c == end, "unexpected eof");

    m(0,1) = std::tan(m(0,1)*DEG_TO_RAD);

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(*c++ != ')', "expected ')'");
    
    t = prod(t, m);
    return true;
}

// skewY ::= "skewY" wsp* "(" wsp* number wsp* ")"
bool parse_skewY(const char*& c, const char* const end, matrix& t)
{
    char tag[] = {'s', 'k', 'e', 'w', 'Y'};
    auto it = std::search(c, end, std::begin(tag), std::end(tag));
    if(it != c) return false;
    c += sizeof(tag);
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(*c++ != '(', "expected '('");
    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    matrix m = identity_matrix{3};
    throw_if(!parse_number(c, end, m(1,0)), "expected number");
    throw_if(c == end, "unexpected eof");

    m(1,0) = std::tan(m(1,0)*DEG_TO_RAD);

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(*c++ != ')', "expected ')'");
    
    t = prod(t, m);
    return true;
}

/*
transform ::=
    matrix
    | translate
    | scale
    | rotate
    | skewX
    | skewY */
bool parse_transform(const char*& c, const char* const end, matrix& t)
{
	if( parse_matrix(c, end, t) || 
        parse_translate(c, end, t) ||
        parse_scale(c, end, t) ||
        parse_rotate(c, end, t) ||
        parse_skewX(c, end, t) ||
        parse_skewY(c, end, t) )
    {
        if(c != end)
            parse_comma_wsp(c, end);
        return true;
    }
	return false;
}

}

parse_error::parse_error(const std::string& what)
 : std::runtime_error(what)
{
}
parse_error::~parse_error() noexcept
{
}

/*
transform-list ::= wsp* transforms? wsp*
transforms ::=
    transform
    | transform comma-wsp+ transforms */
std::array<float, 6> parse_transforms(const char* c, const char* const end)
{
    matrix t = identity_matrix{3};

	while(c != end)
	{
		if( parse_whitespace(c, end) || 
            parse_transform(c, end, t) )
			continue;
		else
			throw parse_error{"expected wsp / matrix / translate / scale / rotate / skewX / skewY"};
	}

    return { t(0,0), t(1,0), t(0,1), t(1,1), t(0,2), t(1,2) };
}

}
}
}
}
