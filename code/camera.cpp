#include "camera.h"
#include <cmath>


namespace VoxelEng {

    // 'camera' class.

    camera* camera::playerCamera_ = nullptr;


    camera::camera(float FOV, float zNear, float zFar, window& window, bool isPlayerCamera,
        const vec3& position, const vec3& rotation)
        : window_(window), moveUp_(false), moveDown_(false), moveNorth_(false), moveSouth_(false), moveEast_(false), moveWest_(false),
        rollRight_(false), rollLeft_(false),
        FOV_(FOV), zNear_(zNear), zFar_(zFar), pitchViewDir_(0.0f), yawViewDir_(0.0f), pitch_(0.0f), yaw_(0.0f), roll_(0.0f), mouseSensibility_(0.25f), movementSpeed_(10.0f),
        rollSpeed_(70.0f), mouseX_(0.0), mouseY_(0.0), oldMouseX_(0.0), oldMouseY_(0.0),
        projectionMatrix_(glm::perspective(glm::radians(FOV_), static_cast<float>(window.width()) / window.height(), zNear_, zFar_)),
        viewMatrix_(1.0f), position_(position), rotation_{0.0f, 0.0f, 0.0f}, viewDirection_(vec3FixedNorth),
        modelMatrix_(1.0f), upAxis_{ vec3FixedUp }, rightAxis_{ vec3FixedEast }, forwardAxis_{ vec3FixedNorth }{

        chunkPosition_.x = trunc(position_.x / SCX);
        chunkPosition_.y = trunc(position_.y / SCY);
        chunkPosition_.z = trunc(position_.z / SCZ);

        oldChunkPos_ = chunkPosition_;

        if (isPlayerCamera)
            playerCamera_ = this;

    }

    void camera::setPos(float newX, float newY, float newZ) {
    
        position_.x = newX;
        position_.y = newY;
        position_.z = newZ;
    
    }

    void camera::setChunkPos(int newCX, int newCY, int newCZ) {
    
        chunkPosition_.x = newCX;
        chunkPosition_.y = newCY;
        chunkPosition_.z = newCZ;
    
    }

    void camera::rotation(float pitch, float yaw, float roll) {
    

    }

    void camera::updateView() {

    #if GRAPHICS_API == OPENGL

        viewMatrix_ = glm::lookAt(position_, position_ + viewDirection_, upAxis_);

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
        
            position_ += viewDirection_ * movementSpeed_ * timeStep;
            //position_ += forwardAxis_ * movementSpeed_ * timeStep;
            moveNorth_ = false;
        
        }
            
        if (moveSouth_) {

            position_ -= viewDirection_ * movementSpeed_ * timeStep;
            //position_ -= forwardAxis_ * movementSpeed_ * timeStep;
            moveSouth_ = false;

        }
            
        if (moveEast_) {

            position_ -= rightAxis_ * movementSpeed_ * timeStep;
            moveEast_ = false;

        }
            
        if (moveWest_) {

            position_ += rightAxis_ * movementSpeed_ * timeStep;
            moveWest_ = false;

        }

        if (moveUp_) {

            //position_ += glm::cross(rightAxis_, viewDirection_) * movementSpeed_ * timeStep;
            position_ += upAxis_ * movementSpeed_ * timeStep;
            moveUp_ = false;

        }
            
        if (moveDown_) {

            //position_ -= glm::cross(rightAxis_, viewDirection_) * movementSpeed_ * timeStep;
            position_ -= upAxis_ * movementSpeed_ * timeStep;
            moveDown_ = false;

        }

        if (rollRight_) {

            roll_ -= rollSpeed_ * timeStep;
            rollRight_ = false;

        }

        if (rollLeft_) {

            roll_ += rollSpeed_ * timeStep;
            rollLeft_ = false;

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

        pitchViewDir_ += (oldMouseY_ - mouseY_) * mouseSensibility_;
        yawViewDir_ += (oldMouseX_ - mouseX_) * mouseSensibility_;

        if (pitchViewDir_ > 89.0f)
            pitchViewDir_ = 89.0f;
        else if (pitchViewDir_ < -89.0f)
            pitchViewDir_ = -89.0f;


        vec3 pitchAxis = vec3FixedEast,
             yawAxis = vec3FixedUp,
             rollAxis = vec3FixedNorth;

        vec3 gravityDir(1.0f, -1.0f, 1.0f);
        gravityDir = glm::normalize(gravityDir);

        upAxis_ = -gravityDir;
        forwardAxis_ = vec3FixedNorth;

        float singleX = glm::degrees(glm::acos(glm::dot(upAxis_, vec3FixedNorth))) - 90.0f,
              singleRoll = glm::degrees(glm::acos(glm::dot(upAxis_, vec3FixedUp))),
              singleZ = glm::degrees(glm::acos(glm::dot(upAxis_, vec3FixedEast))) - 90.0f;

        pitchAxis = glm::rotate(pitchAxis, glm::radians(singleRoll), rollAxis);
        yawAxis = glm::rotate(yawAxis, glm::radians(singleRoll), rollAxis);

        yawAxis = glm::rotate(yawAxis, glm::radians(singleX), pitchAxis);
        rollAxis = glm::rotate(rollAxis, glm::radians(singleX), pitchAxis);

        pitchAxis = glm::rotate(pitchAxis, glm::radians(singleZ), yawAxis);
        rollAxis = glm::rotate(rollAxis, glm::radians(singleZ), yawAxis);

        if (upAxis_ != vec3FixedUp && upAxis_ != vec3FixedDown)
            forwardAxis_ = glm::cross(yawAxis, upAxis_);
        else // Particular case when glm::cross will return (0,0,0)
            forwardAxis_ = glm::rotate(forwardAxis_, glm::radians(singleRoll), rollAxis);

        // Update the camera's direction.
        //viewDirection_.x = cos(glm::radians(pitchViewDir_)) * cos(glm::radians(yawViewDir_));
        //viewDirection_.y = sin(glm::radians(yawViewDir_));
        //viewDirection_.z = sin(glm::radians(pitchViewDir_)) * cos(glm::radians(yawViewDir_));
        
        viewDirection_ = quaternion::rotateVector(forwardAxis_, yawViewDir_, upAxis_);
        rightAxis_ = glm::cross(viewDirection_, upAxis_);
        viewDirection_ = quaternion::rotateVector(viewDirection_, pitchViewDir_, rightAxis_);
       
        

       

        //viewDirection_ = forwardAxis_;

        forwardAxis_ = glm::normalize(forwardAxis_);
        rightAxis_ = glm::normalize(rightAxis_);
        viewDirection_ = glm::normalize(viewDirection_);

        float angle = glm::degrees(glm::acos(glm::dot(forwardAxis_, upAxis_) / (glm::length(forwardAxis_) * glm::length(upAxis_))));

    }

}