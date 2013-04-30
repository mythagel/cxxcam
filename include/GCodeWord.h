/*
 * GCodeWord.h
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#ifndef GCODEWORD_H_
#define GCODEWORD_H_
#include <string>

class GCodeWord
{
public:
	enum Word
	{
		A,	// A axis of machine
		B,	// B axis of machine
		C,	// C axis of machine
		D,	// Tool radius compensation number
		F,	// Feed rate
		G,	// General function (See table Modal Groups)
		H,	// Tool length offset index
		I,	// X offset for arcs and G87 canned cycles
		J,	// Y offset for arcs and G87 canned cycles
		K,	// Z offset for arcs and G87 canned cycles. / Spindle-Motion Ratio for G33 synchronized movements.
		L,	// generic parameter word for G10, M66 and others
		M,	// Miscellaneous function (See table Modal Groups)
		P,	// Dwell time in canned cycles and with G4. / Key used with G10.
		Q,	// Feed increment in G73, G83 canned cycles
		R,	// Arc radius or canned cycle plane
		S,	// Spindle speed
		T,	// Tool selection
		U,	// U axis of machine
		V,	// V axis of machine
		W,	// W axis of machine
		X,	// X axis of machine
		Y,	// Y axis of machine
		Z	// Z axis of machine
	};
private:
	Word m_Word;
	double m_Value;
	std::string m_Comment;
public:
	GCodeWord(Word word, double value);
	GCodeWord(Word word, double value, const std::string& comment);

	operator Word() const;
	double Value() const;

	void Comment(const std::string& comment);
	std::string Comment() const;

	// Debug output
	std::string str() const;

	~GCodeWord() = default;
};

#endif /* GCODEWORD_H_ */
