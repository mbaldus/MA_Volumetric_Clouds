#version 330

 //!< in-variables
layout(location = 0) in vec4 positionAttribute;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 passPosition;

void main()
{
	gl_Position = projection * view * model * positionAttribute;
	passPosition = model * positionAttribute;
}