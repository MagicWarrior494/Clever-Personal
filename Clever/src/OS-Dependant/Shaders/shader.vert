#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 viewproj;
} ubo;

layout(push_constant) uniform constants
{
    mat4 model;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.viewproj * pushConstants.model * vec4(inPosition, 1.0);
    fragColor = inColor;
}