#version 330 core

in vec2 TexCoord;

out vec4 color;

uniform sampler2D particleTexture;
uniform float visibility;

void main()
{
     vec3 texColor = vec3(texture(particleTexture, TexCoord));
     if(texColor.r < 0.1)
          discard;
     color = vec4(texColor, visibility);
}