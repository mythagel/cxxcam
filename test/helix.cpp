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
	size_t steps_per_mm = 1;
	std::cout << arc << "\n";
	// Steps:
	// 0. Determine center point & radius
	// 1. Determine theta to start from center and start point.
	// 2. Determine theta to end from center and end point.
	// 3. Determine revolutions by determining number of radians total possible revolutions (2PI*turns).
	// 4. Subtract end theta from above.
	// 5. Determine step size (theta from step distance on circumference)

	// Note this is the 3d distance. zero the helix angle before checking.	
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
			auto center = point_3{arc.center.x, arc.center.y, 0};
			
			if(!equidistant(start, end, center))
				throw std::runtime_error("XY arc center not equidistant from start and end points.");
			
			double r = distance(start, center);
			double start_theta = atan2(start.y - center.y, start.x - center.x);
			double end_theta = atan2(end.y - center.y, end.x - center.x);
			double turn_theta = 2*PI*(arc.turns-1);
			auto delta_theta = (end_theta - start_theta);
			switch(arc.dir)
			{
				case Clockwise:
				{
					if(delta_theta > 0)
						delta_theta -= 2*PI;
					break;
				}
				case CounterClockwise:
				{
					if(delta_theta < 0)
						delta_theta += 2*PI;
					break;
				}
			}
			if(delta_theta == 0.0)
				delta_theta = 2*PI;
			
			turn_theta += fabs(delta_theta);
			
			double l = helix_length(r, helix / turn_theta, turn_theta);
			double rads_per_step = turn_theta / static_cast<double>(l * steps_per_mm);
			
			std::cout << "Plane: XY\n";
			std::cout << "r    : " << r << "\n";
			std::cout << "hx   : " << helix << "\n";
			std::cout << "td   : " << delta_theta << "\n";
			std::cout << "t0   : " << start_theta << "\n";
			std::cout << "t1   : " << end_theta << "\n";
			std::cout << "tn   : " << turn_theta << "\n";
			std::cout << "l    : " << l << "\n";

			double ch = helix / turn_theta;
			std::vector<point_3> P;
			{
				double step = delta_theta < 0 ? -rads_per_step : rads_per_step;
				size_t n_steps = (l * steps_per_mm);
				
				double t = start_theta;
				double hdt = helix / n_steps;
				for(size_t i = 0; i < n_steps; ++i, t += step)
					P.push_back({ (cos(t)*r)+arc.center.x, (sin(t)*r)+arc.center.y, (hdt*i) + arc.start.z });
			}
			P.push_back(arc.end);
			std::copy(P.begin(), P.end(), std::ostream_iterator<point_3>(std::cout, "\n"));
			break;
		}
		case ZX:
		{
			auto start = point_3{arc.start.x, arc.start.z, 0};
			auto end = point_3{arc.end.x, arc.end.z, 0};
			auto helix = arc.end.y - arc.start.y;
			auto center = point_3{arc.center.x, arc.center.z, 0};
			
			if(!equidistant(start, end, center))
				throw std::runtime_error("ZX arc center not equidistant from start and end points.");
			
			double r = distance(start, center);
			double start_theta = atan2(start.y - center.y, start.x - center.x);
			double end_theta = atan2(end.y - center.y, end.x - center.x);
			double turn_theta = 2*PI*(arc.turns-1);
			auto delta_theta = (end_theta - start_theta);
			switch(arc.dir)
			{
				case Clockwise:
				{
					if(delta_theta > 0)
						delta_theta -= 2*PI;
					break;
				}
				case CounterClockwise:
				{
					if(delta_theta < 0)
						delta_theta += 2*PI;
					break;
				}
			}
			turn_theta += fabs(delta_theta);
			
			double l = helix_length(r, helix / turn_theta, turn_theta);
			double rads_per_step = turn_theta / static_cast<double>(l * steps_per_mm);
			
			std::cout << "Plane: ZX\n";
			std::cout << "r    : " << r << "\n";
			std::cout << "hx   : " << helix << "\n";
			std::cout << "td   : " << delta_theta << "\n";
			std::cout << "t0   : " << start_theta << "\n";
			std::cout << "t1   : " << end_theta << "\n";
			std::cout << "tn   : " << turn_theta << "\n";
			std::cout << "l    : " << l << "\n";

			double ch = helix / turn_theta;
			std::vector<point_3> P;
			{
				double step = delta_theta < 0 ? -rads_per_step : rads_per_step;
				size_t n_steps = (l * steps_per_mm);
				
				double t = start_theta;
				double hdt = helix / n_steps;
				for(size_t i = 0; i < n_steps; ++i, t += step)
					P.push_back({ (cos(t)*r)+arc.center.x, (hdt*i) + arc.start.y, (sin(t)*r)+arc.center.z});
			}
			P.push_back(arc.end);
			std::copy(P.begin(), P.end(), std::ostream_iterator<point_3>(std::cout, "\n"));
			break;
		}
		case YZ:
		{
			auto start = point_3{arc.start.z, arc.start.y, 0};
			auto end = point_3{arc.end.z, arc.end.y, 0};
			auto helix = arc.end.x - arc.start.x;
			auto center = point_3{arc.center.z, arc.center.y, 0};
			
			if(!equidistant(start, end, center))
				throw std::runtime_error("YZ arc center not equidistant from start and end points.");
			
			double r = distance(start, center);
			double start_theta = atan2(start.y - center.y, start.x - center.x);
			double end_theta = atan2(end.y - center.y, end.x - center.x);
			double turn_theta = 2*PI*(arc.turns-1);
			auto delta_theta = (end_theta - start_theta);
			switch(arc.dir)
			{
				case Clockwise:
				{
					if(delta_theta > 0)
						delta_theta -= 2*PI;
					break;
				}
				case CounterClockwise:
				{
					if(delta_theta < 0)
						delta_theta += 2*PI;
					break;
				}
			}
			turn_theta += fabs(delta_theta);
			
			double l = helix_length(r, helix / turn_theta, turn_theta);
			double rads_per_step = turn_theta / static_cast<double>(l * steps_per_mm);
			
			std::cout << "Plane: YZ\n";
			std::cout << "r    : " << r << "\n";
			std::cout << "hx   : " << helix << "\n";
			std::cout << "td   : " << delta_theta << "\n";
			std::cout << "t0   : " << start_theta << "\n";
			std::cout << "t1   : " << end_theta << "\n";
			std::cout << "tn   : " << turn_theta << "\n";
			std::cout << "l    : " << l << "\n";

			double ch = helix / turn_theta;
			std::vector<point_3> P;
			{
				double step = delta_theta < 0 ? -rads_per_step : rads_per_step;
				size_t n_steps = (l * steps_per_mm);
				
				double t = start_theta;
				double hdt = helix / n_steps;
				for(size_t i = 0; i < n_steps; ++i, t += step)
					P.push_back({ (hdt*i) + arc.start.x, (sin(t)*r)+arc.center.y, (cos(t)*r)+arc.center.z});
			}
			P.push_back(arc.end);
			std::copy(P.begin(), P.end(), std::ostream_iterator<point_3>(std::cout, "\n"));
			break;
		}
	}
	std::cout << "\n" << std::string(50, '=') << "\n\n";
}

