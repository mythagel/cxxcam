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

#ifndef GEOM_POLYHEDRON_H_
#define GEOM_POLYHEDRON_H_
#include <iosfwd>
#include <memory>

#include <vector>

namespace geom
{

/*
 * Wrapper class for CGAL::Nef_polyhedron_3 to avoid long build times.
 * Intention is to add higher level interfaces (i.e. volume calculation)
 */
class polyhedron_t
{
public:
	struct private_t;
	friend polyhedron_t make_polyhedron(std::shared_ptr<private_t> priv);
	friend std::shared_ptr<private_t> get_priv(polyhedron_t& polyhedron);
	friend std::shared_ptr<const private_t> get_priv(const polyhedron_t& polyhedron);
	
	friend std::ostream& operator<<(std::ostream&, const polyhedron_t&);
	friend std::istream& operator>>(std::istream&, polyhedron_t&);

private:
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
	
	bool empty() const;
	
	~polyhedron_t();
};

namespace format
{
// Use OFF file format (default; recommended)
std::ios_base& off(std::ios_base&);
// Use OFF file format (useful in some situations but non-portable)
std::ios_base& nef(std::ios_base&);
}

std::ostream& operator<<(std::ostream&, const polyhedron_t&);
std::istream& operator>>(std::istream&, polyhedron_t&);

}

#endif /* GEOM_POLYHEDRON_H_ */
