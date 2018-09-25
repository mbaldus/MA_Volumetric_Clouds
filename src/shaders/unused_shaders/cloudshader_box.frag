#version 330

//!< in-variables
in vec4 passPosition;
in vec4 passWorldPosition;

//!< Camera and Scene Settings
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 u_cameraPosition;
uniform vec2 u_resolution;
uniform float u_time;

//!< Raymarching Settings
uniform sampler3D u_perlinWorleyBase;
uniform sampler3D u_erosionNoise;
uniform sampler2D u_weatherTexture;
uniform sampler2D u_curlnoise1;
uniform sampler2D u_curlnoise2;
uniform vec3 u_boundingVolumeScale;
uniform vec3 u_boundingVolumeMax;
uniform vec3 u_boundingVolumeMin;
uniform int u_marchingSteps;
uniform int u_lightMarchingSteps;
uniform vec3 u_texScale;


//!< Cloudsettings
uniform float u_coverage;
uniform float u_absorptionCloud;
uniform float u_cloudType;
uniform bool u_useCTypeFromTex;
uniform bool u_useWind;
uniform bool u_useDetails;
uniform float u_windSpeed;
uniform vec3 u_windDirection;
uniform float u_testValue;

uniform vec3 u_lightDirection;
uniform vec3 u_lightColor;
uniform bool u_doLightMarch;
uniform float u_absorptionLight;
uniform float u_hgCoeff;
uniform bool u_usePowder;
uniform bool u_useAmbient;

vec4 stratusGradient = vec4(0.02f, 0.05f, 0.1f, 0.15f);
//vec4 stratoCumulusGradient = vec4(0.72f, 0.2f, 0.3f, 0.4f);
vec4 stratoCumulusGradient =  vec4(0.22, 0.37, 0.68, 0.75);
vec4 cumulusGradient = vec4(0.01f, 0.05f, 0.8f, 1.0f);
//!< out-variables
layout (location = 0)out vec4 fragcolor;

// utility function that maps a value from one range to another.
float remap(float original_value, float original_min, float original_max, float new_min, float new_max)
{
    return new_min + (((original_value - original_min) / (original_max - original_min)) * (new_max - new_min));
}
/*
* remapt die Position in Hinsicht auf die Texturkoordinaten des Volumens
* TODO: Skalierung in unterschiedliche Richtugnen berücksichtigen für später
*/
vec3 remapToVolume(vec3 original_value, vec3 original_min, vec3 original_max, vec3 new_min, vec3 new_max)
{
    return new_min + (((original_value - original_min) / (original_max - original_min)) * (new_max - new_min));
}
/*
* Höhengradient für das Boundingvolume. Skaliert Wolkendichte über Höhe
* position: position des raysamples
*/
float height_fraction(vec3 position)
{
    float height_fraction = (position.y - u_boundingVolumeMin.y )   / (u_boundingVolumeMax.y - u_boundingVolumeMin.y);
    return clamp(height_fraction,0.f,1.f);
}

vec2 box_intersection(const vec3 p1, const vec3 p2, const vec3 start, const vec3 ray)
{
    vec3 ray_inverse = 1 / ray;
    vec3 v1 = (p1 - start) * ray_inverse;
    vec3 v2 = (p2 - start) * ray_inverse;
    vec3 n  = min(v1, v2);
    vec3 f  = max(v1, v2);
    vec2 rayInOut = vec2(max(n.x, max(n.y, n.z)), min(f.x, min(f.y, f.z)));

    if (rayInOut.x < rayInOut.y)
        return max(rayInOut, 0); //Kamera liegt innerhalb des Bounding Volumens
    else
        return vec2(0, 0);
}

vec4 getGradientFromCloudType(float u_cloudType)
{
    //interpoliert
    float stratus = 1.0f - clamp(u_cloudType * 2.0f,0.f,1.f);
	float stratocumulus = 1.0f - abs(u_cloudType - 0.5f) * 2.0f;
	float cumulus = clamp(u_cloudType - 0.5f, 0.0f,1.f) * 2.0f;
	return stratusGradient * stratus + stratoCumulusGradient * stratocumulus + cumulusGradient * cumulus;
}

float getCloudTypeHeightGradient(float cloudType, float heightGradient)
{
    vec4 mixedGradients = getGradientFromCloudType(cloudType);
    float cloudTypeGradient = smoothstep(mixedGradients.x, mixedGradients.y, heightGradient) - smoothstep(mixedGradients.z,mixedGradients.w,heightGradient);
    return cloudTypeGradient;
}
vec3 ambientLight(float heightFrac)
{
	return mix(vec3(0.68f, 0.85f, 1.f),vec3(1.0f, 1.0f, 1.0f),heightFrac);
}

