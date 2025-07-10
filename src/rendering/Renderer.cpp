#include "rendering/Renderer.h"
#include "utils/Timer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace pcv {

Renderer::Renderer(int width, int height) : width_(width), height_(height) {
}

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::initialize() {
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    
    // Set OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(point_size_);
    
    // Setup shaders
    setupShaders();
    
    return true;
}

void Renderer::shutdown() {
    // Clean up all VAOs
    for (auto& pair : vaos_) {
        glDeleteVertexArrays(1, &pair.second.vao);
        glDeleteBuffers(1, &pair.second.vbo_positions);
        glDeleteBuffers(1, &pair.second.vbo_colors);
        glDeleteBuffers(1, &pair.second.vbo_normals);
    }
    vaos_.clear();
}

void Renderer::render(const PointCloud& cloud, const Camera& camera) {
    Timer frame_timer;
    
    // Clear the screen
    glClearColor(background_color_.r, background_color_.g, background_color_.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (cloud.empty()) return;
    
    // Create or update VAO
    if (vaos_.find(&cloud) == vaos_.end()) {
        createVAO(cloud);
    }
    
    // Set up shader
    point_shader_->use();
    point_shader_->setMat4("view", camera.getViewMatrix());
    point_shader_->setMat4("projection", camera.getProjectionMatrix());
    point_shader_->setVec3("viewPos", camera.getPosition());
    point_shader_->setFloat("pointSize", point_size_);
    
    // Render
    const VAO& vao = vaos_[&cloud];
    glBindVertexArray(vao.vao);
    glDrawArrays(GL_POINTS, 0, vao.point_count);
    glBindVertexArray(0);
    
    // Update statistics
    stats_.points_rendered = vao.point_count;
    stats_.points_culled = 0;
    stats_.draw_calls = 1;
    stats_.frame_time_ms = frame_timer.elapsed();
    stats_.fps = 1000.0f / stats_.frame_time_ms;
}

void Renderer::renderWithOctree(const PointCloud& cloud, const Octree& octree, const Camera& camera) {
    Timer frame_timer;
    
    // Clear the screen
    glClearColor(background_color_.r, background_color_.g, background_color_.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (cloud.empty()) return;
    
    // Calculate frustum planes
    Octree::FrustumPlanes frustum;
    calculateFrustumPlanes(camera, frustum);
    
    // Query visible points
    std::vector<size_t> visible_indices;
    
    if (use_lod_ && use_frustum_culling_) {
        visible_indices = octree.queryLOD(camera.getPosition(), frustum);
    } else if (use_frustum_culling_) {
        visible_indices = octree.queryFrustum(frustum);
    } else {
        // Render all points
        visible_indices.resize(cloud.size());
        for (size_t i = 0; i < cloud.size(); ++i) {
            visible_indices[i] = i;
        }
    }
    
    // Update VAO with visible points
    updateVAO(cloud, visible_indices);
    
    // Set up shader
    point_shader_->use();
    point_shader_->setMat4("view", camera.getViewMatrix());
    point_shader_->setMat4("projection", camera.getProjectionMatrix());
    point_shader_->setVec3("viewPos", camera.getPosition());
    point_shader_->setFloat("pointSize", point_size_);
    
    // Render
    const VAO& vao = vaos_[&cloud];
    glBindVertexArray(vao.vao);
    glDrawArrays(GL_POINTS, 0, visible_indices.size());
    glBindVertexArray(0);
    
    // Update statistics
    stats_.points_rendered = visible_indices.size();
    stats_.points_culled = cloud.size() - visible_indices.size();
    stats_.draw_calls = 1;
    stats_.frame_time_ms = frame_timer.elapsed();
    stats_.fps = 1000.0f / stats_.frame_time_ms;
}

void Renderer::resize(int width, int height) {
    width_ = width;
    height_ = height;
    glViewport(0, 0, width, height);
}

void Renderer::createVAO(const PointCloud& cloud) {
    VAO vao;
    
    // Generate and bind VAO
    glGenVertexArrays(1, &vao.vao);
    glBindVertexArray(vao.vao);
    
    // Position buffer
    glGenBuffers(1, &vao.vbo_positions);
    glBindBuffer(GL_ARRAY_BUFFER, vao.vbo_positions);
    std::vector<glm::vec3> positions;
    positions.reserve(cloud.size());
    for (const auto& point : cloud) {
        positions.push_back(point.position);
    }
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), 
                 positions.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(0);
    
    // Color buffer
    glGenBuffers(1, &vao.vbo_colors);
    glBindBuffer(GL_ARRAY_BUFFER, vao.vbo_colors);
    std::vector<glm::vec3> colors;
    colors.reserve(cloud.size());
    for (const auto& point : cloud) {
        colors.push_back(point.color);
    }
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), 
                 colors.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(1);
    
    // Normal buffer
    glGenBuffers(1, &vao.vbo_normals);
    glBindBuffer(GL_ARRAY_BUFFER, vao.vbo_normals);
    std::vector<glm::vec3> normals;
    normals.reserve(cloud.size());
    for (const auto& point : cloud) {
        normals.push_back(point.normal);
    }
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), 
                 normals.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    vao.point_count = cloud.size();
    vaos_[&cloud] = vao;
}

