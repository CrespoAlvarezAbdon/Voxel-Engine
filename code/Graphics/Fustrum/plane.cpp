#include "plane.h"

#include "../../logger.h"


namespace VoxelEng {

	bool plane::initialized_ = false;
	const model* plane::planeVertices_ = nullptr;
	const modelTriangles* plane::planeTriangles_ = nullptr;


	void plane::init() {
	
		if (initialized_) {
		
			logger::errorLog("Plane system already initialized");
		
		}
		else {
		
			planeVertices_ = &models::getModelAt(3);
			planeTriangles_ = &models::getModelTriangles(3);

			initialized_ = true;
		
		}
	
	}

	plane::plane() 
	: normal_(glm::normalize(vec3FixedUp)),
	  distance_(0)
	{}

	plane::plane(const vec3& point, const vec3& normal)
	: normal_(glm::normalize(normal)),
	  distance_(glm::dot(point, normal_))
	{}

	void plane::normal(const vec3& newValue) {
	
		normal_.x = newValue.x;
		normal_.y = newValue.y;
		normal_.z = newValue.z;

		normal_ = glm::normalize(normal_);
	
	}

	void plane::distance(const vec3& pointWithNewDistance) {
	
		distance_ = glm::dot(pointWithNewDistance, normal_);
	
	}

	void plane::generateVertices(float planeSize) {
	
		model_ = model();
		for (int face = 0; face < planeTriangles_->size(); face++) {
		
			const triangle* triangles = &planeTriangles_->operator[](face);

			for (int vertex = 0; vertex < triangles->size(); vertex++)
				model_.push_back(planeVertices_->operator[](triangles->operator[](vertex)));
		
		}
		transform_.position = vec3(0, 110, 0);
		transform_.Yaxis = glm::normalize(vec3(0, 1, 1));
		transform_.scale = vec3(2, 2, 2);
		models::applyTransform(model_, transform_, applyRotationMode::DIRECTION_VECTOR, true, true, true);
	
	}

	void plane::reset() {
	
		if (initialized_) {
		
			planeVertices_ = nullptr;
			planeTriangles_ = nullptr;

			initialized_ = false;
		
		}
		else {

			logger::errorLog("Plane system is not initialized");

		}
	
	}

}