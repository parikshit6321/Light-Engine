#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;
out vec3 fragPosition;
out vec3 Normal;
out vec4 FragPosLightSpaceDirec;
out vec4 FragPosLightSpacePoint;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 direcLightSpaceMatrix;
uniform mat4 pointLightSpaceMatrix;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    fragPosition = vec3(model * vec4(position, 1.0f));
    Normal = mat3(transpose(inverse(model))) * normal;
    TexCoords = texCoords;
    FragPosLightSpaceDirec = direcLightSpaceMatrix * vec4(fragPosition, 1.0);
    FragPosLightSpacePoint = pointLightSpaceMatrix * vec4(fragPosition, 1.0);
}