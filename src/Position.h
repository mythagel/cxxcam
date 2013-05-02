/*
 * Position.h
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#ifndef POSITION_H_
#define POSITION_H_
#include <string>

struct Position
{
	double X;
	double Y;
	double Z;

	double A;
	double B;
	double C;

	double U;
	double V;
	double W;

	Position();

	std::string str() const;

	bool operator==(const Position& pos) const;
	bool operator!=(const Position& pos) const;
};

#endif /* POSITION_H_ */
