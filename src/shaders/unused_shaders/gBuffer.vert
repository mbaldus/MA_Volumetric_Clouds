#version 330

//!< in-variables
layout(location=0) in vec3 positionAttribute;
layout(location=1) in vec4 color;

//!< out-variables
out vec3 passPosition;
out vec4 passColor;

void main()
{
	gl_Position = vec4(vec3(0.0), 1.0);
	passPosition = positionAttribute;
}
