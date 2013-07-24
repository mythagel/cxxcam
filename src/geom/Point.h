// Copyright (C) 2006-2008 Anders Logg
//
// This file is part of DOLFIN.
//
// DOLFIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DOLFIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.
//
// Modified by Garth N. Wells, 2006.
// Modified by Andre Massing, 2009.
// Significantly Modified by Nicholas Gill, 2013
//
// First added:  2006-06-12
// Last changed: 2011-04-13

#ifndef __POINT_H
#define __POINT_H

#include <array>
#include <cassert>
#include <CGAL/Bbox_3.h>
#include <CGAL/Point_3.h>

namespace dolfin
{

inline bool near(double x, double x0, double eps = std::numeric_limits<double>::epsilon())
{
	return x0 - eps <= x && x <= x0 + eps;
}

/// A Point represents a point in :math:`\mathbb{R}^3` with
/// coordinates :math:`x, y, z,` or alternatively, a vector in
/// :math:`\mathbb{R}^3`, supporting standard operations like the
/// norm, distances, scalar and vector products etc.

class Point
{
public:

	Point()
	{
		_x[0] = 0.0;
		_x[1] = 0.0;
		_x[2] = 0.0;
	}

	/// Create a point at (x, y, z). Default value (0, 0, 0).
	///
	/// *Arguments*
	///     x (double)
	///         The x-coordinate.
	///     y (double)
	///         The y-coordinate.
	///     z (double)
	///         The z-coordinate.
	Point(const double x, const double y, const double z)
	{
		_x[0] = x;
		_x[1] = y;
		_x[2] = z;
	}

	/// Return x-coordinate
	///
	/// *Returns*
	///     double
	///         The x-coordinate.
	double x() const
	{
		return _x[0];
	}

	/// Return y-coordinate
	///
	/// *Returns*
	///     double
	///         The y-coordinate.
	double y() const
	{
		return _x[1];
	}

	/// Return z-coordinate
	///
	/// *Returns*
	///     double
	///         The z-coordinate.
	double z() const
	{
		return _x[2];
	}

	/// Compute sum of two points
	Point operator+(const Point& p) const
	{
		return {_x[0] + p._x[0], _x[1] + p._x[1], _x[2] + p._x[2]};
	}

	/// Compute difference of two points
	Point operator-(const Point& p) const
	{
		return {_x[0] - p._x[0], _x[1] - p._x[1], _x[2] - p._x[2]};
	}

	/// Add given point
	const Point& operator+=(const Point& p)
	{
		_x[0] += p._x[0];
		_x[1] += p._x[1];
		_x[2] += p._x[2];
		return *this;
	}

	/// Subtract given point
	const Point& operator-=(const Point& p)
	{
		_x[0] -= p._x[0];
		_x[1] -= p._x[1];
		_x[2] -= p._x[2];
		return *this;
	}

	/// Multiplication with scalar
	Point operator*(double a) const
	{
		return {a * _x[0], a * _x[1], a * _x[2]};
	}

	/// Incremental multiplication with scalar
	const Point& operator*=(double a)
	{
		_x[0] *= a;
		_x[1] *= a;
		_x[2] *= a;
		return *this;
	}

	/// Division by scalar
	Point operator/(double a) const
	{
		return {_x[0] / a, _x[1] / a, _x[2] / a};
	}

	/// Incremental division by scalar
	const Point& operator/=(double a)
	{
		_x[0] /= a;
		_x[1] /= a;
		_x[2] /= a;
		return *this;
	}

	/// Assignment operator
	const Point& operator=(const Point& p)
	{
		_x[0] = p._x[0];
		_x[1] = p._x[1];
		_x[2] = p._x[2];
		return *this;
	}

	/// Conversion operator to appropriate CGAL Point_3 class.
	template<typename Kernel>
	operator CGAL::Point_3<Kernel>() const
	{
		return CGAL::Point_3<Kernel>(_x[0], _x[1], _x[2]);
	}

	/// Constructor taking a CGAL::Point_3. Allows conversion from
	/// CGAL Point_3 class to Point class.
	template<typename Kernel>
	Point(const CGAL::Point_3<Kernel> & point)
	{
		_x[0] = point.x();
		_x[1] = point.y();
		_x[2] = point.z();
	}

	/// Provides a CGAL bounding box, using conversion operator.
	template<typename Kernel>
	CGAL::Bbox_3 bbox()
	{
		return CGAL::Point_3<Kernel>(*this).bbox();
	}

	/// Compute distance to given point
	///
	/// *Arguments*
	///     p (_Point_)
	///         The point to compute distance to.
	///
	/// *Returns*
	///     double
	///         The distance.
	///
	/// *Example*
	///     .. code-block:: c++
	///
	///         Point p1(0, 4, 0);
	///         Point p2(2, 0, 4);
	///         info("%g", p1.distance(p2));
	///
	///     output::
	///
	///         6
	double distance(const Point& p) const;

	/// Compute norm of point representing a vector from the origin
	///
	/// *Returns*
	///     double
	///         The (Euclidean) norm of the vector from the origin to
	///         the point.
	///
	/// *Example*
	///     .. code-block:: c++
	///
	///         Point p(1.0, 2.0, 2.0);
	///         info("%g", p.norm());
	///
	///     output::
	///
	///         3
	double norm() const;

	/// Compute cross product with given vector
	///
	/// *Arguments*
	///     p (_Point_)
	///         Another point.
	///
	/// *Returns*
	///     Point
	///         The cross product.
	const Point cross(const Point& p) const;

	/// Compute dot product with given vector
	///
	/// *Arguments*
	///     p (_Point_)
	///         Another point.
	///
	/// *Returns*
	///     double
	///         The dot product.
	///
	/// *Example*
	///     .. code-block:: c++
	///
	///         Point p1(1.0, 4.0, 8.0);
	///         Point p2(2.0, 0.0, 0.0);
	///         info("%g", p1.dot(p2));
	///
	///     output::
	///
	///         2
	double dot(const Point& p) const;

	/// Rotate around a given axis
	///
	/// *Arguments*
	///     a (_Point_)
	///         The axis to rotate around. Must be unit length.
	///     theta (_double_)
	///         The rotation angle.
	///
	/// *Returns*
	///     Point
	///         The rotated point.
	Point rotate(const Point& a, double theta) const;

	// Note: Not a subclass of Variable for efficiency!

	/// Return informal string representation (pretty-print)
	///
	/// *Arguments*
	///     verbose (bool)
	///         Flag to turn on additional output.
	///
	/// *Returns*
	///     std::string
	///         An informal representation of the function space.
	//std::string str(bool verbose=false) const;

private:
	std::array<double, 3> _x;
};

/// Multiplication with scalar
inline Point operator*(double a, const Point& p)
{
	return p * a;
}

}

#endif
