/*
 * nef_polyhedron.h
 *
 *  Created on: 04/02/2013
 *      Author: nicholas
 */

#ifndef NEF_POLYHEDRON_H_
#define NEF_POLYHEDRON_H_
#include <iosfwd>
#include <memory>
#include <vector>

struct polyline_t
{
	struct point
	{
		double x;
		double y;
		double z;
	};
	std::vector<point> line;
};

/*
 * Wrapper class for CGAL::Nef_polyhedron_3 to avoid long build times.
 * Intention is to add higher level interfaces (i.e. volume calculation)
 */
class nef_polyhedron_t
{
private:
	struct private_t;
	std::shared_ptr<private_t> priv;
	nef_polyhedron_t(const std::shared_ptr<private_t>& priv);
	void ensure_unique();

	friend std::ostream& operator<<(std::ostream&, const nef_polyhedron_t&);
	friend std::istream& operator>>(std::istream&, nef_polyhedron_t&);
public:
	nef_polyhedron_t();
	nef_polyhedron_t(const nef_polyhedron_t&) = default;
	nef_polyhedron_t(nef_polyhedron_t&&) = default;
	nef_polyhedron_t& operator=(const nef_polyhedron_t&) = default;
	nef_polyhedron_t& operator=(nef_polyhedron_t&&) = default;

	nef_polyhedron_t operator*(const nef_polyhedron_t& poly) const;
	nef_polyhedron_t operator+(const nef_polyhedron_t& poly) const;
	nef_polyhedron_t operator-(const nef_polyhedron_t& poly) const;
	nef_polyhedron_t operator^(const nef_polyhedron_t& poly) const;
	nef_polyhedron_t operator!() const;
	nef_polyhedron_t& operator*=(const nef_polyhedron_t& poly);
	nef_polyhedron_t& operator+=(const nef_polyhedron_t& poly);
	nef_polyhedron_t& operator-=(const nef_polyhedron_t& poly);
	nef_polyhedron_t& operator^=(const nef_polyhedron_t& poly);

	bool operator==(const nef_polyhedron_t& poly) const;
	bool operator!=(const nef_polyhedron_t& poly) const;
	bool operator<(const nef_polyhedron_t& poly) const;
	bool operator>(const nef_polyhedron_t& poly) const;
	bool operator<=(const nef_polyhedron_t& poly) const;
	bool operator>=(const nef_polyhedron_t& poly) const;

	nef_polyhedron_t glide(const polyline_t& path) const;
	double volume() const;
};

std::ostream& operator<<(std::ostream&, const nef_polyhedron_t&);
std::istream& operator>>(std::istream&, nef_polyhedron_t&);

#endif /* NEF_POLYHEDRON_H_ */
