#version 450
#extension GL_ARB_separate_shader_object : enable

layout(location = 0) in vec3 InPosition;

layout(location = 0) out vec4 OutColour;

void main()
{
    OutColour = vec4(
        InPosition.r + 0.5, 
        InPosition.g + 0.5, 
        InPosition.b + 0.5, 
        1.0
    );
}