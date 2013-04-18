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
public:
	Spindle();

	/*
	 * Given a requested speed, find the closest real machine speed possible.
	 */
	unsigned long Normalise(unsigned long requested_speed);

	void AddRange(unsigned long range_start, unsigned long range_end);
	void AddDiscrete(unsigned long discrete_value);

	std::string str() const;

	~Spindle();
};

#endif /* SPINDLE_H_ */
