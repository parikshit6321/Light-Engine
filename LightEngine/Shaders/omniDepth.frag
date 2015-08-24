#version 330 core
in vec4 FragPos;

void main()
{
    // Write this as modified depth
    gl_FragDepth = gl_FragCoord.z;
}