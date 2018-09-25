#version 330

//!< in-variables
in vec2 passUV;

uniform sampler2D outputTexture;

//!< out-variables
out vec4 fragcolor;


void main()
{
    fragcolor = texture(outputTexture, passUV);
}