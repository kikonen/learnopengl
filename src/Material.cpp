#include "Material.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


Material::Material(std::string& name)
{
	this->name = name;
}

Material::~Material()
{
	delete texture;
}

int Material::load(std::string& materialDir)
{
	int result = 0;
	return result;
}
