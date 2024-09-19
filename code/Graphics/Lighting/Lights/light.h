#ifndef _VOXELENG_LIGHTING_
#define _VOXELENG_LIGHTING_

#include <vec.h>
#include <Registry/registryElement.h>

namespace VoxelEng {

	//////////////
	//Constants.//
	//////////////

	const unsigned int MAX_LIGHTS = 256; // TODO. MAKE THIS NUMBER DYNAMIC IN TERMS OF HOW MANY DIFFERENT TYPES OF LIGHT ARE REGISTERED AT ENGINE'S GRAPHICAL MODE STARTUP.

	class light : public registryElement {

	public:

		// Observers.

		/**
		* @brief Get the number of arguments needed to construct an object of this class.
		* @returns The number of arguments needed to construct an object of this class.
		*/
		static unsigned int nArgs();

	protected:

		static const unsigned int nArgs_;

		vec3 diffuse_;
		vec3 specular_;

	};

	inline unsigned int light::nArgs() {

		return nArgs_;

	}

}

#endif