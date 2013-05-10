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
 * Spindle.h
 *
 *  Created on: 08/05/2012
 *      Author: nicholas
 */

#ifndef SPINDLE_H_
#define SPINDLE_H_
#include <string>
#include <set>

/*
 * Represents real spindle speeds attainable by a particular machine.
 */
class Spindle
{
private:
	struct Entry
	{
		const enum
		{
			type_Range,
			type_Discrete
		} m_Type;

		union
		{
			struct
			{
				const unsigned long m_RangeStart;
				const unsigned long m_RangeEnd;
			};
			const unsigned long m_Discrete;
		};
		Entry(unsigned long range_start, unsigned long range_end);
		explicit Entry(unsigned long discrete_value);

		bool Contains(unsigned long speed) const;
		long Distance(unsigned long speed) const;

		bool operator<(const Entry& other) const;
	};

	std::set<Entry> m_Entries;
	unsigned long m_Tolerance;
public:
	Spindle(unsigned long tolerance = 100);

	/*
	 * Given a requested speed, find the closest real machine speed possible.
	 * Will throw an exception if the attainable speed is outside the
	 * given tolerance.
	 */
	unsigned long Normalise(unsigned long requested_speed) const;

	void AddRange(unsigned long range_start, unsigned long range_end);
	void AddDiscrete(unsigned long discrete_value);

	std::string str() const;
};

#endif /* SPINDLE_H_ */
