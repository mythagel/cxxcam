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

struct generic_arc
{
	double radius;
	double height_per_rev;
	double rotations;
	double start_angle_rad;
	
	struct
	{
		double x;
		double y;
		double z;
	} plane;
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

/*
normalise arc to centered around 0, 0, 0
*/
double arc_center(const gcode_arc& arc)
{
	point_3 start = {0, 0, 0};
	point_3 end = {0, 0, 0};
	double theta = 0.0;
	switch(arc.plane)
	{
		case XY:
			start = point_3{arc.start.x - arc.center.x, arc.start.y - arc.center.y, 0};
			theta = atan2(start.y, -start.x);
			end = point_3{arc.end.x - arc.center.x, arc.end.y - arc.center.y, 0};
			break;
		case ZX:
			start = point_3{arc.start.x - arc.center.x, 0, arc.start.z - arc.center.z};
			theta = atan2(start.z, -start.x);
			end = point_3{arc.end.x - arc.center.x, 0, arc.end.z - arc.center.z};
			break;
		case YZ:
			start = point_3{0, arc.start.y - arc.center.y, arc.start.z - arc.center.z};
			theta = atan2(start.z, -start.y);
			end = point_3{0, arc.end.y - arc.center.y, arc.end.z - arc.center.z};
			break;
	}
	std::cout << "start: " << start << " end: " << end << '\n';
	std::cout << "rads2start: " << theta << "\n";
	return theta;
}

double helix_length(double r, double h, double p)
{
	double c = h / (2*PI);
	
	return (2*PI*p) * sqrt((r*r) + (c*c));
}

void arc_test(const gcode_arc& arc)
{
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
				// error
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
				// error
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
				// error
			}
			break;
		}
	}
}

int main()
{
	gcode_arc simple_xy_arc = {Clockwise, XY, {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, 1};
	gcode_arc simple_zx_arc = {Clockwise, ZX, {0, 0, 0}, {1, 0, 1}, {1, 0, 0}, 1};
	gcode_arc simple_yz_arc = {Clockwise, YZ, {0, 0, 0}, {0, 1, 1}, {0, 0, 1}, 1};
	
	gcode_arc simple_xyz_helix = {Clockwise, XY, {0, 0, 0}, {1, 1, 1}, {1, 0, 0}, 1};
	
	arc_test(simple_xy_arc);
	std::cout << "\n";
	arc_test(simple_zx_arc);
	std::cout << "\n";
	arc_test(simple_yz_arc);
	std::cout << "\n";
	arc_test(simple_xyz_helix);
	std::cout << "\n\n";
	return 0;
	
	std::cout << std::boolalpha << "validate_radius(simple_xy_arc): " << validate_radius(simple_xy_arc) << ", arc_angle: " << round6(arc_angle(simple_xy_arc)) << "\n";
	die_if(round6(arc_angle(simple_xy_arc)) != round6(0.785398));	// 45 degrees
	std::cout << std::boolalpha << "validate_radius(simple_yz_arc): " << validate_radius(simple_yz_arc) << ", arc_angle: " << round6(arc_angle(simple_yz_arc)) << "\n";
	
	std::cout << std::boolalpha << "validate_radius(simple_xyz_helix): " << validate_radius(simple_xyz_helix) << ", arc_angle: " << round6(arc_angle(simple_xyz_helix)) << "\n";
	
	std::cout << "\n";
	
	gcode_arc simple_xy_arc_zero = {Clockwise, XY, {-1, 0, 0}, {0, 1, 0}, {0, 0, 0}, 1};
	arc_center(simple_xy_arc_zero);
	arc_center(simple_xy_arc);
	
	// unit circle represented as helix
	{
		double r = 1, h = 0, p = 1, theta = 0;
		std::cout << "helix{" << r << ", " << h << ", " << p << "} (L: " << round6(length(r, h, p)) << "): \n";
		die_if(round6(length(r, h, p)) != round6(2*PI));
		auto points = helix_points(r, h, p, theta, {}, 4);
		std::copy(begin(points), end(points), std::ostream_iterator<point_3>(std::cout, "\n"));
		std::cout << "start: " << points.front() << " end: " << points.back() << "\n";
	}

	std::cout << "\n";

	// unit line represented as helix
	{
		double r = 0, h = 1, p = 1, theta = 0;
		std::cout << "helix{" << r << ", " << h << ", " << p << ", " << theta << "} (L: " << round6(length(r, h, p)) << "): \n";
		die_if(round6(length(r, h, p)) != round6(1));
		auto points = helix_points(r, h, p, theta, {}, 4);
		std::copy(begin(points), end(points), std::ostream_iterator<point_3>(std::cout, "\n"));
		std::cout << "start: " << points.front() << " end: " << points.back() << "\n";
	}
	
	std::cout << "\n";
	
	// unit helix
	{
		double r = 1, h = 1, p = 1, theta = 0;
		std::cout << "helix{" << r << ", " << h << ", " << p << ", " << theta << "} (L: " << round6(length(r, h, p)) << "): \n";
		auto points = helix_points(r, h, p, theta, {}, 4);
		std::copy(begin(points), end(points), std::ostream_iterator<point_3>(std::cout, "\n"));
		std::cout << "start: " << points.front() << " end: " << points.back() << "\n";
	}
	
	std::cout << "\n";
	
	{
		
	
		double r = 100, h = 1, p = 0.35, theta = PI/4;
		std::cout << "helix{" << r << ", " << h << ", " << p << ", " << theta << "} (L: " << round6(length(r, h, p)) << "): \n";
		auto points = helix_points(r, h, p, theta, {50, 50, 50}, 60);
		std::ofstream os("points");
		std::copy(begin(points), end(points), std::ostream_iterator<point_3>(os, "\n"));
		std::cout << "start: " << points.front() << " end: " << points.back() << "\n";
		/* TODO
		convert gcode_arc to generic_arc
		gcode_arc is on XY, ZX, or YZ planes.
		generic_arc can be rotated to any plane.
		
		
		
		Find length of helix
		Find theta to start point
		Find height
		How to finish on end point?
		Translate helix points to correct location
		Rotate helix points to correct orientation (plane)
		
		*/
	}
	
	std::cout << "\n";
	
	{
		// Scaling & translation
		
		auto expected_points = helix_points(125, 1, 1, 0, {10, 10, 15}, 10);
		
		auto points = helix_points(1, 1, 1, 0, {}, 10);
		std::vector<point_3> actual;
		for(const auto& p : points)
		{
			double r = 125;
			point_3 c = {10, 10, 15};
			
			// helix axis (z) is not scaled by radius
			actual.emplace_back((p.x*r)+c.x, (p.y*r)+c.y, p.z+c.z);
		}
		
		std::cout << "Expected: \n";
		std::copy(begin(expected_points), end(expected_points), std::ostream_iterator<point_3>(std::cout, "\n"));
		std::cout << "\n";
		std::cout << "Actual: \n";
		std::copy(begin(actual), end(actual), std::ostream_iterator<point_3>(std::cout, "\n"));
		
		std::cout << "Expected start: " << expected_points.front() << " end: " << expected_points.back() << "\n";
		std::cout << "Actual   start: " << actual.front() << " end: " << actual.back() << "\n";
	}
	
	return 0;
}

