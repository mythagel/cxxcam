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

std::ostream& operator<<(std::ostream& os, const gcode_arc& arc)
{
	os << "Direction: ";
	switch(arc.dir)
	{
		case CounterClockwise:
			os << "Counter";
		case Clockwise:
			os << "Clockwise";
	}
	os << '\n';
	os << "Plane    : ";
	switch(arc.plane)
	{
		case XY:
			os << "XY\n";
			break;
		case ZX:
			os << "ZX\n";
			break;
		case YZ:
			os << "YZ\n";
			break;
	}
	os << "Start    : " << arc.start << "\n";
	os << "End      : " << arc.end << "\n";
	os << "Center   : " << arc.center << "\n";
	os << "Turns    : " << arc.turns << "\n";
	return os;
}

/*
r = radius
h = height per turn
p = turns
theta = start angle(radians)
steps_per_rev = effectively resolution of returned points.
*/
std::vector<point_3> helix_points(double r, double h, double p, double theta, const point_3& c, double steps_per_rev)
{
	std::vector<point_3> P;
	double ch = h / (2*PI);
	
	auto begin = theta;
	auto end = begin+(2*PI*p);
	auto step = (2*PI)/steps_per_rev;
	
	for(double t = begin; t <= end; t += step)
		P.push_back({ (cos(t)*r)+c.x, (sin(t)*r)+c.y, (t*ch) + c.z});
	return P;
}

double helix_length(double r, double h, double p)
{
	double c = h / (2*PI);
	
	return (2*PI*p) * sqrt((r*r) + (c*c));
}

void arc_test(const gcode_arc& arc)
{
	std::cout << arc << "\n";
	// Steps:
	// 0. Determine center point & radius
	// 1. Determine theta to start from center and start point.
	// 2. Determine theta to end from center and end point.
	// 3. Determine revolutions by determining number of radians total possible revolutions (2PI*turns).
	// 4. Subtract end theta from above.
	// 5. Determine step size (theta from step distance on circumference)
	
	auto equidistant = [](const point_3& p0, const point_3& p1, const point_3& ref) -> bool
	{
		double d0 = distance(p0, ref);
		double d1 = distance(p1, ref);

		if(fabs(d0 - d1) > 0.0000001)
			return false;
		return true;
	};
	
	switch(arc.plane)
	{
		case XY:
		{
			auto start = point_3{arc.start.x, arc.start.y, 0};
			auto end = point_3{arc.end.x, arc.end.y, 0};
			auto helix = arc.end.z - arc.start.z;
			if(equidistant(start, end, arc.center))
			{
				double r = distance(start, arc.center);
				double start_theta = atan2(start.y - arc.center.y, start.x - arc.center.x);
				double end_theta = atan2(end.y - arc.center.y, end.x - arc.center.x);
				double turn_theta = 2*PI*arc.turns;
				turn_theta -= end_theta;
				double l = helix_length(r, helix/arc.turns, turn_theta);
				
				std::cout << "Plane: XY\n";
				std::cout << "r    : " << r << "\n";
				std::cout << "hx   : " << helix << "\n";
				std::cout << "t0   : " << start_theta << "\n";
				std::cout << "t1   : " << end_theta << "\n";
				std::cout << "tn   : " << turn_theta << "\n";
				std::cout << "l    : " << l << "\n";

				double ch = helix / (turn_theta - start_theta);
				std::vector<point_3> P;
				for(double t = start_theta; t < turn_theta; t += 0.1)
					P.push_back({ (cos(t)*r)+arc.center.x, -(sin(t)*r)+arc.center.y, ((t - start_theta)*ch) + arc.start.z });
				P.push_back(arc.end);
				std::copy(P.begin(), P.end(), std::ostream_iterator<point_3>(std::cout, "\n"));
			}
			else
			{
				throw std::runtime_error("XY arc center not equidistant from start and end points.");
			}
			break;
		}
		case ZX:
		{
			auto start = point_3{arc.start.x, 0, arc.start.z};
			auto end = point_3{arc.end.x, 0, arc.end.z};
			auto helix = arc.end.y - arc.start.y;
			if(equidistant(start, end, arc.center))
			{
				double r = distance(start, arc.center);
				double start_theta = atan2(start.z - arc.center.z, start.x - arc.center.x);
				double end_theta = atan2(end.z - arc.center.z, end.x - arc.center.x);
				double turn_theta = 2*PI*arc.turns;
				turn_theta -= end_theta;
				double l = helix_length(r, helix/arc.turns, turn_theta);
				
				std::cout << "Plane: ZX\n";
				std::cout << "r    : " << r << "\n";
				std::cout << "hx   : " << helix << "\n";
				std::cout << "t0   : " << start_theta << "\n";
				std::cout << "t1   : " << end_theta << "\n";
				std::cout << "tn   : " << turn_theta << "\n";
				std::cout << "l    : " << l << "\n";
				
				double ch = helix / (turn_theta - start_theta);
				std::vector<point_3> P;
				for(double t = start_theta; t < turn_theta; t += 0.1)
					P.push_back({ (cos(t)*r)+arc.center.x, ((t - start_theta)*ch) + arc.start.y, -(sin(t)*r)+arc.center.z});
				P.push_back(arc.end);
				std::copy(P.begin(), P.end(), std::ostream_iterator<point_3>(std::cout, "\n"));
			}
			else
			{
				throw std::runtime_error("ZX arc center not equidistant from start and end points.");
			}
			break;
		}
		case YZ:
		{
			auto start = point_3{0, arc.start.y, arc.start.z};
			auto end = point_3{0, arc.end.y, arc.end.z};
			auto helix = arc.end.x - arc.start.x;
			if(equidistant(start, end, arc.center))
			{
				double r = distance(start, arc.center);
				double start_theta = atan2(start.y - arc.center.y, start.z - arc.center.z);
				double end_theta = atan2(end.y - arc.center.y, end.z - arc.center.z);
				double turn_theta = 2*PI*arc.turns;
				turn_theta -= end_theta;
				double l = helix_length(r, helix/arc.turns, turn_theta);
				
				std::cout << "Plane: YZ\n";
				std::cout << "r    : " << r << "\n";
				std::cout << "hx   : " << helix << "\n";
				std::cout << "t0   : " << start_theta << "\n";
				std::cout << "t1   : " << end_theta << "\n";
				std::cout << "tn   : " << turn_theta << "\n";
				std::cout << "l    : " << l << "\n";
				
				double ch = helix / (turn_theta - start_theta);
				std::vector<point_3> P;
				for(double t = start_theta; t < turn_theta; t += 0.1)
					P.push_back({ ((t - start_theta)*ch) + arc.start.x, -(sin(t)*r)+arc.center.y, (cos(t)*r)+arc.center.z});
				P.push_back(arc.end);
				std::copy(P.begin(), P.end(), std::ostream_iterator<point_3>(std::cout, "\n"));
			}
			else
			{
				throw std::runtime_error("YZ arc center not equidistant from start and end points.");
			}
			break;
		}
	}
	std::cout << "\n" << std::string(50, '=') << "\n\n";
}

