#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "svg_path/numberlist.h"

std::ostream& operator<<(std::ostream& os, const std::array<float, 4>& a)
{
    // TODO enforce number of significant digits.
    os << "[ " << a[0] << " " << a[1] << " " << a[2] << " " << a[3] << " ]";
    return os;
}

void check_parse(const std::string& nl, const std::string& expected)
{
    auto c = nl.c_str();
    auto end = c + nl.size();

    auto l = svg::types::parsers::parse_numberlist<4>(c, end);

    std::stringstream s;
    s << l;

    if(s.str() != expected)
        throw std::logic_error(nl + " Not parsed to '" + expected + "' actual: '" + s.str() + "'");
    std::cout << "'" << nl << "' -> '" << s.str() << "'\n";
}

int main()
{
    check_parse("1, 2, 3, 4", "[ 1 2 3 4 ]");
    check_parse("1 , 2 , 3 , 4", "[ 1 2 3 4 ]");
    check_parse("1 2 3 4", "[ 1 2 3 4 ]");
    check_parse("1,2,3,4", "[ 1 2 3 4 ]");

    check_parse("1,2,3,4,5,6,7,8", "[ 1 2 3 4 ]");

    return 0;
}
