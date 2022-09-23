#include "camera.h"
#include <cmath>


namespace VoxelEng {

    // 'camera' class.

    camera* camera::playerCamera_ = nullptr;


    camera::camera(float FOV, float zNear, float zFar, window& window, bool isPlayerCamera,
        const vec3& position, const vec3& direction)
        : window_(window), moveUp_(false), moveDown_(false), moveNorth_(false), moveSouth_(false), moveEast_(false), moveWest_(false),
        FOV_(FOV), zNear_(zNear), zFar_(zFar), angleX_(0), angleY_(0), mouseSensibility_(0.25f), movementSpeed_(5.0f),
        mouseX_(0), mouseY_(0), oldMouseX_(0), oldMouseY_(0),
        projectionMatrix_(glm::perspective(glm::radians(FOV_), static_cast<float>(window.width()) / window.height(), zNear_, zFar_)),
        viewMatrix_(glm::mat4(1.0f)), position_(position), direction_(direction),
        modelMatrix_(glm::mat4(1.0f)), upAxis_(vec3(0.0f, 1.0f, 0.0f)) {

        chunkPosition_.x = trunc(position_.x / SCX);
        chunkPosition_.y = trunc(position_.y / SCY);
        chunkPosition_.z = trunc(position_.z / SCZ);

        oldChunkPos_ = chunkPosition_;

        if (isPlayerCamera)
            playerCamera_ = this;

    }

    void camera::setPos(int newX, int newY, int newZ) {
    
        position_.x = newX;
        position_.y = newY;
        position_.z = newZ;
    
    }

    void camera::setChunkPos(int newCX, int newCY, int newCZ) {
    
        chunkPosition_.x = newCX;
        chunkPosition_.y = newCY;
        chunkPosition_.z = newCZ;
    
    }

    void camera::setDirection(int newDX, int newDY, int newDZ) {
    
        direction_.x = newDX;
        direction_.y = newDY;
        direction_.z = newDZ;
    
    }

    void camera::setFOV(float FOV) {

        FOV_ = FOV;

        updateProjectionMatrix();

    }

    void camera::updateProjectionMatrix() {

        #if GRAPHICS_API == OPENGL

            projectionMatrix_ = glm::perspective(glm::radians(FOV_), static_cast<float>(window_.width()) / window_.height(), zNear_, zFar_);

        #else

        #endif

    }

    void camera::updatePos(float timeStep) {

        oldChunkPos_ = chunkPosition_;

        // Camera's movement.
        if (moveNorth_) {
        
            position_ += direction_ * movementSpeed_ * timeStep;
            moveNorth_ = false;
        
        }
            
        if (moveSouth_) {

            position_ -= direction_ * movementSpeed_ * timeStep;
            moveSouth_ = false;

        }
            
        if (moveEast_) {

            position_ -= glm::normalize(glm::cross(direction_, upAxis_)) * movementSpeed_ * timeStep;
            moveEast_ = false;

        }
            
        if (moveWest_) {

            position_ += glm::normalize(glm::cross(direction_, upAxis_)) * movementSpeed_ * timeStep;
            moveWest_ = false;

        }
            
        if (moveUp_) {

            position_ += upAxis_ * movementSpeed_ * timeStep;
            moveUp_ = false;

        }
            
        if (moveDown_) {

            position_ -= upAxis_ * movementSpeed_ * timeStep;
            moveDown_ = false;

        }

        // Update chunk-relative coordinates.
        chunkPosition_.x = trunc(position_.x / SCX);
        chunkPosition_.y = trunc(position_.y / SCY);
        chunkPosition_.z = trunc(position_.z / SCZ);

        // Get and compute mouse input.
        oldMouseX_ = mouseX_;
        oldMouseY_ = mouseY_;

        #if GRAPHICS_API == OPENGL

            glfwGetCursorPos(window_.windowAPIpointer(), &mouseX_, &mouseY_);

        #else

        #endif

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

}