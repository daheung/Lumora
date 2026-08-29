#version 450
#extension GL_ARB_separate_shader_object : enable

layout(location = 0) in vec3 InPosition;
layout(location = 0) out vec3 OutPosition;

void main()
{
    gl_Position = vec4(InPosition, 1.0);
    OutPosition = InPosition;
}