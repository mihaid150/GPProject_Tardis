#version 410 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 model;
uniform mat4 lightSpaceMatrix; // Transformation matrix to light space

void main() {
    gl_Position = lightSpaceMatrix * model * vec4(vertexPosition, 1.0);
}
