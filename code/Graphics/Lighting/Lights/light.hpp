#ifndef _VOXELENG_LIGHT_
#define _VOXELENG_LIGHT_

#include <vec.h>
#include <Graphics/Lighting/definitions.hpp>
#include <Registry/registryElement.h>

namespace VoxelEng {

	//////////////
	//Constants.//
	//////////////

	const unsigned int MAX_LIGHTS = 256; // TODO. MAKE THIS NUMBER DYNAMIC IN TERMS OF HOW MANY DIFFERENT TYPES OF LIGHT ARE REGISTERED AT ENGINE'S GRAPHICAL MODE STARTUP.

	class light : public registryElement {

	public:

		// Initialisation.

		/**
		* @brief Initialise the registryElement system.
		*/
		static void init(const std::string& typeName);


		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		light();

		/**
		* @brief Class constructor.
		* @param ambientR Percentage of red color emitted in ambient lighting calculations by this light type.
		* @param ambientG Percentage of green color emitted in ambient lighting calculations by this light type.
		* @param ambientB Percentage of blue color emitted in ambient lighting calculations by this light type.
		* @param diffuseR Percentage of red color emitted in diffuse lighting calculations by this light type.
		* @param diffuseG Percentage of green color emitted in diffuse lighting calculations by this light type.
		* @param diffuseB Percentage of blue color emitted in diffuse lighting calculations by this light type.
		* @param specularR Percentage of red color emitted in specular lighting calculations by this light type.
		* @param specularG Percentage of green color emitted in specular lighting calculations by this light type.
		* @param specularB Percentage of blue color emitted in specular lighting calculations by this light type.
		*/
		light(lightValue ambientR, lightValue ambientG, lightValue ambientB, lightValue ambientA,
			lightValue diffuseR, lightValue diffuseG, lightValue diffuseB, lightValue diffuseA,
			lightValue specularR, lightValue specularG, lightValue specularB, lightValue specularA);


		// Observers.

		/**
		* @brief Get the registry element's typename.
		*/
		static const std::string& typeName();

		/**
		* @brief Get the number of arguments needed to construct an object of this class.
		* @returns The number of arguments needed to construct an object of this class.
		*/
		static unsigned int nArgs();

		/**
		* @brief Get the ambient color provided by this light.
		*/
		basicVec4 ambient() const;

		/**
		* @brief Get the diffuse color provided by this light.
		*/
		basicVec4 diffuse() const;

		/**
		* @brief Get the specular color provided by this light.
		*/
		basicVec4 specular() const;

	protected:

		static bool initialised_;
		static std::string typeName_;
		static const unsigned int nArgs_;

		basicVec4 ambient_;
		basicVec4 diffuse_;
		basicVec4 specular_;

	};

	inline light::light()
	: ambient_(basicVec4Zero), diffuse_(basicVec4Zero), specular_(basicVec4Zero)
	{}

	inline light::light(lightValue ambientR, lightValue ambientG, lightValue ambientB, lightValue ambientA,
		lightValue diffuseR, lightValue diffuseG, lightValue diffuseB, lightValue diffuseA,
		lightValue specularR, lightValue specularG, lightValue specularB, lightValue specularA)
	: ambient_{ ambientR, ambientG, ambientB, ambientA, }, diffuse_{ diffuseR, diffuseG, diffuseB, diffuseA },
	  specular_{ specularR, specularG, specularB, specularA }
	{}

	inline const std::string& light::typeName() {

		return typeName_;

	}

	inline unsigned int light::nArgs() {

		return nArgs_;

	}

	inline basicVec4 light::ambient() const
	{
		return ambient_;
	}

	inline basicVec4 light::diffuse() const
	{
		return diffuse_;
	}

	inline basicVec4 light::specular() const
	{
		return specular_;
	}

}

#endif