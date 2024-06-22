#include "plane.h"

namespace VoxelEng {

	plane::plane() 
	: entity()
	{
	
		entityModel(planeModelID);
		applyRotationMode_ = applyRotationMode::DIRECTION_VECTOR;
		transform_.Yaxis = glm::normalize(vec3FixedUp);
	
	}

	plane::plane(const vec3& point, const vec3& normal)
	: entity(),
	  mathPlane_(point, normal)
	{
	
		entityModel(planeModelID);
		applyRotationMode_ = applyRotationMode::DIRECTION_VECTOR;
		transform_.position = point;
		transform_.Yaxis = mathPlane_.normal();
		
	}

	void plane::point(const vec3& newPoint) {

		mathPlane_.point(newPoint);
		transform_.position = mathPlane_.point();

	}

	void plane::normal(const vec3& newValue) {
	
		mathPlane_.normal(newValue);
		transform_.Yaxis = mathPlane_.normal();

		// It is not necessary to set the updateXYXRotation flags because those flags are used only
		// with applyRotationMode::EULER_ANGLES.
	
	}

}