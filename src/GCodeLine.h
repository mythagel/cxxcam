/*
 * GCodeLine.h
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#ifndef GCODELINE_H_
#define GCODELINE_H_
#include <vector>
#include <string>
#include "GCodeWord.h"

class GCodeLine
{
private:
	std::vector<GCodeWord> m_Words;
	std::string m_Comment;
public:

	typedef std::vector<GCodeWord>::const_iterator const_iterator;

	GCodeLine();
	explicit GCodeLine(const std::string& comment);
	explicit GCodeLine(const GCodeWord& word, const std::string& comment = {});

	const_iterator begin() const;
	const_iterator end() const;
	bool empty() const;

	void Comment(const std::string& comment);
	std::string Comment() const;

	GCodeLine& operator+=(const GCodeWord& word);

	// Debug output
	std::string str() const;

	~GCodeLine() = default;
};

#endif /* GCODELINE_H_ */
