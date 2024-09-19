#ifndef _VOXELENG_DIRECTIONAL_LIGHTING_
#define _VOXELENG_DIRECTIONAL_LIGHTING_

#include <vec.h>
#include <Registry/registryElement.h>
#include <Graphics/Lighting/lights/light.h>

namespace VoxelEng {

	class directionalLight : public light {

	public:

		// Observers.

		/**
		* @brief Get the number of arguments needed to construct an object of this class.
		* @returns The number of arguments needed to construct an object of this class.
		*/
		static unsigned int nArgs();

	protected:

		static const unsigned int nArgs_;

		vec3 direction_;
		
	};

	inline unsigned int directionalLight::nArgs() {

		return light::nArgs_ + nArgs_;

	}

}

#endif