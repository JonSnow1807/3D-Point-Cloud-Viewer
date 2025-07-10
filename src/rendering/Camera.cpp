#include "rendering/Camera.h"
#include <algorithm>

namespace pcv {

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : position_(position), world_up_(up), yaw_(yaw), pitch_(pitch) {
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
}

void Camera::processKeyboard(Movement direction, float deltaTime) {
    float velocity = movement_speed_ * deltaTime;
    
    switch (direction) {
        case FORWARD:
            position_ += front_ * velocity;
            break;
        case BACKWARD:
            position_ -= front_ * velocity;
            break;
        case LEFT:
            position_ -= right_ * velocity;
            break;
        case RIGHT:
            position_ += right_ * velocity;
            break;
        case UP:
            position_ += up_ * velocity;
            break;
        case DOWN:
            position_ -= up_ * velocity;
            break;
    }
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouse_sensitivity_;
    yoffset *= mouse_sensitivity_;
    
    yaw_ += xoffset;
    pitch_ += yoffset;
    
    // Constrain pitch to avoid screen flip
    if (constrainPitch) {
        pitch_ = std::clamp(pitch_, -89.0f, 89.0f);
    }
    
    updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset) {
    zoom_ -= yoffset;
    zoom_ = std::clamp(zoom_, 1.0f, 90.0f);
    
    // Update projection if using perspective
    setPerspective(zoom_, 1.0f, 0.1f, 100.0f); // Aspect ratio will be updated by resize
}

void Camera::setPerspective(float fov, float aspect, float near, float far) {
    projection_ = glm::perspective(glm::radians(fov), aspect, near, far);
    zoom_ = fov;
}

void Camera::setOrthographic(float left, float right, float bottom, float top, float near, float far) {
    projection_ = glm::ortho(left, right, bottom, top, near, far);
}

void Camera::updateCameraVectors() {
    // Calculate new front vector
    glm::vec3 front;
    front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front.y = sin(glm::radians(pitch_));
    front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front_ = glm::normalize(front);
    
    // Recalculate right and up vectors
    right_ = glm::normalize(glm::cross(front_, world_up_));
    up_ = glm::normalize(glm::cross(right_, front_));
}

} // namespace pcv