int main()
{
	gcode_arc simple_xy_arc = {Clockwise, XY, {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, 1};
	gcode_arc simple_xy_arc_ccw = {CounterClockwise, XY, {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, 1};
	gcode_arc simple_zx_arc = {Clockwise, ZX, {0, 0, 0}, {1, 0, 1}, {1, 0, 0}, 1};
	gcode_arc simple_yz_arc = {Clockwise, YZ, {0, 0, 0}, {0, 1, 1}, {0, 0, 1}, 1};
	
	gcode_arc simple_xyz_helix = {Clockwise, XY, {0, 0, 0}, {1, 1, 1}, {1, 0, 0}, 1};
	
	gcode_arc xy_circle = {Clockwise, XY, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}, 1};
	gcode_arc xyz_helix = {Clockwise, XY, {1, 0, 0}, {1, 0, 10}, {0, 0, 0}, 2};
	
	arc_test(simple_xy_arc);
	arc_test(simple_xy_arc_ccw);
	arc_test(simple_zx_arc);
	arc_test(simple_yz_arc);
	arc_test(simple_xyz_helix);
	arc_test(xy_circle);
	arc_test(xyz_helix);
	
	std::cout << "\n";
	// unit circle represented as helix
	{
		double r = 1, h = 0, p = 1, theta = 0;
		std::cout << "helix{" << r << ", " << h << ", " << p << "} (L: " << round6(helix_length(r, h, p)) << "): \n";
		die_if(round6(helix_length(r, h, p)) != round6(2*PI));
		auto points = helix_points(r, h, p, theta, {}, 4);
		std::cout << "start: " << points.front() << " end: " << points.back() << "\n";
	}

	std::cout << "\n";

	// unit line represented as helix
	{
		double r = 0, h = 1, p = 1, theta = 0;
		std::cout << "helix{" << r << ", " << h << ", " << p << ", " << theta << "} (L: " << round6(helix_length(r, h, p)) << "): \n";
		die_if(round6(helix_length(r, h, p)) != round6(1));
		auto points = helix_points(r, h, p, theta, {}, 4);
		std::cout << "start: " << points.front() << " end: " << points.back() << "\n";
	}
	
	std::cout << "\n";
	
	// unit helix
	{
		double r = 1, h = 1, p = 1, theta = 0;
		std::cout << "helix{" << r << ", " << h << ", " << p << ", " << theta << "} (L: " << round6(helix_length(r, h, p)) << "): \n";
		auto points = helix_points(r, h, p, theta, {}, 4);
		std::cout << "start: " << points.front() << " end: " << points.back() << "\n";
	}
	
	return 0;
}

