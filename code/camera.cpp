#include "camera.h"
#include <cmath>


namespace VoxelEng {

    // 'camera' class.

    camera* camera::playerCamera_ = nullptr;


    camera::camera(float FOV, float zNear, float zFar, window& window, bool isPlayerCamera,
        const vec3& position, const vec3& rotation)
        : window_(window),
        moveUp_(false),
        moveDown_(false),
        moveNorth_(false),
        moveSouth_(false),
        moveEast_(false),
        moveWest_(false),
        rollRight_(false),
        rollLeft_(false),
        FOV_(FOV),
        zNear_(zNear),
        zFar_(zFar),
        pitchViewDir_(0.0f),
        yawViewDir_(0.0f),
        mouseSensibility_(0.25f),
        movementSpeed_(10.0f),
        rollSpeed_(70.0f),
        mouseX_(0.0),
        mouseY_(0.0),
        oldMouseX_(0.0),
        oldMouseY_(0.0),
        projectionMatrix_(glm::perspective(glm::radians(FOV_), static_cast<float>(window.width()) / window.height(), zNear_, zFar_)),
        viewMatrix_(1.0f),
        viewDirection_(vec3FixedNorth),
        modelMatrix_(1.0f),
        upAxis_(vec3FixedUp),
        rightAxis_(vec3FixedEast),
        forwardAxis_(vec3FixedNorth),
        gravityDirection_(vec3FixedDown),
        pitchAxis_(vec3FixedEast),
        yawAxis_(vec3FixedUp),
        rollAxis_(vec3FixedNorth) {

        chunkPosition_.x = trunc(transform_.position.x / SCX);
        chunkPosition_.y = trunc(transform_.position.y / SCY);
        chunkPosition_.z = trunc(transform_.position.z / SCZ);

        oldChunkPos_ = chunkPosition_;

        if (isPlayerCamera)
            playerCamera_ = this;

    }

    void camera::setPos(float newX, float newY, float newZ) {
    
        transform_.position.x = newX;
        transform_.position.y = newY;
        transform_.position.z = newZ;
    
    }

    void camera::setChunkPos(int newCX, int newCY, int newCZ) {
    
        chunkPosition_.x = newCX;
        chunkPosition_.y = newCY;
        chunkPosition_.z = newCZ;
    
    }

    void camera::rotation(float newX, float newY, float newZ) {
    
        transform_.rotation.x = newX;
        transform_.rotation.y = newY;
        transform_.rotation.z = newZ;

    }

    void camera::updateView() {

    #if GRAPHICS_API == OPENGL

        viewMatrix_ = glm::lookAt(transform_.position, transform_.position + viewDirection_, upAxis_);

    #else



    #endif

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
        
            transform_.position += viewDirection_ * movementSpeed_ * timeStep;
            //transform_.position += forwardAxis_ * movementSpeed_ * timeStep;
            moveNorth_ = false;
        
        }
            
        if (moveSouth_) {

            transform_.position -= viewDirection_ * movementSpeed_ * timeStep;
            //transform_.position -= forwardAxis_ * movementSpeed_ * timeStep;
            moveSouth_ = false;

        }
            
        if (moveEast_) {

            transform_.position -= rightAxis_ * movementSpeed_ * timeStep;
            moveEast_ = false;

        }
            
        if (moveWest_) {

            transform_.position += rightAxis_ * movementSpeed_ * timeStep;
            moveWest_ = false;

        }

        if (moveUp_) {

            transform_.position += upAxis_ * movementSpeed_ * timeStep;
            moveUp_ = false;

        }
            
        if (moveDown_) {

            transform_.position -= upAxis_ * movementSpeed_ * timeStep;
            moveDown_ = false;

        }

        if (rollRight_) {

            //roll_ -= rollSpeed_ * timeStep;
            rollRight_ = false;

        }

        if (rollLeft_) {

            //roll_ += rollSpeed_ * timeStep;
            rollLeft_ = false;

        }

        // Update chunk-relative coordinates.
        chunkPosition_.x = trunc(transform_.position.x / SCX);
        chunkPosition_.y = trunc(transform_.position.y / SCY);
        chunkPosition_.z = trunc(transform_.position.z / SCZ);

        // Get and compute mouse input.
        oldMouseX_ = mouseX_;
        oldMouseY_ = mouseY_;

        #if GRAPHICS_API == OPENGL

            glfwGetCursorPos(window_.windowAPIpointer(), &mouseX_, &mouseY_);

        #else

        #endif

        pitchViewDir_ += (oldMouseY_ - mouseY_) * mouseSensibility_;
        yawViewDir_ += (oldMouseX_ - mouseX_) * mouseSensibility_;

        if (pitchViewDir_ > 89.0f)
            pitchViewDir_ = 89.0f;
        else if (pitchViewDir_ < -89.0f)
            pitchViewDir_ = -89.0f;

        pitchAxis_ = vec3FixedEast;
        yawAxis_ = vec3FixedUp;
        rollAxis_ = vec3FixedNorth;

        upAxis_ = -glm::normalize(gravityDirection_);
        forwardAxis_ = vec3FixedNorth;

        // Obtain the angles needed to rotate the constant vector vec3FixedUp to the current value of upAxis_.
        transform_.rotation.x = glm::degrees(glm::acos(glm::dot(upAxis_, vec3FixedNorth))) - 90.0f;
        transform_.rotation.y = glm::degrees(glm::acos(glm::dot(upAxis_, vec3FixedUp)));
        transform_.rotation.z = glm::degrees(glm::acos(glm::dot(upAxis_, vec3FixedEast))) - 90.0f;

        pitchAxis_ = glm::rotate(pitchAxis_, glm::radians(transform_.rotation.y), rollAxis_);
        yawAxis_ = glm::rotate(yawAxis_, glm::radians(transform_.rotation.y), rollAxis_);

        yawAxis_ = glm::rotate(yawAxis_, glm::radians(transform_.rotation.x), pitchAxis_);
        rollAxis_ = glm::rotate(rollAxis_, glm::radians(transform_.rotation.x), pitchAxis_);

        pitchAxis_ = glm::rotate(pitchAxis_, glm::radians(transform_.rotation.z), yawAxis_);
        rollAxis_ = glm::rotate(rollAxis_, glm::radians(transform_.rotation.z), yawAxis_);

        if (upAxis_ != vec3FixedUp && upAxis_ != vec3FixedDown)
            forwardAxis_ = glm::cross(yawAxis_, upAxis_);
        else // Particular case when glm::cross will return (0,0,0)
            forwardAxis_ = glm::rotate(forwardAxis_, glm::radians(transform_.rotation.y), rollAxis_);

        viewDirection_ = quaternion::rotateVector(forwardAxis_, yawViewDir_, upAxis_);
        rightAxis_ = glm::cross(viewDirection_, upAxis_);
        viewDirection_ = quaternion::rotateVector(viewDirection_, pitchViewDir_, rightAxis_);

        forwardAxis_ = glm::normalize(forwardAxis_);
        rightAxis_ = glm::normalize(rightAxis_);
        viewDirection_ = glm::normalize(viewDirection_);

    }

}