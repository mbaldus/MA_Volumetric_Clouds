#version 330

 //!< in-variables
layout(location = 0) in vec4 positionAttribute;

//!< out-variables
out vec3 texCoords;

uniform mat4 view;
uniform mat4 projection;



void main()
{
	gl_Position = projection * view * positionAttribute;
	texCoords = positionAttribute.xyz;
}