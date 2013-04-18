/*
 * Material.cpp
 *
 *  Created on: 28/04/2012
 *      Author: nicholas
 */

#include "Material.h"

Material_t::Material_t(const std::string& name, double brinell)
 : m_Name(name), m_Brinell(brinell)
{
}

std::string Material_t::Name() const
{
	return m_Name;
}

double Material_t::Hardness() const
{
	return m_Brinell;
}

Material_t::~Material_t()
{
}

