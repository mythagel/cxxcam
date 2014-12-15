#include "svg_path/path.h"
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

struct normalise_path : svg::types::parsers::path::parser
{
    std::stringstream s;
    
	void move_to(bool abs, float x, float y) override
	{
	    s << (abs?"M" : "m") << " " << x << " " << y << " ";
	}

	void line_to(bool abs, float x, float y) override
	{
    	s << (abs?"L" : "l") << " " << x << " " << y << " ";
	}
	void horizontal_line_to(bool abs, float x) override
	{
	    s << (abs?"H" : "h") << " " << x << " ";
	}
	void vertical_line_to(bool abs, float y) override
	{
    	s << (abs?"V" : "v") << " " << y << " ";
	}

	void curve_to(bool abs, float x1, float y1, float x2, float y2, float x, float y) override
	{
    	s << (abs?"C" : "c") << " " << x1 << " " << y1 << " " << x2 << " " << y2 << " " << x << " " << y << " ";
	}
	void smooth_curve_to(bool abs, float x2, float y2, float x, float y) override
	{
    	s << (abs?"S" : "s") << " " << x2 << " " << y2 << " " << x << " " << y << " ";
	}

	void bezier_curve_to(bool abs, float x1, float y1, float x, float y) override
	{
    	s << (abs?"Q" : "q") << " " << x1 << " " << y1 << " " << x << " " << y << " ";
	}
	void smooth_bezier_curve_to(bool abs, float x, float y) override
	{
    	s << (abs?"T" : "t") << " " << x << " " << y << " ";
	}

	void close_path() override
	{
	    s << "Z" << " ";
	}
	void eof() override
	{
	}
};

void check_path(const std::string& path, const std::string& expected)
{
    normalise_path parser;
    auto c = path.c_str();
    auto end = c + path.size();
    
    parser.parse(c, end);
    
    if(parser.s.str() != expected)
        throw std::logic_error(path + " Not parsed to '" + expected + "' actual: '" + parser.s.str() + "'");
    std::cout << "'" << path << "' -> '" << parser.s.str() << "'\n";
}

inline bool throw_if(bool cond, const std::string& what)
{
    if(cond) throw std::logic_error(what);
    return cond;
}

int main()
{
    check_path("M 100 100 L 200 200", "M 100 100 L 200 200 ");
    check_path("M100 100L200 200", "M 100 100 L 200 200 ");
    check_path("M100 100 200 200", "M 100 100 L 200 200 ");
    
    check_path("M 100 200 L 200 100 L -100 -200", "M 100 200 L 200 100 L -100 -200 ");
    check_path("M 100 200 L 200 100 -100 -200", "M 100 200 L 200 100 L -100 -200 ");
    check_path("M 100-200", "M 100 -200 ");
    check_path("M 0.6.5", "M 0.6 0.5 ");

    check_path("M100,100L200,200", "M 100 100 L 200 200 ");
    check_path("M100,100L200,200", "M 100 100 L 200 200 ");
    check_path("M100,100,200,200", "M 100 100 L 200 200 ");
    
    check_path("M100,200L200,100L-100,-200", "M 100 200 L 200 100 L -100 -200 ");
    check_path("M100,200L200,100,-100,-200", "M 100 200 L 200 100 L -100 -200 ");
    check_path("M100,-200", "M 100 -200 ");
    check_path("M0.6,.5", "M 0.6 0.5 ");

    return 0;
}

