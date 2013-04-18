/*
 * nef_polyhedron.h
 *
 *  Created on: 04/02/2013
 *      Author: nicholas
 */

#ifndef NEF_POLYHEDRON_H_
#define NEF_POLYHEDRON_H_
#include <istream>
#include <ostream>
#include <memory>

class nef_polyhedron_t;
std::ostream& operator<<(std::ostream& os, const nef_polyhedron_t& poly);
std::istream& operator>>(std::istream& is, nef_polyhedron_t& poly);

class nef_polyhedron_t
{
private:
	struct private_t;
	std::shared_ptr<private_t> priv;
	nef_polyhedron_t(const std::shared_ptr<private_t>& priv);
	void ensure_unique();

	friend std::ostream& operator<<(std::ostream& os, const nef_polyhedron_t& poly);
	friend std::istream& operator>>(std::istream& is, nef_polyhedron_t& poly);
public:
	nef_polyhedron_t();

	nef_polyhedron_t& operator=(const nef_polyhedron_t& poly);

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

	~nef_polyhedron_t();
};

#endif /* NEF_POLYHEDRON_H_ */
