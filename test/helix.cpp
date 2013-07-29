#include <iostream>
#include <cmath>
#include <sstream>
#include <string>
#include <iomanip>
#include <tuple>
#include <vector>
#include <iterator>
#include "die_if.h"

static const double PI = 3.14159265358979323846;

struct point_3
{
	double x;
	double y;
	double z;
	
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

enum Direction
{
	Clockwise,
	CounterClockwise
};

enum Plane
{
	XY,
	ZX,
	YZ
};

struct gcode_arc
{
	Direction dir;
	Plane plane;
	
	point_3 start;
	point_3 end;
	
	point_3 center;
	
	unsigned int turns;
};

bool validate_radius(const gcode_arc& arc)
{
	point_3 start = {0, 0, 0};
	point_3 end = {0, 0, 0};
	switch(arc.plane)
	{
		case XY:
			start = point_3{arc.start.x, arc.start.y, 0};
			end = point_3{arc.end.x, arc.end.y, 0};
			break;
		case ZX:
			start = point_3{arc.start.x, 0, arc.start.z};
			end = point_3{arc.end.x, 0, arc.end.z};
			break;
		case YZ:
			start = point_3{0, arc.start.y, arc.start.z};
			end = point_3{0, arc.end.y, arc.end.z};
			break;
	}
	
	double ds = distance(start, arc.center);
	double de = distance(end, arc.center);
	
	if(fabs(ds - de) > 0.0000001)
		return false;
	
	return true;
}

/*
get angle from start to end.
*/
double arc_angle(const gcode_arc& arc)
{
	switch(arc.plane)
	{
		case XY:
			return atan2(arc.end.y - arc.start.y, arc.end.x - arc.start.x);
		case ZX:
			return atan2(arc.end.z - arc.start.z, arc.end.x - arc.start.x);
		case YZ:
			return atan2(arc.end.z - arc.start.z, arc.end.y - arc.start.y);
	}
}

double length(double r, double h, double p)
{
	double c = h / (2*PI);
	
	return (2*PI*p) * sqrt((r*r) + (c*c));
}

/*
r = radius
h = height per turn
p = turns
theta = start angle(radians)
steps_per_rev = effectively resolution of returned points.
*/
std::vector<point_3> helix_points(double r, double h, double p, double theta, double steps_per_rev)
{
	std::vector<point_3> P;
	double c = h / (2*PI);
	
	auto begin = theta;
	auto end = begin+(2*PI*p);
	auto step = (2*PI)/steps_per_rev;
	
	for(double t = begin; t <= end; t += step)
		P.push_back({cos(t)*r, sin(t)*r, t*c});
	return P;
}

/*
normalise arc to centered around 0, 0, 0
*/
void arc_center(const gcode_arc& arc)
{
	point_3 start = {0, 0, 0};
	point_3 end = {0, 0, 0};
	switch(arc.plane)
	{
		case XY:
			start = point_3{arc.start.x - arc.center.x, arc.start.y - arc.center.y, 0};
			end = point_3{arc.end.x - arc.center.x, arc.end.y - arc.center.y, 0};
			break;
		case ZX:
			start = point_3{arc.start.x - arc.center.x, 0, arc.start.z - arc.center.z};
			end = point_3{arc.end.x - arc.center.x, 0, arc.end.z - arc.center.z};
			break;
		case YZ:
			start = point_3{0, arc.start.y - arc.center.y, arc.start.z - arc.center.z};
			end = point_3{0, arc.end.y - arc.center.y, arc.end.z - arc.center.z};
			break;
	}
	std::cout << "start: " << start << " end: " << end << '\n';
}

int main()
{
	gcode_arc simple_xy_arc = {Clockwise, XY, {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, 1};
	gcode_arc simple_yz_arc = {Clockwise, YZ, {0, 0, 0}, {0, 1, 1}, {0, 0, 1}, 1};
	
	gcode_arc simple_xyz_helix = {Clockwise, XY, {0, 0, 0}, {1, 1, 1}, {1, 0, 0}, 1};
	
	std::cout << std::boolalpha << "validate_radius(simple_xy_arc): " << validate_radius(simple_xy_arc) << " arc_angle: " << round6(arc_angle(simple_xy_arc)) << "\n";
	die_if(round6(arc_angle(simple_xy_arc)) != round6(0.785398));	// 45 degrees
	std::cout << std::boolalpha << "validate_radius(simple_yz_arc): " << validate_radius(simple_yz_arc) << " arc_angle: " << round6(arc_angle(simple_yz_arc)) << "\n";
	
	std::cout << std::boolalpha << "validate_radius(simple_xyz_helix): " << validate_radius(simple_xyz_helix) << " arc_angle: " << round6(arc_angle(simple_xyz_helix)) << "\n";
	
	gcode_arc simple_xy_arc_zero = {Clockwise, XY, {-1, 0, 0}, {0, 1, 0}, {0, 0, 0}, 1};
	arc_center(simple_xy_arc_zero);
	arc_center(simple_xy_arc);
	
	// unit circle represented as helix
	{
		double r = 1, h = 0, p = 1, theta = 0;
		std::cout << "helix{" << r << ", " << h << ", " << p << "} (L: " << round6(length(r, h, p)) << "): \n";
		die_if(round6(length(r, h, p)) != round6(2*PI));
		auto points = helix_points(r, h, p, theta, 4);
		std::copy(begin(points), end(points), std::ostream_iterator<point_3>(std::cout, "\n"));
	}

	// unit line represented as helix
	{
		double r = 0, h = 1, p = 1, theta = 0;
		std::cout << "helix{" << r << ", " << h << ", " << p << ", " << theta << "} (L: " << round6(length(r, h, p)) << "): \n";
		die_if(round6(length(r, h, p)) != round6(1));
		auto points = helix_points(r, h, p, theta, 4);
		std::copy(begin(points), end(points), std::ostream_iterator<point_3>(std::cout, "\n"));
	}
	
	// unit helix
	{
		double r = 1, h = 1, p = 1, theta = 0;
		std::cout << "helix{" << r << ", " << h << ", " << p << ", " << theta << "} (L: " << round6(length(r, h, p)) << "): \n";
		auto points = helix_points(r, h, p, theta, 4);
		std::copy(begin(points), end(points), std::ostream_iterator<point_3>(std::cout, "\n"));
	}
	
	return 0;
}

