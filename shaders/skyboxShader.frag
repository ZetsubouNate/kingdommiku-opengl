#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skyboxDay;
uniform samplerCube skyboxNight;
uniform float blendFactor;

void main()
{
    vec4 dayColor = texture(skyboxDay, textureCoordinates);
    vec4 nightColor = texture(skyboxNight, textureCoordinates);
    
    color = mix(nightColor, dayColor, blendFactor);
}
