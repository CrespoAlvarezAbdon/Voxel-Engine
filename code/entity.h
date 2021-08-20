#ifndef _ENTITY_
#define _ENTITY_
#include <glm.hpp>
#include "camera.h"

class Player
{

public:

	Player(float FOV, float width, float height, float z_near, float z_far, GLFWwindow* window, const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& direction = glm::vec3(0.0f, 0.0f, 0.0f));

	const Camera& camera() const noexcept;
	Camera& camera() noexcept;

private:

	Camera camera_;

};

inline const Camera& Player::camera() const noexcept
{

	return camera_;

}

inline Camera& Player::camera() noexcept
{

	return camera_;

}

#endif