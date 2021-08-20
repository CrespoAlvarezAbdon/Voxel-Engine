#include "chunk.h"
#include "camera.h"
#include <cmath>
#include <iostream>
#include <ostream>
using namespace std;

Camera::Camera(float FOV, float width, float height, float z_near, float z_far, GLFWwindow* window, const glm::vec3& position, const glm::vec3& direction)
    : FOV_(FOV), z_far_(z_far), angle_x_(0), angle_y_(0), mouse_sensibility_(0.25f), movement_speed_(20.0f),
    mouse_x_(0), mouse_y_(0), old_mouse_x_(0), old_mouse_y_(0),
    projection_(glm::perspective(FOV_, width/height, z_near, z_far_)), view_(glm::mat4(1.0f)), window_(window),
    position_(position), direction_(direction), up_axis_(glm::vec3(0.0f, 1.0f, 0.0f))
{

    chunk_relative_position_.x = round(position_.x/SCX);
    chunk_relative_position_.y = round(position_.y/SCY);
    chunk_relative_position_.z = round(position_.z/SCZ);

    old_chunk_relative_pos_ = chunk_relative_position_;

}

void Camera::updatePos(float timeStep) 
{

    old_chunk_relative_pos_ = chunk_relative_position_;

    // Get and compute keyboard input
    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
        position_ += direction_ * movement_speed_ * timeStep;
    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
        position_ -= direction_ * movement_speed_ * timeStep;
    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
        position_ -= glm::normalize(glm::cross(direction_, up_axis_)) * movement_speed_ * timeStep;
    if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
        position_ += glm::normalize(glm::cross(direction_, up_axis_)) * movement_speed_ * timeStep;
    if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS)
        position_ += up_axis_ * movement_speed_ * timeStep;
    if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        position_ -= up_axis_ * movement_speed_ * timeStep;

    // Calculate chunk-relative coordinates
    chunk_relative_position_.x = trunc(position_.x / SCX);
    chunk_relative_position_.y = trunc(position_.y / SCY);
    chunk_relative_position_.z = trunc(position_.z / SCZ);

    // Get and compute mouse input
    old_mouse_x_ = mouse_x_;
    old_mouse_y_ = mouse_y_;
    glfwGetCursorPos(window_, &mouse_x_, &mouse_y_);

    angle_x_ += (mouse_x_ - old_mouse_x_) * mouse_sensibility_;
    angle_y_ += (old_mouse_y_ - mouse_y_) * mouse_sensibility_;

    if (angle_y_ > 89.0f)
        angle_y_ = 89.0f;
    if (angle_y_ < -89.0f)
        angle_y_ = -89.0f;

    direction_.x = cos(glm::radians(angle_x_)) * cos(glm::radians(angle_y_));
    direction_.y = sin(glm::radians(angle_y_));
    direction_.z = sin(glm::radians(angle_x_)) * cos(glm::radians(angle_y_));

}