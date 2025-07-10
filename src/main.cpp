#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <chrono>
#include <cmath>

#include "core/PointCloud.h"
#include "core/Octree.h"
#include "rendering/Renderer.h"
#include "rendering/Camera.h"
#include "processing/OutlierRemoval.h"
#include "processing/VoxelDownsampling.h"
#include "utils/Timer.h"

using namespace pcv;

// Global variables for callbacks
Camera* g_camera = nullptr;
float g_lastX = 400, g_lastY = 300;
bool g_firstMouse = true;
bool g_mouseCaptured = false;

// Window dimensions
const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window, float deltaTime);

// Generate sample point cloud
PointCloud::Ptr generateSamplePointCloud(size_t num_points = 100000) {  // Changed from 1000000
    auto cloud = std::make_shared<PointCloud>();
    cloud->reserve(num_points);
    
    // Generate a Stanford Bunny-like shape (simplified)
    for (size_t i = 0; i < num_points; ++i) {
        float t = static_cast<float>(i) / num_points * 2.0f * M_PI * 10.0f;
        float r = 2.0f + 0.5f * std::sin(5.0f * t);
        float height = 3.0f * static_cast<float>(i) / num_points;
        
        glm::vec3 pos(
            r * std::cos(t) + (rand() / float(RAND_MAX) - 0.5f) * 0.1f,
            height + (rand() / float(RAND_MAX) - 0.5f) * 0.1f,
            r * std::sin(t) + (rand() / float(RAND_MAX) - 0.5f) * 0.1f
        );
        
        // Color based on height
        glm::vec3 color(
            height / 3.0f,
            1.0f - height / 3.0f,
            0.5f
        );
        
        cloud->addPoint(pos, color);
    }
    
    return cloud;
}

int main(int argc, char* argv[]) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 
                                          "3D Point Cloud Viewer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    // Create camera
    Camera camera(glm::vec3(0.0f, 5.0f, 10.0f));
    camera.setPerspective(45.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
    g_camera = &camera;
    
    // Create renderer
    Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!renderer.initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return -1;
    }
    
    // Load or generate point cloud
    PointCloud::Ptr cloud;
    if (argc > 1) {
        cloud = std::make_shared<PointCloud>();
        if (!cloud->loadFromFile(argv[1])) {
            std::cerr << "Failed to load point cloud from: " << argv[1] << std::endl;
            std::cerr << "Generating sample point cloud instead..." << std::endl;
            cloud = generateSamplePointCloud();
        }
    } else {
        std::cout << "No point cloud file specified. Generating sample point cloud..." << std::endl;
        cloud = generateSamplePointCloud();
    }
    
    std::cout << "Point cloud loaded: " << cloud->size() << " points" << std::endl;
    std::cout << "Memory usage: " << cloud->getMemoryUsage() / (1024.0 * 1024.0) << " MB" << std::endl;
    
    // Apply filters
    std::cout << "Applying filters..." << std::endl;
    Timer filter_timer;
    
    // Voxel downsampling
    VoxelDownsampling::Parameters voxel_params;
    voxel_params.leaf_size = 0.05f;
    auto stats = VoxelDownsampling::getStatistics(*cloud, voxel_params);
    std::cout << "Voxel downsampling would reduce from " << stats.original_points 
              << " to " << stats.downsampled_points << " points" << std::endl;
    
    // Outlier removal - COMMENTED OUT FOR PERFORMANCE
    // This is O(n^2) and very slow for large point clouds
    /*
    OutlierRemoval::StatisticalParams outlier_params;
    outlier_params.k_neighbors = 50;
    outlier_params.std_multiplier = 1.0f;
    auto outliers = OutlierRemoval::findStatisticalOutliers(*cloud, outlier_params);
    std::cout << "Found " << outliers.size() << " outliers" << std::endl;
    */
    
    std::cout << "Filters processed in " << filter_timer.elapsed() << " ms" << std::endl;
    
    // Build octree
    std::cout << "Building octree..." << std::endl;
    Timer octree_timer;
    Octree octree(*cloud);
    octree.build();
    std::cout << "Octree built in " << octree_timer.elapsed() << " ms" << std::endl;
    std::cout << "Max depth: " << octree.getMaxDepth() << std::endl;
    
    // Center the point cloud
    cloud->translateCentroid(glm::vec3(0.0f));
    
    // Timing variables
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    
    // Rendering options
    bool use_octree = true;
    bool show_stats = true;
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Process input
        processInput(window, deltaTime);
        
        // Render
        if (use_octree) {
            renderer.renderWithOctree(*cloud, octree, camera);
        } else {
            renderer.render(*cloud, camera);
        }
        
        // Display statistics
        if (show_stats) {
            const auto& stats = renderer.getStatistics();
            glfwSetWindowTitle(window, 
                ("3D Point Cloud Viewer - FPS: " + std::to_string(static_cast<int>(stats.fps)) +
                 " | Points: " + std::to_string(stats.points_rendered) + "/" + std::to_string(cloud->size()) +
                 " | Frame: " + std::to_string(stats.frame_time_ms) + "ms").c_str());
        }
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (g_camera) {
        g_camera->setPerspective(g_camera->getZoom(), 
                                (float)width / height, 0.1f, 100.0f);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!g_mouseCaptured) return;
    
    if (g_firstMouse) {
        g_lastX = xpos;
        g_lastY = ypos;
        g_firstMouse = false;
    }
    
    float xoffset = xpos - g_lastX;
    float yoffset = g_lastY - ypos; // Reversed since y-coordinates go from bottom to top
    
    g_lastX = xpos;
    g_lastY = ypos;
    
    if (g_camera) {
        g_camera->processMouseMovement(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (g_camera) {
        g_camera->processMouseScroll(yoffset);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_SPACE:
                g_mouseCaptured = !g_mouseCaptured;
                glfwSetInputMode(window, GLFW_CURSOR, 
                                g_mouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
                g_firstMouse = true;
                break;
        }
    }
}

void processInput(GLFWwindow* window, float deltaTime) {
    if (!g_camera) return;
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        g_camera->processKeyboard(Camera::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        g_camera->processKeyboard(Camera::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        g_camera->processKeyboard(Camera::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        g_camera->processKeyboard(Camera::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        g_camera->processKeyboard(Camera::DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        g_camera->processKeyboard(Camera::UP, deltaTime);
}