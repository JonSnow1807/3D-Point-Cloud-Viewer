#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace pcv {

class Shader {
public:
    // Constructor reads and builds the shader
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();
    
    // Use/activate the shader
    void use() const;
    
    // Utility uniform functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    
    // Get program ID
    GLuint getProgram() const { return program_; }
    
private:
    GLuint program_;
    mutable std::unordered_map<std::string, GLint> uniform_location_cache_;
    
    // Utility function for loading shader source
    std::string readFile(const std::string& filePath);
    
    // Utility function for checking shader compilation/linking errors
    void checkCompileErrors(GLuint shader, const std::string& type);
    
    // Get uniform location with caching
    GLint getUniformLocation(const std::string& name) const;
};

} // namespace pcv