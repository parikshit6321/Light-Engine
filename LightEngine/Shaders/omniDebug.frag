#version 330 core

in vec3 TexCoords;

out vec4 color;

uniform samplerCube skybox;
uniform vec3 lightPos;
uniform float far_plane;

void main()
{
     // Get vector between fragment position and light position
     vec3 fragToLight = TexCoords - lightPos;

     // Use the light to fragment vector to sample from the depth map
     float closestDepth = texture(skybox, fragToLight).r;

     color = vec4(vec3(closestDepth / far_plane), 1.0);
}