#version 330 core

in vec3 FragPos;
in vec3 Color;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 viewPos;

void main() {
    // Simple shading with ambient and diffuse
    vec3 lightDir = normalize(viewPos - FragPos);
    vec3 norm = normalize(Normal);
    
    // If normal is zero, use light direction
    if (length(norm) < 0.01) {
        norm = lightDir;
    }
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * Color;
    vec3 ambient = 0.3 * Color;
    
    vec3 result = ambient + diffuse;
    
    // Circular point rendering
    vec2 coord = gl_PointCoord - vec2(0.5);
    if (length(coord) > 0.5) {
        discard;
    }
    
    FragColor = vec4(result, 1.0);
}