#version 330 core

out vec3 color;

uniform int pointLightOn;

void main()
{
     if(pointLightOn == 1)
         color = vec4(1.0, 1.0, 1.0, 1.0);
     else
         color = vec4(0.02, 0.02, 0.02, 0.5);
}