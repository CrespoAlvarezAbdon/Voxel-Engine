#include "camera.h"

#include "player.h"

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
        projectionMatrix_(glm::perspective(glm::radians(FOV_), static_cast<float>(window.width()) / window.height(), zNear_, zFar_)),
        viewMatrix_(1.0f),
        modelMatrix_(1.0f) {

        transform_.position = position;
        transform_.rotation = rotation;

        transform_.chunkPosition.x = trunc(transform_.position.x / SCX);
        transform_.chunkPosition.y = trunc(transform_.position.y / SCY);
        transform_.chunkPosition.z = trunc(transform_.position.z / SCZ);

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