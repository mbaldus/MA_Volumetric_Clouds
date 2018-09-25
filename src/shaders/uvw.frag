#version 330

in vec3 passUVWCoord;

//!< out-variables
layout (location = 1)out vec4 backUVWtexture;
layout (location = 2)out vec4 frontUVWtexture;


void main()
 {
 	if(gl_FrontFacing) // front face
 	{
 		frontUVWtexture = vec4(passUVWCoord, gl_FragCoord.z); // alpha contains fragment depth
 		backUVWtexture  = vec4(0);
 	}
 	else // back face
 	{
 		backUVWtexture = vec4(passUVWCoord, gl_FragCoord.z); // alpha contains fragment depth
 		frontUVWtexture  = vec4(0);
 	}
 }