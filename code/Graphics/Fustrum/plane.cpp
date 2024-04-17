#include "plane.h"

#include "../../logger.h"


namespace VoxelEng {

	bool plane::initialized_ = false;

	void plane::init() {
	
		if (initialized_) {
		
			logger::errorLog("Plane system already initialized");
		
		}
		else {

			initialized_ = true;
		
		}
	
	}

	plane::plane() 
	: entity(),
	  distance_(0)
	{
	
		entityModel(planeModelID);
		applyRotationMode_ = applyRotationMode::DIRECTION_VECTOR;
		transform_.Yaxis = glm::normalize(vec3FixedUp);
	
	}

	plane::plane(const vec3& point, const vec3& normal)
	: entity(),
	  distance_(glm::dot(point, normal))
	{
	
		entityModel(planeModelID);
		transform_.position = point;
		applyRotationMode_ = applyRotationMode::DIRECTION_VECTOR;
		transform_.Yaxis = glm::normalize(normal);
	
	}

	const vec3& plane::normal() const {
	
		return transform_.Yaxis;
	
	}

	void plane::normal(const vec3& newValue) {
	
		transform_.Yaxis.x = newValue.x;
		transform_.Yaxis.y = newValue.y;
		transform_.Yaxis.z = newValue.z;

		transform_.Yaxis = glm::normalize(transform_.Yaxis);

		// It is not necessary to set the updateXYXRotation flags because those flags are used only
		// with applyRotationMode::EULER_ANGLES.
	
	}

	void plane::distance(const vec3& pointWithNewDistance) {
	
		distance_ = glm::dot(pointWithNewDistance, transform_.Yaxis);
	
	}

	void plane::reset() {
	
		if (initialized_) {

			initialized_ = false;
		
		}
		else {

			logger::errorLog("Plane system is not initialized");

		}
	
	}

}