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

namespace gcode
{

class Line
{
private:
	std::vector<Word> m_Words;
	std::string m_Comment;
public:

	typedef std::vector<Word>::const_iterator const_iterator;

	Line();
	explicit Line(const std::string& comment);
	explicit Line(const Word& word, const std::string& comment = {});

	const_iterator begin() const;
	const_iterator end() const;
	bool empty() const;

	void Comment(const std::string& comment);
	std::string Comment() const;

	Line& operator+=(const Word& word);

	std::string debug_str() const;

	~Line() = default;
};

}

#endif /* GCODELINE_H_ */
