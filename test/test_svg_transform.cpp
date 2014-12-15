#include "svg_path/transform.h"
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

std::ostream& operator<<(std::ostream& os, const std::array<float, 6>& a)
{
    // TODO enforce number of significant digits.
    os << "[ " << a[0] << " " << a[1] << " " << a[2] << " " << a[3] << " " << a[4] << " " << a[5] << " ]";
    return os;
}

void check_transform(const std::string& trans, const std::string& expected)
{
    auto c = trans.c_str();
    auto end = c + trans.size();
    
    auto m = svg::types::parsers::transform::parse_transforms(c, end);
    
    std::stringstream s;
    s << m;
    
    if(s.str() != expected)
        throw std::logic_error(trans + " Not parsed to '" + expected + "' actual: '" + s.str() + "'");
    std::cout << "'" << trans << "' -> '" << s.str() << "'\n";
}

inline bool throw_if(bool cond, const std::string& what)
{
    if(cond) throw std::logic_error(what);
    return cond;
}

int main()
{
    check_transform("matrix(1 2 3 4 5 6)", "[ 1 2 3 4 5 6 ]");
    check_transform("matrix(0 0 0 0 0 0 )", "[ 0 0 0 0 0 0 ]");
    check_transform("translate(10, 20.1)", "[ 1 0 0 1 10 20.1 ]");
    check_transform("translate(10)", "[ 1 0 0 1 10 0 ]");
    check_transform("scale(10, 20.1)", "[ 10 0 0 20.1 0 0 ]");
    check_transform("scale(10)", "[ 10 0 0 10 0 0 ]");

    check_transform("translate ( 10  20.1 ) ", "[ 1 0 0 1 10 20.1 ]");
    check_transform("translate ( 10 ) ", "[ 1 0 0 1 10 0 ]");
    check_transform("scale ( 10 , 20.1 ) ", "[ 10 0 0 20.1 0 0 ]");
    check_transform("scale ( 10 ) ", "[ 10 0 0 10 0 0 ]");

    check_transform("skewX(10)", "[ 1 0 0.176327 1 0 0 ]");
    check_transform("skewY(10)", "[ 1 0.176327 0 1 0 0 ]");

    check_transform("translate(50, 90)", "[ 1 0 0 1 50 90 ]");
    check_transform("rotate(-45)", "[ 0.707107 -0.707107 0.707107 0.707107 0 0 ]");
    check_transform("translate(130, 160)", "[ 1 0 0 1 130 160 ]");

    check_transform("translate(50, 90), rotate(-45), translate(130, 160)", "[ 0.707107 -0.707107 0.707107 0.707107 255.061 111.213 ]");

    /* Perhaps this has to be handled specially..
       ~ Section 7.6.1 If the list of transforms includes a matrix with all values set to zero (that is, 'matrix(0,0,0,0,0,0)'), then rendering of the element is disabled. Such a value is not an unsupported value. */
//    check_transform("translate(10, 20.1) matrix(0 0 0 0 0 0 )", "[ 0 0 0 0 0 0 ]");

    return 0;
}

