#include "camera.h"
#include <cmath>


namespace VoxelEng {

    // 'camera' class.

    camera* camera::playerCamera_ = nullptr;


    camera::camera(float FOV, float zNear, float zFar, window& window, bool isPlayerCamera,
        const vec3& position, const vec3& rotation)
        : window_(window),
        FOV_(FOV),
        zNear_(zNear),
        zFar_(zFar),
        pitchViewDir_(0.0f),
        yawViewDir_(0.0f),
        mouseSensibility_(0.25f),
        mouseX_(0.0),
        mouseY_(0.0),
        oldMouseX_(0.0),
        oldMouseY_(0.0),
        projectionMatrix_(glm::perspective(glm::radians(FOV_), static_cast<float>(window.width()) / window.height(), zNear_, zFar_)),
        viewMatrix_(1.0f),
        modelMatrix_(1.0f),
        gravityDirection_(vec3FixedDown),
        pitchAxis_(vec3FixedEast),
        yawAxis_(vec3FixedUp),
        rollAxis_(vec3FixedNorth) {

        transform_.chunkPosition.x = trunc(transform_.position.x / SCX);
        transform_.chunkPosition.y = trunc(transform_.position.y / SCY);
        transform_.chunkPosition.z = trunc(transform_.position.z / SCZ);

        oldChunkPos_ = transform_.chunkPosition;

        if (isPlayerCamera)
            playerCamera_ = this;

    }

    void camera::setPos(float newX, float newY, float newZ) {
    
        transform_.position.x = newX;
        transform_.position.y = newY;
        transform_.position.z = newZ;
    
    }

    void camera::setChunkPos(int newCX, int newCY, int newCZ) {
    
        transform_.chunkPosition.x = newCX;
        transform_.chunkPosition.y = newCY;
        transform_.chunkPosition.z = newCZ;
    
    }

    void camera::rotation(float newX, float newY, float newZ) {
    
        transform_.rotation.x = newX;
        transform_.rotation.y = newY;
        transform_.rotation.z = newZ;

    }

    void camera::updatePos() {

        // NEXT. TERMINAR DE TRASPASAR EL MOVIMIENTO DEL JUGADOR A LA CLASE PLAYER Y DE QUE CAMERA SIMPLEMENTE COJA EL TRANSFORM DEL PLAYER
        // PARA USAR ESE TRANSFORM DE BASE (POR EJEMPLO, SI SE QUIERE METER UN MODO TERCERA PERSONA SERÍA SIMPLEMENTE HACER RETROCEDER LA CÁMARA
        // A PARTIR DEL TRANSFORM DEL PLAYER.

        oldChunkPos_ = transform_.chunkPosition; 

        //transform_ = player::getTransform();

        // Update chunk-relative coordinates.
        transform_.chunkPosition.x = trunc(transform_.position.x / SCX);
        transform_.chunkPosition.y = trunc(transform_.position.y / SCY);
        transform_.chunkPosition.z = trunc(transform_.position.z / SCZ);

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

        transform_.Yaxis = -glm::normalize(gravityDirection_);
        transform_.Xaxis = vec3FixedNorth;

        // Obtain the angles needed to rotate the constant vector vec3FixedUp to the current value of upAxis_.
        transform_.rotation.x = glm::degrees(glm::acos(glm::dot(transform_.Yaxis, vec3FixedNorth))) - 90.0f;
        transform_.rotation.y = glm::degrees(glm::acos(glm::dot(transform_.Yaxis, vec3FixedUp)));
        transform_.rotation.z = glm::degrees(glm::acos(glm::dot(transform_.Yaxis, vec3FixedEast))) - 90.0f;

        pitchAxis_ = glm::rotate(pitchAxis_, glm::radians(transform_.rotation.y), rollAxis_);
        yawAxis_ = glm::rotate(yawAxis_, glm::radians(transform_.rotation.y), rollAxis_);

        yawAxis_ = glm::rotate(yawAxis_, glm::radians(transform_.rotation.x), pitchAxis_);
        rollAxis_ = glm::rotate(rollAxis_, glm::radians(transform_.rotation.x), pitchAxis_);

        pitchAxis_ = glm::rotate(pitchAxis_, glm::radians(transform_.rotation.z), yawAxis_);
        rollAxis_ = glm::rotate(rollAxis_, glm::radians(transform_.rotation.z), yawAxis_);

        if (transform_.Yaxis != vec3FixedUp && transform_.Yaxis != vec3FixedDown)
            transform_.Xaxis = glm::cross(yawAxis_, transform_.Yaxis);
        else // Particular case when glm::cross will return (0,0,0)
            transform_.Xaxis = glm::rotate(transform_.Xaxis, glm::radians(transform_.rotation.y), rollAxis_);

        transform_.viewDirection = quaternion::rotateVector(transform_.Xaxis, yawViewDir_, transform_.Yaxis);
        transform_.Zaxis = glm::cross(transform_.viewDirection, transform_.Yaxis);
        transform_.viewDirection = quaternion::rotateVector(transform_.viewDirection, pitchViewDir_, transform_.Zaxis);

        transform_.Xaxis = glm::normalize(transform_.Xaxis);
        transform_.Zaxis = glm::normalize(transform_.Zaxis);
        transform_.viewDirection = glm::normalize(transform_.viewDirection);

    }

    void camera::updateView() {

    #if GRAPHICS_API == OPENGL

        viewMatrix_ = glm::lookAt(transform_.position, transform_.position + transform_.viewDirection, transform_.Yaxis);

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

}