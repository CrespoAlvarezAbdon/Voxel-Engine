#include "entity.h"

player::player(float FOV, float width, float height, float zNear, float zFar, GLFWwindow* window, const glm::vec3& position, const glm::vec3& direction)
	: camera_(FOV, width, height, zNear, zFar, window, position, direction)
{}