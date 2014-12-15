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
 * path.cpp
 *
 *  Created on: 2013-12-04
 *      Author: nicholas
 */

#include "path.h"
#include "basic.h"
#include <cctype>
#include <cerrno>
#include <sstream>

namespace svg
{
namespace types
{
namespace parsers
{
namespace path
{

namespace
{

struct point
{
    float x;
    float y;
};

inline bool throw_if(bool cond, const std::string& what)
{
    if(cond) throw parser::error(what);
    return cond;
}

bool parse_coordinate_pair(const char*& c, const char* const end, point& p)
{
    if(!parse_number(c, end, p.x))
        return false;

    parse_comma_wsp(c, end) && throw_if(c == end, "unexpected eof");

    throw_if(!parse_number(c, end, p.y), "expected coordinate-pair");
    return true;
}

}

bool parser::parse_moveto(const char*& c, const char* const end)
{
    if(*c != 'M' && *c != 'm')
        return false;

    const auto cmd = *c++;
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    point p;
    throw_if(!parse_coordinate_pair(c, end, p), "expected coordinate-pair");
    move_to(cmd == 'M', p.x, p.y);

    if(c == end)
        return true;

    /* This is not as strict as the formal grammer.
     * This code will allow a comma to terminate the coordinate sequence
     * where the lineto-argument-sequence production would require another
     * coordinate. */
    parse_comma_wsp(c, end);
    while(c != end && parse_coordinate_pair(c, end, p))
    {
        line_to(cmd == 'M', p.x, p.y);
        parse_comma_wsp(c, end);
    }

    return true;
}
bool parser::parse_lineto(const char*& c, const char* const end)
{
    if(*c != 'L' && *c != 'l')
        return false;

    const auto cmd = *c++;
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    point p;
    throw_if(!parse_coordinate_pair(c, end, p), "expected coordinate-pair");
    line_to(cmd == 'L', p.x, p.y);

    if(c == end)
        return true;

    /* This is not as strict as the formal grammer.
     * This code will allow a comma to terminate the coordinate sequence
     * where the lineto-argument-sequence production would require another
     * coordinate. */
    parse_comma_wsp(c, end);
    while(c != end && parse_coordinate_pair(c, end, p))
    {
        line_to(cmd == 'L', p.x, p.y);
        parse_comma_wsp(c, end);
    }

    return true;
}
bool parser::parse_horizontal_lineto(const char*& c, const char* const end)
{
    if(*c != 'H' && *c != 'h')
        return false;

    const auto cmd = *c++;
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    float x;
    throw_if(!parse_number(c, end, x), "expected coordinate");
    horizontal_line_to(cmd == 'H', x);

    if(c == end)
        return true;

    parse_comma_wsp(c, end);
    while(c != end && parse_number(c, end, x))
    {
        horizontal_line_to(cmd == 'H', x);
        parse_comma_wsp(c, end);
    }

    return true;
}
bool parser::parse_vertical_lineto(const char*& c, const char* const end)
{
    if(*c != 'V' && *c != 'v')
        return false;

    const auto cmd = *c++;
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    float y;
    throw_if(!parse_number(c, end, y), "expected coordinate");
    vertical_line_to(cmd == 'V', y);

    if(c == end)
        return true;

    parse_comma_wsp(c, end);
    while(c != end && parse_number(c, end, y))
    {
        vertical_line_to(cmd == 'V', y);
        parse_comma_wsp(c, end);
    }

    return true;
}
bool parser::parse_curveto(const char*& c, const char* const end)
{
    if(*c != 'C' && *c != 'c')
        return false;

    const auto cmd = *c++;
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    point p1;
    point p2;
    point p;
    throw_if(!parse_coordinate_pair(c, end, p1), "expected coordinate-pair p1");
    throw_if(c == end, "unexpected eof");
    parse_comma_wsp(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(!parse_coordinate_pair(c, end, p2), "expected coordinate-pair p2");
    throw_if(c == end, "unexpected eof");
    parse_comma_wsp(c, end) && throw_if(c == end, "unexpected eof");
    throw_if(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");
    curve_to(cmd == 'C', p1.x, p1.y, p2.x, p2.y, p.x, p.y);

    if(c == end)
        return true;

    parse_comma_wsp(c, end);
    while(c != end && parse_coordinate_pair(c, end, p1))
    {
        throw_if(c == end, "unexpected eof");
        parse_comma_wsp(c, end) && throw_if(c == end, "unexpected eof");
        throw_if(!parse_coordinate_pair(c, end, p2), "expected coordinate-pair p2");

        throw_if(c == end, "unexpected eof");
        parse_comma_wsp(c, end) && throw_if(c == end, "unexpected eof");
        throw_if(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");

        curve_to(cmd == 'C', p1.x, p1.y, p2.x, p2.y, p.x, p.y);
        parse_comma_wsp(c, end);
    }

    return true;
}
bool parser::parse_smooth_curveto(const char*& c, const char* const end)
{
    if(*c != 'S' && *c != 's')
        return false;

    const auto cmd = *c++;
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    point p2;
    point p;
    throw_if(!parse_coordinate_pair(c, end, p2), "expected coordinate-pair p2");
    throw_if(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");
    smooth_curve_to(cmd == 'S', p2.x, p2.y, p.x, p.y);

    if(c == end)
        return true;

    parse_comma_wsp(c, end);
    while(c != end && parse_coordinate_pair(c, end, p2))
    {
        throw_if(c == end, "unexpected eof");
        parse_comma_wsp(c, end) && throw_if(c == end, "unexpected eof");
        throw_if(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");

        smooth_curve_to(cmd == 'S', p2.x, p2.y, p.x, p.y);
        parse_comma_wsp(c, end);
    }

    return true;
}
bool parser::parse_quadratic_bezier_curveto(const char*& c, const char* const end)
{
    if(*c != 'Q' && *c != 'q')
        return false;

    const auto cmd = *c++;
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    point p1;
    point p;
    throw_if(!parse_coordinate_pair(c, end, p1), "expected coordinate-pair p1");
    throw_if(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");
    bezier_curve_to(cmd == 'Q', p1.x, p1.y, p.x, p.y);

    if(c == end)
        return true;

    parse_comma_wsp(c, end);
    while(c != end && parse_coordinate_pair(c, end, p1))
    {
        throw_if(c == end, "unexpected eof");
        parse_comma_wsp(c, end) && throw_if(c == end, "unexpected eof");
        throw_if(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");

        bezier_curve_to(cmd == 'Q', p1.x, p1.y, p.x, p.y);
        parse_comma_wsp(c, end);
    }

    return true;
}
bool parser::parse_smooth_quadratic_bezier_curveto(const char*& c, const char* const end)
{
    if(*c != 'T' && *c != 't')
        return false;

    const auto cmd = *c++;
    throw_if(c == end, "unexpected eof");

    parse_whitespace(c, end) && throw_if(c == end, "unexpected eof");

    point p;
    throw_if(!parse_coordinate_pair(c, end, p), "expected coordinate-pair");
    smooth_bezier_curve_to(cmd == 'T', p.x, p.y);

    if(c == end)
        return true;

    parse_comma_wsp(c, end);
    while(c != end && parse_coordinate_pair(c, end, p))
    {
        smooth_bezier_curve_to(cmd == 'T', p.x, p.y);
        parse_comma_wsp(c, end);
    }

    return true;
}
bool parser::parse_closepath(const char*& c, const char* const)
{
    if(*c != 'Z' && *c != 'z')
        return false;
    ++c;
    close_path();
    return true;
}

parser::error::error(const std::string& what)
 : std::runtime_error(what)
{
}
parser::error::~error() noexcept
{
}

void parser::parse(const char* c, const char* const end)
{
	while(c != end)
	{
		if( parse_whitespace(c, end) || 
            parse_moveto(c, end) ||
            parse_lineto(c, end) || 
            parse_horizontal_lineto(c, end) || 
            parse_vertical_lineto(c, end) || 
            parse_curveto(c, end) || 
            parse_smooth_curveto(c, end) || 
            parse_quadratic_bezier_curveto(c, end) || 
            parse_smooth_quadratic_bezier_curveto(c, end) || 
            parse_closepath(c, end) )
			continue;
		else
			throw error{"expected wsp / moveto / lineto / horizontal-lineto / vertical-lineto / curveto / smooth-curveto / quadratic-bezier-curveto / smooth-quadratic-bezier-curveto / closepath"};
	}
	eof();
}

parser::~parser()
{
}

}
}
}
}
