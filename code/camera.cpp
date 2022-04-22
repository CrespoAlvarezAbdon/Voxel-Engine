#include <cmath>
#include "camera.h"


// 'camera' class.

camera::camera(float FOV, float zNear, float zFar, VoxelEng::window& window,
               const glm::vec3& position, const glm::vec3& direction)
    : FOV_(FOV), zNear_(zNear), zFar_(zFar), angleX_(0), angleY_(0), mouseSensibility_(0.25f), movementSpeed_(5.0f),
    mouseX_(0), mouseY_(0), oldMouseX_(0), oldMouseY_(0), 
    projectionMatrix_(glm::perspective(glm::radians(FOV_), static_cast<float>(window.width()) / window.height(), zNear_, zFar_)),
    viewMatrix_(glm::mat4(1.0f)), window_(window), position_(position), direction_(direction),
    upAxis_(glm::vec3(0.0f, 1.0f, 0.0f)), chunkMng_(nullptr)
{

    chunkRelativePosition_.x = trunc(position_.x/SCX);
    chunkRelativePosition_.y = trunc(position_.y/SCY);
    chunkRelativePosition_.z = trunc(position_.z/SCZ);

    oldChunkRelativePos_ = chunkRelativePosition_;

}

void camera::setFOV(float FOV)
{

    FOV_ = FOV;

    updateProjectionMatrix();

}

void camera::updateProjectionMatrix()
{

     projectionMatrix_ = glm::perspective(glm::radians(FOV_), static_cast<float>(window_.width()) / window_.height(), zNear_, zFar_);

}

void camera::updatePos(float timeStep) 
{

    oldChunkRelativePos_ = chunkRelativePosition_;

    // Get and compute keyboard input for movement.
    if (glfwGetKey(window_.windowAPIpointer(), GLFW_KEY_W) == GLFW_PRESS)
        position_ += direction_ * movementSpeed_ * timeStep;
    if (glfwGetKey(window_.windowAPIpointer(), GLFW_KEY_S) == GLFW_PRESS)
        position_ -= direction_ * movementSpeed_ * timeStep;
    if (glfwGetKey(window_.windowAPIpointer(), GLFW_KEY_A) == GLFW_PRESS)
        position_ -= glm::normalize(glm::cross(direction_, upAxis_)) * movementSpeed_ * timeStep;
    if (glfwGetKey(window_.windowAPIpointer(), GLFW_KEY_D) == GLFW_PRESS)
        position_ += glm::normalize(glm::cross(direction_, upAxis_)) * movementSpeed_ * timeStep;
    if (glfwGetKey(window_.windowAPIpointer(), GLFW_KEY_SPACE) == GLFW_PRESS)
        position_ += upAxis_ * movementSpeed_ * timeStep;
    if (glfwGetKey(window_.windowAPIpointer(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        position_ -= upAxis_ * movementSpeed_ * timeStep;

    // Update chunk-relative coordinates.
    chunkRelativePosition_.x = trunc(position_.x / SCX);
    chunkRelativePosition_.y = trunc(position_.y / SCY);
    chunkRelativePosition_.z = trunc(position_.z / SCZ);

    // Get and compute mouse input.
    oldMouseX_ = mouseX_;
    oldMouseY_ = mouseY_;
    glfwGetCursorPos(window_.windowAPIpointer(), &mouseX_, &mouseY_);

    angleX_ += (mouseX_ - oldMouseX_) * mouseSensibility_;
    angleY_ += (oldMouseY_ - mouseY_) * mouseSensibility_;

    if (angleY_ > 89.0f)
        angleY_ = 89.0f;
    if (angleY_ < -89.0f)
        angleY_ = -89.0f;

    // Update the camera's direction.
    direction_.x = cos(glm::radians(angleX_)) * cos(glm::radians(angleY_));
    direction_.y = sin(glm::radians(angleY_));
    direction_.z = sin(glm::radians(angleX_)) * cos(glm::radians(angleY_));

}