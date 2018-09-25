#version 330
#define M_PI 3.1415926535897932384626433832795

in vec3 passWorldPosition;

//!< uniforms
uniform vec3 u_lightDirection;
uniform vec3 u_lightColor;
uniform float u_sunpower;
uniform float u_sunpower_mie;
uniform vec3 u_cameraPosition;
uniform vec3 u_betaR;
uniform float u_betaM;
uniform bool u_useDetails;
uniform float u_hgCoeff;
uniform float u_fraction;
uniform int u_skyVolumeEndHeight;

//!< out-variables
layout (location = 0)out vec4 fragcolor;

float henyeyGreenstein(float cos, float g)
{
	float g2 = g * g;
	return 1.f/(4.f*M_PI) * ((1.0f - g2) / pow((1.0f + g2 - 2.0f * g * cos), 1.5f));
}

float rayleigh(float cos)
{
    return 3.f/(16.f*M_PI)*(1+cos*cos);
}

void main()
{
        vec3 spherePointPos = normalize(passWorldPosition) * u_skyVolumeEndHeight;
        vec3 ray_dir = spherePointPos - u_cameraPosition;
        vec3 betaR = u_betaR * u_fraction;
        vec3 betaM = vec3(u_betaM * u_fraction);
        float s = length(ray_dir);
        //	fExtinction = exp(-(betaR + betaM)*s);

        float theta = dot(normalize(-u_lightDirection),normalize(ray_dir));
        float rayleighCoeff = rayleigh(theta);
        float mieCoeff = henyeyGreenstein(theta, u_hgCoeff);
        vec3 lInScattering = (rayleighCoeff * u_sunpower + mieCoeff*u_sunpower_mie) + u_lightColor * (1.f - exp(-(betaR + betaM)*s));
        fragcolor = vec4(lInScattering,1.0f);
}