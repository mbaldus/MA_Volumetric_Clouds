#version 330

//!< uniforms
uniform vec4 color;
//!< out-variables
layout (location = 0)out vec4 fragcolor;

in vec4 passPosition;
void main()
{
        fragcolor = vec4(1.0f,0,0,1.f) * passPosition.y;
}