void Renderer::updateVAO(const PointCloud& cloud, const std::vector<size_t>& indices) {
    if (vaos_.find(&cloud) == vaos_.end()) {
        createVAO(cloud);
        return;
    }
    
    VAO& vao = vaos_[&cloud];
    glBindVertexArray(vao.vao);
    
    // Update position buffer
    std::vector<glm::vec3> positions;
    positions.reserve(indices.size());
    for (size_t idx : indices) {
        positions.push_back(cloud[idx].position);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vao.vbo_positions);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), 
                 positions.data(), GL_DYNAMIC_DRAW);
    
    // Update color buffer
    std::vector<glm::vec3> colors;
    colors.reserve(indices.size());
    for (size_t idx : indices) {
        colors.push_back(cloud[idx].color);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vao.vbo_colors);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), 
                 colors.data(), GL_DYNAMIC_DRAW);
    
    // Update normal buffer
    std::vector<glm::vec3> normals;
    normals.reserve(indices.size());
    for (size_t idx : indices) {
        normals.push_back(cloud[idx].normal);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vao.vbo_normals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), 
                 normals.data(), GL_DYNAMIC_DRAW);
    
    glBindVertexArray(0);
}

void Renderer::setupShaders() {
    point_shader_ = std::make_unique<Shader>("shaders/point.vert", "shaders/point.frag");
}

void Renderer::calculateFrustumPlanes(const Camera& camera, Octree::FrustumPlanes& planes) {
    glm::mat4 vp = camera.getProjectionMatrix() * camera.getViewMatrix();
    
    // Extract frustum planes from view-projection matrix
    // Left plane
    planes[0] = glm::vec4(
        vp[0][3] + vp[0][0],
        vp[1][3] + vp[1][0],
        vp[2][3] + vp[2][0],
        vp[3][3] + vp[3][0]
    );
    
    // Right plane
    planes[1] = glm::vec4(
        vp[0][3] - vp[0][0],
        vp[1][3] - vp[1][0],
        vp[2][3] - vp[2][0],
        vp[3][3] - vp[3][0]
    );
    
    // Bottom plane
    planes[2] = glm::vec4(
        vp[0][3] + vp[0][1],
        vp[1][3] + vp[1][1],
        vp[2][3] + vp[2][1],
        vp[3][3] + vp[3][1]
    );
    
    // Top plane
    planes[3] = glm::vec4(
        vp[0][3] - vp[0][1],
        vp[1][3] - vp[1][1],
        vp[2][3] - vp[2][1],
        vp[3][3] - vp[3][1]
    );
    
    // Near plane
    planes[4] = glm::vec4(
        vp[0][3] + vp[0][2],
        vp[1][3] + vp[1][2],
        vp[2][3] + vp[2][2],
        vp[3][3] + vp[3][2]
    );
    
    // Far plane
    planes[5] = glm::vec4(
        vp[0][3] - vp[0][2],
        vp[1][3] - vp[1][2],
        vp[2][3] - vp[2][2],
        vp[3][3] - vp[3][2]
    );
    
    // Normalize planes
    for (auto& plane : planes) {
        float length = glm::length(glm::vec3(plane));
        plane /= length;
    }
}

} // namespace pcv