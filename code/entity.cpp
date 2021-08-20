#include "entity.h"

Player::Player(float FOV, float width, float height, float z_near, float z_far, GLFWwindow* window, const glm::vec3& position, const glm::vec3& direction)
	: camera_(FOV, width, height, z_near, z_far, window, position, direction)
{}