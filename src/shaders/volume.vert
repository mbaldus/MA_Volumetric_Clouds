#version 330

 //!< in-variables
layout(location = 0) in vec4 positionAttribute;
layout(location = 1) in vec3 normalAttribute;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec3 uvwCoords;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//!< out-variables

out vec3 passWorldPosition;
out vec4 passPosition;
out vec3 passUVWCoord;
out vec2 passImagePos;
// TODO:
//out vec3 passNormal;
//out vec3 passWorldNormal;


void main()
{
	gl_Position = projection * view * model * positionAttribute;

	passWorldPosition = (model * positionAttribute).xyz;
	passPosition = view * model * positionAttribute;
    passImagePos = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
	passUVWCoord = uvwCoords;
}