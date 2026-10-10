#ifndef _VOXELENG_RENDERMODE_
#define _VOXELENG_RENDERMODE_

namespace VoxelEng {

	// TODO. DEFINE A CUSTOM FILE FORMAT THAT IS INTERPRETED BY THE ENGINE AND 'COMPILED' INTO EVERYWHERE WHERE IS IT NEEDED.
	enum class renderMode : int {
	
		NONE = 0,
		MODE_3D = 1,
		MODE_2D = 2
	
	};

	inline int toInt(renderMode mode) {
	
		return static_cast<int>(mode);
	
	}

}

#endif