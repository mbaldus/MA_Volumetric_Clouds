#version 330

 //!< in-variables
layout(location = 0) in vec4 positionAttribute;
layout(location = 1) in vec3 normalAttribute;
layout(location = 2) in vec2 texCoords;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//!< out-variables
out vec4 passPosition;
out vec3 passUVW;


void main()
{
	gl_Position = projection * view * model * positionAttribute;
	passPosition = view * model * positionAttribute;
	passWorldPosition = model.positionAttribute;
	passUVW = (view*positionAttribute).xyz; //sind die Vertexpositions = texturkoordinaten?

}