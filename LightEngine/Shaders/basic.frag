#version 330 core

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;
uniform sampler2D pointShadowMap;

uniform int flashLight;
uniform int pointLightOn;

const float shininess = 64.0;
const float exposure = 0.1;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoord;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 TangentCameraDir;
    vec4 FragPosLightSpaceDirec;
    vec4 FragPosLightSpacePoint;
} fs_in;

out vec4 color;

float DirecShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    float bias = 0.01;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    shadow = min(0.95, shadow);

    return shadow;
}

float PointShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    float bias = 0.01;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(pointShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(pointShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    shadow = min(0.95, shadow);

    return shadow;
}

void main()
{
    // Setting up Directional Light.
    DirLight dirLight;
    dirLight.direction = vec3(1.0, -1.0, -1.0);
    dirLight.ambient = vec3(0.05, 0.05, 0.05);
    dirLight.diffuse = vec3(0.2, 0.2, 0.2);
    dirLight.specular = vec3(0.5, 0.5, 0.5);

    // Setting up Point Light.
    PointLight pointLight;
    pointLight.position = vec3(10.0, 5.0, -10.0);
    pointLight.ambient = vec3(0.0, 0.0, 0.0);
    pointLight.diffuse = vec3(10.0, 10.0, 10.0);
    pointLight.specular = vec3(10.0, 10.0, 10.0);
    pointLight.constant = 1.0;
    pointLight.linear = 0.09;
    pointLight.quadratic = 0.032;

    // Setting up Spot Light.
    SpotLight spotLight;
    spotLight.position = fs_in.TangentViewPos;
    spotLight.direction = fs_in.TangentCameraDir;
    spotLight.cutOff = 0.9763;
    spotLight.outerCutOff = 0.9;
    spotLight.constant = 1.0;
    spotLight.linear = 0.09;
    spotLight.quadratic = 0.032;
    spotLight.ambient = vec3(0.0, 0.0, 0.0);
    spotLight.diffuse = vec3(10.0, 10.0, 10.0);
    spotLight.specular = vec3(10.0, 10.0, 10.0);

    // Obtain normal from normal map in range [0,1]
    vec3 normal = texture(normalMap, fs_in.TexCoord).rgb;

    // Transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);

    // Calculate Directional Lighting.

    // Calculate shadow
    float shadow = DirecShadowCalculation(fs_in.FragPosLightSpaceDirec);

    vec3 lightDir = normalize(-dirLight.direction);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    // Combine results
    float gamma = 2.2;
    vec3 ambient = dirLight.ambient * vec3(texture(diffuseMap, fs_in.TexCoord));
    ambient = pow(ambient, vec3(gamma));
    ambient = ambient * (1.0 - shadow);
    vec3 diffuse = dirLight.diffuse * diff * vec3(texture(diffuseMap, fs_in.TexCoord));
    diffuse = pow(diffuse, vec3(gamma));
    diffuse = diffuse * (1.0 - shadow);
    vec3 specular = dirLight.specular * spec * vec3(texture(specularMap, fs_in.TexCoord));
    specular = pow(specular, vec3(gamma));
    specular = specular * (1.0 - shadow);

    vec3 result = ambient + diffuse + specular;

    // Calculate Point Lighting.

    // Calculate shadow
    float pointShadow = PointShadowCalculation(fs_in.FragPosLightSpacePoint);

    lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    
    // Diffuse shading
    diff = max(dot(normal, lightDir), 0.0);

    // Specular shading
    halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    // Attenuation
    float distance = length(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float attenuation = 1.0f / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance));

    // Combine results
    ambient = pointLight.ambient * vec3(texture(diffuseMap, fs_in.TexCoord));
    ambient = pow(ambient, vec3(gamma));

    diffuse = pointLight.diffuse * diff * vec3(texture(diffuseMap, fs_in.TexCoord));
    diffuse = pow(diffuse, vec3(gamma));

    specular = pointLight.specular * spec * vec3(texture(specularMap, fs_in.TexCoord));
    specular = pow(specular, vec3(gamma));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    ambient = ambient * (1.0 - pointShadow);
    diffuse = diffuse * (1.0 - pointShadow);
    specular = specular * (1.0 - pointShadow);

    if(pointLightOn == 1)
        result = result + (ambient + diffuse + specular);

    // Calculate Spot Lighting.
    lightDir = normalize(spotLight.position - fs_in.TangentFragPos);
    // Diffuse shading
    diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    // Attenuation
    distance = length(spotLight.position - fs_in.TangentFragPos);
    attenuation = 1.0f / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * (distance * distance));
    // Spotlight intensity
    float theta = dot(lightDir, normalize(-spotLight.direction));
    float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);
    // Combine results
    ambient = spotLight.ambient * vec3(texture(diffuseMap, fs_in.TexCoord));
    ambient = pow(ambient, vec3(gamma));
    diffuse = spotLight.diffuse * diff * vec3(texture(diffuseMap, fs_in.TexCoord));
    diffuse = pow(diffuse, vec3(gamma));
    specular = spotLight.specular * spec * vec3(texture(specularMap, fs_in.TexCoord));
    specular = pow(specular, vec3(gamma));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    result = result + (vec3(flashLight, flashLight, flashLight) * (ambient + diffuse + specular));

    // apply exposure tone-mapping
    result = vec3(1.0) - exp(-result * exposure);

    // apply gamma correction
    result.rgb = pow(result.rgb, vec3(1.0/gamma));

    color = vec4(result, 1.0);
}