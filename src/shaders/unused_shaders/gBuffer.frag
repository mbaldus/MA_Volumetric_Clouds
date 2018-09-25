#version 330

//!< in-variables
in vec3 passPosition;
in vec4 passColor;

//!< out-variables
layout(location=0) out vec3 fragPosition;
layout(location=1) out vec4 fragColor;

void main()
{
    fragPosition = passPosition;
	fragColor = passColor;
}
