/* cxxcam - C++ CAD/CAM driver library.
 * Copyright (C) 2013  Nicholas Gill
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * polyhedron.h
 *
 *  Created on: 04/02/2013
 *      Author: nicholas
 */

#ifndef NEF_POLYHEDRON_H_
#define NEF_POLYHEDRON_H_
#include <iosfwd>
#include <memory>

#include <vector>

namespace nef
{

struct object_t;
struct polyline_t;

/*
 * Wrapper class for CGAL::Nef_polyhedron_3 to avoid long build times.
 * Intention is to add higher level interfaces (i.e. volume calculation)
 */
class polyhedron_t
{
friend polyhedron_t make_sphere(double x, double y, double z, double r, std::size_t slices);
friend polyhedron_t make_box(double x1, double y1, double z1, double x2, double y2, double z2);
friend polyhedron_t make_cone(double x1, double y1, double z1, double x2, double y2, double z2, double top_radius, double bottom_radius, std::size_t slices);

friend polyhedron_t glide(const polyhedron_t& polyhedron, const polyline_t& path);
friend double volume(const polyhedron_t& polyhedron);

friend std::vector<polyhedron_t> explode(const polyhedron_t& poly);

friend std::ostream& operator<<(std::ostream&, const polyhedron_t&);
friend std::istream& operator>>(std::istream&, polyhedron_t&);

friend object_t to_object(const polyhedron_t& poly);
friend void write_off(std::ostream&, const polyhedron_t& poly);

private:
	struct private_t;
	std::shared_ptr<private_t> priv;
	polyhedron_t(const std::shared_ptr<private_t>& priv);
	void ensure_unique();
public:
	polyhedron_t();
	polyhedron_t(const polyhedron_t&) = default;
	polyhedron_t(polyhedron_t&&) = default;
	polyhedron_t& operator=(const polyhedron_t&) = default;
	polyhedron_t& operator=(polyhedron_t&&) = default;

	polyhedron_t operator*(const polyhedron_t& poly) const;
	polyhedron_t operator+(const polyhedron_t& poly) const;
	polyhedron_t operator-(const polyhedron_t& poly) const;
	polyhedron_t operator^(const polyhedron_t& poly) const;
	polyhedron_t operator!() const;
	polyhedron_t& operator*=(const polyhedron_t& poly);
	polyhedron_t& operator+=(const polyhedron_t& poly);
	polyhedron_t& operator-=(const polyhedron_t& poly);
	polyhedron_t& operator^=(const polyhedron_t& poly);

	bool operator==(const polyhedron_t& poly) const;
	bool operator!=(const polyhedron_t& poly) const;
	bool operator<(const polyhedron_t& poly) const;
	bool operator>(const polyhedron_t& poly) const;
	bool operator<=(const polyhedron_t& poly) const;
	bool operator>=(const polyhedron_t& poly) const;
	
	~polyhedron_t();
private:
	friend polyhedron_t make_polyhedron(std::shared_ptr<private_t> priv);
	friend std::weak_ptr<private_t> get_priv(polyhedron_t& polyhedron);
	friend std::weak_ptr<const private_t> get_priv(const polyhedron_t& polyhedron);
};

std::ostream& operator<<(std::ostream&, const polyhedron_t&);
std::istream& operator>>(std::istream&, polyhedron_t&);

}

#endif /* NEF_POLYHEDRON_H_ */
