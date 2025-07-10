#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

out vec3 FragPos;
out vec3 Color;
out vec3 Normal;

uniform mat4 view;
uniform mat4 projection;
uniform float pointSize;

void main() {
    FragPos = aPos;
    Color = aColor;
    Normal = aNormal;
    
    gl_Position = projection * view * vec4(aPos, 1.0);
    
    // Adjust point size based on distance
    vec4 viewPos = view * vec4(aPos, 1.0);
    float dist = length(viewPos.xyz);
    gl_PointSize = pointSize * (50.0 / dist);
    gl_PointSize = clamp(gl_PointSize, 1.0, 10.0);
}