#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoord;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 TangentCameraDir;
    vec4 FragPosLightSpaceDirec;
    vec4 FragPosLightSpacePoint;
} vs_out;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 cameraDir;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 direcLightSpaceMatrix;
uniform mat4 pointLightSpaceMatrix;

void main()
{
    gl_Position = projection * view *  model * vec4(position, 1.0f);
    vs_out.FragPos = vec3(model * vec4(position, 1.0f));
    vs_out.TexCoord = texCoord;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 B = normalize(normalMatrix * bitangent);
    vec3 N = normalize(normalMatrix * normal);

    mat3 TBN = transpose(mat3(T, B, N));
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
    vs_out.TangentCameraDir = TBN * cameraDir;

    vs_out.FragPosLightSpaceDirec = direcLightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    vs_out.FragPosLightSpacePoint = pointLightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
}