float cloud_density(vec3 texture_pos, float heightGradient)
{
    vec3 original_texture_pos = texture_pos; //scaled from [0,1]
    if (u_useWind)
    {
//        texture_pos += heightGradient * vec3(0.1f,0.0f,0.1f) * u_testValue;
        texture_pos += u_windDirection * u_time * u_windSpeed;
    }

    //load densities from 3D Noise Texture
    vec4 den = texture(u_perlinWorleyBase, texture_pos);

    //load cloudtype from weathertexture
    float cloudTypeGradient = 0;
    if(u_useCTypeFromTex)
        cloudTypeGradient = getCloudTypeHeightGradient(texture(u_weatherTexture,texture_pos.xz).g * 0.5 , heightGradient);
    else
        cloudTypeGradient = getCloudTypeHeightGradient(u_cloudType, heightGradient);

    //create fbm from low frequency noises
    float low_freq_fbm = (den.g *0.625f) + (den.b * 0.25f) + (den.a * 0.125f);

    float base_cloud = remap(den.r, - low_freq_fbm, 1.0f, 0.0f, 1.0f);
    base_cloud = remap(base_cloud, 1.0 - cloudTypeGradient, 1.0f,0.0f,1.0f);

    base_cloud = remap(base_cloud, 1.0 - u_coverage, 1.0f,0.0f,1.0f);


//    float curlNoise1 = texture(u_curlnoise1,orginal_texture_pos.xz).r;
//    float curlNoise2 = texture(u_curlnoise2,orginal_texture_pos.xz).r;
////
//    texture_pos.xz += vec2(curlNoise1,curlNoise2) * (1.0f-heightGradient)* u_testValue;

    float cloudCoverage = texture(u_weatherTexture, original_texture_pos.xz).r; //use texture_pos.xz if coverage signal should move
//    cloudCoverage *= texture(u_weatherTexture, original_texture_pos.xy).r;

    //anvil deformation
//    cloudCoverage = pow(cloudCoverage, remap(1.0f-heightGradient, 0.7, 0.8, 1.0, mix(1.0f, 0.f, u_testValue)));
    cloudCoverage = clamp(cloudCoverage,0.f,1.f);
    float base_cloud_with_coverage  = remap(base_cloud, 1.0f - cloudCoverage, 1.0, 0.0, 1.0); //-1 um cloud coverage umzudrehen

 if(u_useDetails)
 {

    vec3 highFreqNoise = texture(u_erosionNoise, texture_pos*0.1f).xyz;
    float highFreqFbm = (highFreqNoise.x * 0.625f) + (highFreqNoise.y * 0.25f) + (highFreqNoise.z * 0.125f);

    float highFreqNoiseModifier = mix(highFreqFbm,  highFreqFbm, clamp(heightGradient,0.f,1.0f));
    base_cloud_with_coverage = remap(base_cloud_with_coverage,highFreqNoiseModifier * 0.2f ,1.0f,0.0f,1.0f);
 }

    base_cloud_with_coverage = clamp(base_cloud_with_coverage,0.f,1.f);
    base_cloud= clamp(base_cloud,0.f,1.f);
    return base_cloud_with_coverage;
}

//https://www.astro.umd.edu/~jph/HG_note.pdf
float henyeyGreenstein(float cos, float g)
{
	float g2 = g * g;
	return ((1.0f - g2) / pow((1.0f + g2 - 2.0f * g * cos), 1.5f))/(4.f*3.14159f);
}

