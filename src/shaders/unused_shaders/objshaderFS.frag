#version 430

//!< in-variables
in vec3 positionFragment;
in vec3 normalFragment;
in vec3 lightFragment;

//!< out-variables
out vec4 fragcolor;


void main()
{
	//for face
	//fragcolor = passPosition;

	// b) Ambient light color
    vec3 color = vec3(0.3, 0.2, 0.2);

    // c) Diffuse light
    vec3 lightVector = normalize( lightFragment - positionFragment);
    float cos_phi = max( dot( normalFragment, lightVector), 0.0f);
    // material ist nicht angegeben, hab ich weggelassen
    color += cos_phi * vec3(1.0, 0.0, 0.0);

    // d) Specular light
    vec3 eye = normalize( -positionFragment);
    vec3 reflection = normalize( reflect( -lightVector, normalFragment));
    //shininess von 15 ist vorgegeben
    float cos_psi_n = pow( max( dot( reflection, eye), 0.0f), 1.0);
    color += cos_psi_n * vec3(1.0, 1.0, 1.0);

    fragcolor = vec4(color, 1.0);	
}