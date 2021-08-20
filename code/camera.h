#ifndef _CAMERA_
#define _CAMERA_
#include <GLFW/glfw3.h>
#include <gtx/quaternion.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm.hpp>

class Camera
{

public:

	Camera(float FOV, float width, float height, float z_near, float z_far, GLFWwindow* window, const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& direction = glm::vec3(0.0f, 0.0f, 0.0f));

	void updatePos(float timeStep);
	void updateView();

	float FOV() const noexcept;
	float z_far() const noexcept;
	float mouse_sensibility() const noexcept;
	float& mouse_sensibility() noexcept;
	float movement_speed() const noexcept;
	float& movement_speed() noexcept;
	const glm::mat4& projection_matrix() const noexcept;
	glm::mat4& projection_matrix() noexcept;
	const glm::mat4& view_matrix() const noexcept;
	glm::mat4& view_matrix() noexcept;
	const glm::vec3& pos() const noexcept;
	glm::vec3& pos() noexcept;
	const glm::vec3& chunkPos() const noexcept;
	glm::vec3& chunkPos() noexcept;

private:

	float FOV_, z_far_, angle_x_, angle_y_, mouse_sensibility_, movement_speed_;
	double mouse_x_, mouse_y_, old_mouse_x_, old_mouse_y_;
	glm::mat4 projection_, view_;
	GLFWwindow* window_;
	glm::vec3 position_, direction_, up_axis_, chunk_relative_position_, old_chunk_relative_pos_;

	// old_ variables like old_mouse_x_ and old_chunk_relative_pos_ are used to store the value of, respectively, mouse_x_ and chunk_relative_position before doing any changes to them

};

inline void Camera::updateView()
{

	view_ = glm::lookAt(position_, position_ + direction_, up_axis_);

}

inline float Camera::FOV() const noexcept
{

	return FOV_;

}

inline float Camera::z_far() const noexcept
{

	return z_far_;

}

inline float Camera::mouse_sensibility() const noexcept
{

	return mouse_sensibility_;

}

inline float& Camera::mouse_sensibility() noexcept
{

	return mouse_sensibility_;

}

inline float Camera::movement_speed() const noexcept
{

	return movement_speed_;

}

inline float& Camera::movement_speed() noexcept
{

	return movement_speed_;

}

inline const glm::mat4& Camera::projection_matrix() const noexcept
{

	return projection_;

}

inline glm::mat4& Camera::projection_matrix() noexcept
{

	return projection_;

}

inline const glm::mat4& Camera::view_matrix() const noexcept
{

	return view_;

}

inline glm::mat4& Camera::view_matrix() noexcept
{

	return view_;

}

inline const glm::vec3& Camera::pos() const noexcept
{

	return position_;

}

inline glm::vec3& Camera::pos() noexcept
{

	return position_;

}

inline const glm::vec3& Camera::chunkPos() const noexcept
{

	return chunk_relative_position_;

}

inline glm::vec3& Camera::chunkPos() noexcept
{

	return chunk_relative_position_;

}

#endif