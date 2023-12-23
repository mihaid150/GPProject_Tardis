#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = glm::normalize(cameraUp);
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        this->worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        switch (direction)
        {
        case gps::MOVE_FORWARD:
            cameraPosition += speed * cameraFrontDirection;
            cameraTarget += speed * cameraFrontDirection;
            break;
        case gps::MOVE_BACKWARD:
            cameraPosition -= speed * cameraFrontDirection;
            cameraTarget -= speed * cameraFrontDirection;
            break;
        case gps::MOVE_RIGHT:
            cameraPosition += speed * cameraRightDirection;
            cameraTarget += speed * cameraRightDirection;
            break;
        case gps::MOVE_LEFT:
            cameraPosition -= speed * cameraRightDirection;
            cameraTarget -= speed * cameraRightDirection;
            break;
        default:
            break;
        }
    }
    //update the camera internal parameters following a camera rotate event
        //yaw - camera rotation around the y axis
        //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        this->yaw += yaw;
        this->pitch += pitch;

        if (this->pitch > 89.0f)
            this->pitch = 89.0f;
        if (this->pitch < -89.0f)
            this->pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        direction.y = sin(glm::radians(this->pitch));
        direction.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        this->cameraFrontDirection = glm::normalize(direction);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->worldUp));
        this->cameraUpDirection = glm::normalize(glm::cross(this->cameraRightDirection, this->cameraFrontDirection));
        this->cameraTarget = this->cameraPosition + this->cameraFrontDirection;

    }
}