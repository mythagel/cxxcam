/*
 * Position.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "Position.h"
#include <sstream>

Position::Position()
 : X(),
   Y(),
   Z(),

   A(),
   B(),
   C(),

   U(),
   V(),
   W()
{
}

std::string Position::str() const
{
	std::stringstream s;

	if(X != 0.0 || Y != 0.0 || Z != 0.0)
	{
		s << "X: " << X << " ";
		s << "Y: " << Y << " ";
		s << "Z: " << Z << " \n";
	}

	if(A != 0.0 || B != 0.0 || C != 0.0)
	{
		s << "A: " << A << " ";
		s << "B: " << B << " ";
		s << "C: " << C << " \n";
	}

	if(U != 0.0 || V != 0.0 || W != 0.0)
	{
		s << "U: " << U << " ";
		s << "V: " << V << " ";
		s << "W: " << W << " \n";
	}

	return s.str();
}

bool Position::operator==(const Position& pos) const
{
	return 	(X == pos.X) &&
			(Y == pos.Y) &&
			(Z == pos.Z) &&

			(A == pos.A) &&
			(B == pos.B) &&
			(C == pos.C) &&

			(U == pos.U) &&
			(V == pos.V) &&
			(W == pos.W);
}
bool Position::operator!=(const Position& pos) const
{
	return 	(X != pos.X) ||
			(Y != pos.Y) ||
			(Z != pos.Z) ||

			(A != pos.A) ||
			(B != pos.B) ||
			(C != pos.C) ||

			(U != pos.U) ||
			(V != pos.V) ||
			(W != pos.W);
}
