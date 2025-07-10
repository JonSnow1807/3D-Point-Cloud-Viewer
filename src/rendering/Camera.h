#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace pcv {

class Camera {
public:
    enum Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };
    
    // Constructor
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = 0.0f);
    
    // Matrices
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const { return projection_; }
    
    // Position and orientation
    const glm::vec3& getPosition() const { return position_; }
    const glm::vec3& getFront() const { return front_; }
    const glm::vec3& getUp() const { return up_; }
    const glm::vec3& getRight() const { return right_; }
    
    // Movement
    void processKeyboard(Movement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);
    
    // Projection settings
    void setPerspective(float fov, float aspect, float near, float far);
    void setOrthographic(float left, float right, float bottom, float top, float near, float far);
    
    // Speed settings
    void setMovementSpeed(float speed) { movement_speed_ = speed; }
    void setMouseSensitivity(float sensitivity) { mouse_sensitivity_ = sensitivity; }
    
    // Zoom
    float getZoom() const { return zoom_; }
    
private:
    // Camera attributes
    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 world_up_;
    
    // Euler angles
    float yaw_;
    float pitch_;
    
    // Camera options
    float movement_speed_ = 2.5f;
    float mouse_sensitivity_ = 0.1f;
    float zoom_ = 45.0f;
    
    // Projection
    glm::mat4 projection_{1.0f};
    
    // Update camera vectors
    void updateCameraVectors();
};

} // namespace pcv