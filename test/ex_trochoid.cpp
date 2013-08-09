#include <iostream>
#include <cmath>
#include <sstream>
#include <string>
#include <iomanip>
#include <tuple>
#include <vector>
#include <iterator>
#include <fstream>
#include "die_if.h"
#include <stdexcept>

static const double PI = 3.14159265358979323846;

struct point_3
{
	double x;
	double y;
	double z;
	
	point_3()
	 : x(), y(), z()
	{
	}
	point_3(double x, double y, double z)
	 : x(x), y(y), z(z)
	{
	}
	
	bool operator==(const point_3& o) const
	{
		return std::tie(x, y, z) == std::tie(o.x, o.y, o.z);
	}
};

auto round6(double n) -> std::string
{
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6) << n;
	auto s = ss.str();

	s.erase(s.find_last_not_of('0') + 1, std::string::npos);
	if(s.back() == '.')
		s.pop_back();
	return s;
};

std::ostream& operator<<(std::ostream& os, const point_3& p)
{
	os << round6(p.x) << ", " << round6(p.y) << ", " << round6(p.z);
	return os;
}

double distance(const point_3& p0, const point_3& p1)
{
	return sqrt((p0.x-p1.x)*(p0.x-p1.x) + (p0.y-p1.y)*(p0.y-p1.y) + (p0.z-p1.z)*(p0.z-p1.z));
}

int main()
{
	int r = 1;
	double cx = 0;
	double cy = 0;
	for(double t = 0; t < 2*PI; t += PI/4)
	{
		double x = cx + r * cos(t);
		double y = cy + r * sin(t);
		
		std::cout << "(" << round6(x) << ", " << round6(y) << ")\n";
	}

	return 0;
}

