#version 330

 //!< in-variables
in vec2 positionAttribute;

//!< out-variables
out vec2 passUV;


void main()
{
	gl_Position = vec4(positionAttribute, 0.0f,1.0f);
	passUV = (positionAttribute+1)/2;
}