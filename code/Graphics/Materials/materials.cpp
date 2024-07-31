#include "materials.h"

#include "../../logger.h"

namespace VoxelEng {

	const material& material::getMaterial(const std::string& name) {
	
		return setMaterial(name);
	
	}

	void material::registerMaterial(const std::string& name,
		float ambientR, float ambientG, float ambientB,
		float diffuseR, float diffuseG, float diffuseB,
		float specularR, float specularG, float specularB, float shininess) {

		if (materials_.contains(name))
			logger::errorLog("The material " + name + " is already registered.");
		else
			materials_[name] = material(ambientR, ambientG, ambientB, diffuseR, diffuseG, diffuseB, specularR, specularG, specularB, shininess);

	}

	material& material::setMaterial(const std::string& name) {

		if (materials_.contains(name))
			return materials_[name];
		else
			logger::errorLog("The material " + name + " is not registered.");

	}

	void material::unregisterMaterial(const std::string& name) {

		if (materials_.contains(name))
			materials_.erase(name);
		else
			logger::errorLog("The material " + name + " is not registered.");

	}

	material::material()
	: ambient{ 1.0f, 1.0f, 1.0f }, diffuse{ 1.0f, 1.0f, 1.0f }, specular{ 1.0f, 1.0f, 1.0f }, shininess(32)
	{
	
	
	
	}

	material::material(float ambientR, float ambientG, float ambientB,
		float diffuseR, float diffuseG, float diffuseB,
		float specularR, float specularG, float specularB, float shininess)
	: ambient{ ambientR, ambientG, ambientB }, diffuse{ diffuseR, diffuseG, diffuseB }, specular{ specularR, specularG, specularB }, shininess(shininess)
	{



	}

}