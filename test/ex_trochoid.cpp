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
	int theta = 2*PI*5;	// 5 = number of spindle revolutions
	int r = 10;			// radius
	
	double cx = 0;		// center point
	double cy = 0;
	double cz = 0;
	
	double distance = 10;	// distance travelled while revolving
	
	// Won't really be x but the distance on the vector of the tool motion
	double xpt = distance / theta;
	double ypt = 0;
	double zpt = 0;
	
	// 4 flutes evenly spaced
	std::vector<double/*theta*/> flute_thetas;
	{
		unsigned int n_flutes = 4;
		auto t = (2*PI)/n_flutes;
		for(unsigned int n = 0; n < n_flutes; ++n)
			flute_thetas.push_back(t*n);
	}
	
	// x = load("points")
	// plot3(x(:, 1), x(:, 2), x(:, 3),x(:, 4), x(:, 5), x(:, 6),x(:, 7), x(:, 8), x(:, 9),x(:, 10), x(:, 11), x(:, 12))
	std::ofstream os("points");
	for(double t = 0; t < theta; t += PI/16)
	{
		int i = 0;
		for(auto ft : flute_thetas)
		{
			ft += t;
			point_3 x = {cx + (r*cos(ft)) + (ft*xpt), 
				         cy + (r*sin(ft)) + (ft*ypt),
				         cz + i           + (ft*zpt)};
		
			os << x << ", ";
			++i;
		}
		os << "\n";
	}

	return 0;
}

