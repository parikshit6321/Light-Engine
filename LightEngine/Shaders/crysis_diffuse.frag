#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform sampler2D texture_diffuse1;

void main()
{    
    color = vec4((vec3(texture(texture_diffuse1, TexCoords)) + vec3(0.5, 0.5, 0.5)), 1.0);
}