#version 430 core

 //!< in-variables
layout(location = 0) in vec4 positionAttribute;
layout(location = 1) in vec3 Normal;

//strange behavior
//in int gl_VertexID;

//!< out-variables

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 positionFragment;
out vec3 normalFragment;
out vec3 lightFragment;

void main()
{
	//vec4 position = vec4(particles[gl_VertexID].position_and_radius.xyz ,1.0f);
	vec4 position = positionAttribute;
	//vec3 normal = normals[gl_VertexID].xyz;
	vec3 normal = Normal;
	mat4 MV = view * model;
    vec4 position_camcoord = MV * position;
    positionFragment = position_camcoord.xyz;
	gl_Position = projection * position_camcoord;

	// Transforming Normals in CamCoord
    mat3 NormalMatrix = mat3( transpose( inverse( MV)));
    vec3 normal_camcoord = normalize( NormalMatrix * normal);
    normalFragment = normal_camcoord;

	//TODO: Lichtquelle anpassen
	vec3 lightPosition = vec3( 0, 4, 0);

	//transformiere Lichtquellenposition zu Kamerakoordinaten
    vec3 light_camcoord = (view * vec4(lightPosition, 1.0)).xyz;
    lightFragment = light_camcoord;

	//gl_PointSize = particles[gl_VertexID].position_and_radius.w;
	//	gl_PointSize = 30;
}