vec3 computeLightMarch(vec3 samplePosition, vec3 eyeRayDirection)
{
    //Schnittpunkt von Lichtvektor mit Boundingbox ermitteln
    vec2 lightInOut = box_intersection(u_boundingVolumeMin,u_boundingVolumeMax, samplePosition, -u_lightDirection);
    float dt = (lightInOut.y-lightInOut.x)/u_lightMarchingSteps;
    float step = lightInOut.x;

    //Winkel zwischen Lightray und Cameraray
    float cos = dot(normalize(-u_lightDirection),normalize(eyeRayDirection));
//    float phase = henyeyGreenstein(cos, u_hgCoeff);
    // Mix aus forward und backward scattering (mehr forwardscattering) Frostbitepaper
    float phase = mix(henyeyGreenstein(cos, 0.75f), henyeyGreenstein(cos,-0.5f), u_hgCoeff);
    vec3 incidentLight = vec3(0.f);
    float rhomult_dt = -u_absorptionCloud * dt;
    float T = 1.f;

    float powder = 1.0f;

    for (int i = 0; i < u_lightMarchingSteps; i++)
    {
      vec3 pos = samplePosition + step * (-u_lightDirection);
      vec3 texture_position =  remapToVolume(pos, u_boundingVolumeMin,u_boundingVolumeMax,vec3(0.f),u_texScale);
      float heightGradient = height_fraction(pos);
      float densityFromLight = cloud_density(texture_position, heightGradient);
//      if(u_usePowder)
//      {
//        float powderi = 1.0f - exp(rhomult_dt * 2.f * densityFromLight);
//        powder *= powderi;
//        incidentLight += T * u_lightColor * phase * powder;
//      }
//      else
        incidentLight += T * u_lightColor * phase ;
      //Beer-Lambert Gesetz
      float Ti = exp(rhomult_dt * densityFromLight);
      T *= Ti;

      if(T < 0.001f) break;
      step += dt;
    }


   incidentLight = clamp(incidentLight,0.f,1.f);
   return incidentLight;
}

/*
* ray_origin : Ursprung/Startpunkt des Rays
* ray_dir : Richtung des Rays
* rayInOut: .x gibt t Wert an bei dem der Strahl in das Volumen eindringt:
            Eintrittspunkt :-> camera_position + rayInOut.x * ray_dir
*/
vec4 raymarch(vec3 ray_origin, vec3 ray_dir, vec2 rayInOut)
{
    vec4 sum = vec4(0.0f);
    float dt = (rayInOut.y - rayInOut.x) / u_marchingSteps;   // berechnet gleichmäßige Abstände für Eintritt und Austritt

    float step = rayInOut.x;
    vec4 outcolor = vec4(0.f);
    float lightEnergy =0.f;
    vec3 lightColor = vec3(1.0f,1.0f,1.0f);
    float T = 1.f;

    float rhomult_dt = -u_absorptionCloud * dt;

    for(int i = 0; i<u_marchingSteps; i++)
    {
         if(outcolor.a > 0.999f) break;
         vec3 pos = ray_origin + step * ray_dir;
         vec3 texture_pos =  remapToVolume(pos, u_boundingVolumeMin,u_boundingVolumeMax,vec3(0.f),u_texScale);

         float heightGradient = height_fraction(pos);
         float density = cloud_density(texture_pos, heightGradient);

         //Beer-Lambert Gesetz
         float Ti = exp(rhomult_dt * density);
         T *= Ti;

         if (T < 0.001f) break;
         if(u_doLightMarch)
         {
            lightColor = computeLightMarch(pos,ray_dir);
         }
         if(u_useAmbient)
             outcolor.rgb += T * lightColor * density * dt + T * vec3(0.1f,0.1f,0.1f) * density;
        else
             outcolor.rgb += T * lightColor * density * dt ;//;+ T * vec3(0.1f,0.1f,0.1f) * density;

        outcolor.a += (1.0f-Ti) * (1.0f - outcolor.a);

        step += dt ;
    }
    outcolor = clamp(outcolor,0.0f,1.0f);

    return outcolor;
}


vec3 createRay(vec2 px, mat4 projectionInv, mat4 viewInverse)
{
    vec2 pxNDS = px*2.0f - 1.0f;
    vec3 pNDS = vec3(pxNDS,-1.0f);
    vec4 pNDSH = vec4(pNDS,1.0f);
    vec4 dirEye = normalize(projectionInv*pNDSH);
    dirEye.w = 0;

    vec3 dirWorld = (viewInverse * dirEye).xyz;

    return normalize(dirWorld);
}

void main()
{
    vec2 fragPosition = vec2((gl_FragCoord.x) / u_resolution.x, (gl_FragCoord.y) / u_resolution.y );

    vec3 ray_dir = createRay(fragPosition, inverse(projection), inverse(view));

    vec2 rayInOut = box_intersection(u_boundingVolumeMin, u_boundingVolumeMax, u_cameraPosition, ray_dir);
    vec4 outputColor = vec4(1.f);
    if(rayInOut.x != rayInOut.y)
         outputColor = raymarch(u_cameraPosition, ray_dir, rayInOut);

    fragcolor = outputColor;
}