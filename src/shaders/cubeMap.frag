#version 330

//!< in-variables
in vec3 texCoords;

//!< uniforms
uniform samplerCube skybox;
//!< out-variables
layout(location = 0)out vec4 fragcolor;

void main()
{
    fragcolor = texture(skybox, texCoords);
}