int main()
{
	gcode_arc simple_xy_arc = {Clockwise, XY, {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, 1};
	gcode_arc simple_xy_arc_ccw = {CounterClockwise, XY, {0, 0, 1}, {1, 1, 1}, {1, 0, 1}, 1};
	gcode_arc simple_xy_arc_opp = {Clockwise, XY, {1, 1, 2}, {0, 0, 2}, {1, 0, 2}, 1};
	gcode_arc simple_xy_arc_ccw_opp = {CounterClockwise, XY, {1, 1, 3}, {0, 0, 3}, {1, 0, 3}, 1};

	gcode_arc simple_zx_arc = {Clockwise, ZX, {0, 0, 0}, {1, 0, 1}, {1, 0, 0}, 1};
	gcode_arc simple_zx_arc_ccw = {CounterClockwise, ZX, {0, 0, 0}, {1, 0, 1}, {1, 0, 0}, 1};
	gcode_arc simple_zx_helix = {Clockwise, ZX, {0, 0, 0}, {1, 1, 1}, {1, 0, 0}, 1};

	gcode_arc simple_yz_arc = {Clockwise, YZ, {0, 0, 0}, {0, 1, 1}, {0, 0, 1}, 1};
	gcode_arc simple_xyz_helix = {Clockwise, XY, {0, 0, 0}, {1, 1, 1}, {1, 0, 0}, 1};
	gcode_arc xy_circle = {Clockwise, XY, {1, 0, 0}, {1, 0, 0}, {0, 0, 0}, 1};
	gcode_arc xyz_helix = {Clockwise, XY, {1, 0, 0}, {1, 0, 1}, {0, 0, 0}, 1};

	std::cout << "simple_xy_arc\n";
	arc_test(simple_xy_arc);
	std::cout << "simple_xy_arc_ccw\n";
	arc_test(simple_xy_arc_ccw);
	std::cout << "simple_xy_arc_opp\n";
	arc_test(simple_xy_arc_opp);
	std::cout << "simple_xy_arc_ccw_opp\n";
	arc_test(simple_xy_arc_ccw_opp);
	
	std::cout << "simple_zx_arc\n";
	arc_test(simple_zx_arc);
	std::cout << "simple_zx_arc_ccw\n";
	arc_test(simple_zx_arc_ccw);
	std::cout << "simple_zx_helix\n";
	arc_test(simple_zx_helix);	

	std::cout << "simple_yz_arc\n";
	arc_test(simple_yz_arc);
	
	std::cout << "simple_xyz_helix\n";
	arc_test(simple_xyz_helix);
	
	std::cout << "xy_circle\n";
	arc_test(xy_circle);
	
	std::cout << "xyz_helix\n";
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

