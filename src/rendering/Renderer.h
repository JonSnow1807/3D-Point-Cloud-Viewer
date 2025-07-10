#pragma once

#include "core/PointCloud.h"
#include "core/Octree.h"
#include "rendering/Camera.h"
#include "rendering/Shader.h"
#include <GL/glew.h>
#include <memory>
#include <unordered_map>

namespace pcv {

struct RenderStatistics {
    size_t points_rendered = 0;
    size_t points_culled = 0;
    float frame_time_ms = 0.0f;
    float fps = 0.0f;
    size_t draw_calls = 0;
};

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();
    
    // Initialization
    bool initialize();
    void shutdown();
    
    // Rendering
    void render(const PointCloud& cloud, const Camera& camera);
    void renderWithOctree(const PointCloud& cloud, const Octree& octree, const Camera& camera);
    
    // Settings
    void setPointSize(float size) { point_size_ = size; }
    void setBackgroundColor(const glm::vec3& color) { background_color_ = color; }
    void enableLOD(bool enable) { use_lod_ = enable; }
    void enableFrustumCulling(bool enable) { use_frustum_culling_ = enable; }
    
    // Window management
    void resize(int width, int height);
    
    // Statistics
    const RenderStatistics& getStatistics() const { return stats_; }
    
private:
    int width_;
    int height_;
    float point_size_ = 2.0f;
    glm::vec3 background_color_{0.1f, 0.1f, 0.1f};
    bool use_lod_ = true;
    bool use_frustum_culling_ = true;
    
    // OpenGL resources
    struct VAO {
        GLuint vao = 0;
        GLuint vbo_positions = 0;
        GLuint vbo_colors = 0;
        GLuint vbo_normals = 0;
        size_t point_count = 0;
    };
    
    std::unordered_map<const PointCloud*, VAO> vaos_;
    std::unique_ptr<Shader> point_shader_;
    
    // Statistics
    RenderStatistics stats_;
    
    // Helper functions
    void createVAO(const PointCloud& cloud);
    void updateVAO(const PointCloud& cloud, const std::vector<size_t>& indices);
    void deleteVAO(const PointCloud& cloud);
    
    void setupShaders();
    void calculateFrustumPlanes(const Camera& camera, Octree::FrustumPlanes& planes);
};

} // namespace pcv