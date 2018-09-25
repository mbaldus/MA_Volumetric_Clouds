#version 330
#define M_PI 3.1415926535897932384626433832795

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
uniform bool u_applyFog;
uniform float u_fogValue;

//!< Raymarching Settings
uniform sampler3D u_perlinWorleyBase;
uniform sampler3D u_erosionNoise;
uniform sampler2D u_weatherTexture;
uniform sampler2D u_curlnoise1;
uniform sampler2D u_backCloudTex;
uniform vec3 u_planetCenter;             //center of the planet
uniform int u_planetRadius;             //radius of the planet
uniform int u_skyVolumeStartHeight;     //radius from planet center to start of sky volume
uniform int u_skyVolumeThickness;       //thickness of the sky volume where clouds appear
uniform int u_skyVolumeEndHeight;       //radius from planet center to end of sky volume
uniform float u_detailLevel;
uniform int u_marchingStepsMin;
uniform int u_marchingStepsMax;
uniform int u_lightMarchingSteps;
uniform vec3 u_texScaleNoise;
uniform float u_texScaleErosion;
uniform float u_texScaleCoverage;
uniform float u_texScaleCurl;

//!< Cloudsettings
uniform float u_coverage;
uniform float u_absorptionCloud;
uniform float u_cloudType;
uniform bool u_useCTypeFromTex;
uniform bool u_useWind;
uniform bool u_useDetails;
uniform float u_windSpeed;
uniform vec3 u_windDirection;
uniform float u_sunpower;
uniform float u_sunpower_mie;
uniform float u_cloudScale;
uniform float u_shapeModifier;

uniform vec3 u_lightDirection;
uniform vec3 u_lightColor;
uniform bool u_doLightMarch;
uniform float u_hgCoeff;
uniform bool u_usePowder;
uniform bool u_useAmbient;
uniform bool u_useBackClouds;
uniform float u_ambientAmount;
uniform vec3 u_betaR;
uniform float u_betaM;
uniform float u_vh;
uniform float u_testvalue;

uniform bool u_lightning;
uniform float u_lightningThreshold;
uniform vec3 u_lightningPos;
uniform float u_lightningStrength;

vec4 stratusGradient = vec4(0.f, 0.05f,u_testvalue-0.2f, u_testvalue);
//vec4 stratoCumulusGradient = vec4(0.72f, 0.2f, 0.3f, 0.4f);
vec4 stratoCumulusGradient =  vec4(0.02f, 0.05f, 0.68, 0.75);
vec4 cumulusGradient = vec4(0.f, 0.1f, 0.75f, 1.0f);
//!< out-variables
layout (location = 0)out vec4 fragcolor;

// utility function that maps a value from one range to another.
float remap(float original_value, float original_min, float original_max, float new_min, float new_max)
{
    return new_min + (((original_value - original_min) / (original_max - original_min)) * (new_max - new_min));
}
//returns t0_t1_numhits
//t0_t1_numhits.x : t0
//t0_t1_numhits.y : t1
//t0_t1_numhits.z : number of hits with sphere [0,2]
//http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
vec3 raySphereIntersection(vec3 ray_origin, vec3 ray_dir, vec3 sphereCenter, float sphereRadius)
{
    vec3 t0_t1_numhits = vec3(0.f);
	vec3 l = ray_origin - sphereCenter;
	float a = 1.0f;
	float b = 2.0f * dot(ray_dir, l);
	float c = dot(l, l) - sphereRadius * sphereRadius;
	float discriminate = b * b - 4.0f * a * c;
	if(discriminate < 0.0f)
	{
	    //no hit with sphere
		t0_t1_numhits.x = 0.0f;
		t0_t1_numhits.y = 0.0f;
		t0_t1_numhits.z = 0.0f;
		return t0_t1_numhits;
	}
	else if(abs(discriminate) - 0.00005f <= 0.0f)
	{
	    //one hit with sphere
	    t0_t1_numhits.x = -0.5f * b / a;
	    t0_t1_numhits.y = -0.5f * b / a;
		t0_t1_numhits.z = 1.f;
		return t0_t1_numhits;
	}
	else
	{
		float q = b > 0.0f ? -0.5f * (b + sqrt(discriminate)) : -0.5f * (b - sqrt(discriminate));
		float h1 = q / a;
		float h2 = c / q;
		t0_t1_numhits.x = min(h1, h2);
		t0_t1_numhits.y = max(h1, h2);
		if(t0_t1_numhits.x < 0.0f)
		{
			t0_t1_numhits.x = t0_t1_numhits.y;  //if t0 is negative then use t1
			if(t0_t1_numhits.x < 0.0f)
			{
			    t0_t1_numhits.z = 0.f;         //if t1 is also negtaive return 0 hits
				return t0_t1_numhits;
			}
			t0_t1_numhits.z = 1.f;
			return t0_t1_numhits;
		}
		t0_t1_numhits.z = 2.0f;
		return t0_t1_numhits;
	}
}

vec4 getFogColor(vec4 inColor, vec3 ray_start_pos, vec3 ray_dir, float mult)
{
        float fogAmount = 1.0f - exp(-distance(u_cameraPosition, ray_start_pos) * mult/u_fogValue);
        fogAmount = pow(fogAmount,4);
        float sunAmount = max( dot( ray_dir, u_lightDirection ), 0.0 );
        vec3  fogColor  = vec3(0.7,0.8,0.9); // bluish
//        vec3  fogColor  = mix( vec3(0.6,0.7,0.8), // bluish
//                                   vec3(1.0,0.9,0.7), // yellowish
//                                   pow(sunAmount,8.0) );
    return vec4(mix(inColor.rgb, fogColor, fogAmount),inColor.a);
}

/*
* Höhengradient, skaliert Wolkendichte über Höhe
* position: position des raysamples
*/
float height_fraction(vec3 position)
{
    float height_fraction = (length(position) - u_skyVolumeStartHeight) / u_skyVolumeThickness;
    return clamp(height_fraction,0.f,1.f);
}

vec4 getGradientFromCloudType(float cloudType)
{
    //interpoliert
    float stratus = 1.0f - clamp(cloudType * 2.0f,0.f,1.f);
   	float stratocumulus = 1.0f - abs(cloudType - 0.5f) * 2.0f;
   	float cumulus = clamp(cloudType - 0.5f, 0.0f,1.f) * 2.0f;
   	return stratusGradient * stratus + stratoCumulusGradient * stratocumulus + cumulusGradient * cumulus;
}

float getFullCloudCoverage(float cloudCoverageFromTex)
{
    //interpolation between emptysky, coveragemap and
    float emptySky = 1.0f - clamp(u_coverage * 2.0f,0.f,1.f);
    float coverageMap = 1.0f - abs(u_coverage - 0.5f) * 2.0f;
    float fullCoverage = clamp(u_coverage - 0.5f, 0.0f,1.f) * 2.0f;
    return emptySky * 0.001f + coverageMap * cloudCoverageFromTex + fullCoverage * 1.f;
}

vec4 getGradientFromCloudType2(float u_cloudType)
{
	return mix(stratusGradient, cumulusGradient, u_cloudType);
}

float getCloudTypeHeightGradient(float cloudType, float heightGradient)
{
    vec4 mixedGradients = getGradientFromCloudType2(cloudType);
    float cloudTypeGradient = smoothstep(mixedGradients.x, mixedGradients.y, heightGradient) - smoothstep(mixedGradients.z,mixedGradients.w,heightGradient);
    return cloudTypeGradient;
}

vec3 ambientLight(float heightFrac)
{
	return mix(vec3(0.85f, 0.93f, 1.f),vec3(1.0f, 1.0f, 1.0f),heightFrac);
}

//https://www.astro.umd.edu/~jph/HG_note.pdf
float henyeyGreenstein(float cos, float g)
{
	float g2 = g * g;
	return 1.f/(4.f*M_PI) * ((1.0f - g2) / pow((1.0f + g2 - 2.0f * g * cos), 1.5f));
}

float rayleigh(float cos)
{
    return 3.f/(16.f*M_PI)*(1+cos*cos);
}


float cloud_density(vec3 texture_pos, float heightGradient)
{
    vec3 original_texture_pos = texture_pos; //scaled from [0,1]
    if (u_useWind)
    {
//        texture_pos += heightGradient * vec3(0.1f,0.0f,0.1f) * u_testValue;
        texture_pos += u_windDirection * u_time * u_windSpeed;
    }

//    vec2 texture2Dpos = texture_pos.xz*0.25f + 0.05f*original_texture_pos.xz+0.5f;
//    vec2 texturePosCoverage2D = texture_pos.xz*u_texScaleCoverage * 0.25f + 0.05f*original_texture_pos.xz+0.5f;
    vec2 texturePosCoverage2D = texture_pos.xz*u_texScaleCoverage * 0.25f + 0.05f * original_texture_pos.xz;
                                                                    //offsetmotion
//    vec2 texturePosCoverage2D =  0.05f*original_texture_pos.xz * u_texScaleCoverage;
    vec2 texture2Dpos2 =  texturePosCoverage2D;
    //load densities from 3D Noise Texture
    float base_cloud = texture(u_perlinWorleyBase, texture_pos).r;

    //load cloudtype from weathertexture
    float cloudTypeGradient = 0.f;
    float heightValue = texture(u_curlnoise1,texture2Dpos2*4.f).g;
    heightValue = remap(heightValue,0.f,1.f,0.2,1.f);
    vec4 cloudTypeNew = vec4(0.0f,0.1f, heightValue-0.2f, heightValue);
    if(u_useCTypeFromTex)
//        cloudTypeGradient = getCloudTypeHeightGradient(texture(u_weatherTexture,texture2Dpos2).b  , heightGradient);
         cloudTypeGradient = smoothstep(cloudTypeNew.x, cloudTypeNew.y, heightGradient) - smoothstep(cloudTypeNew.z,cloudTypeNew.w,heightGradient);
    else
        cloudTypeGradient = getCloudTypeHeightGradient(u_cloudType, heightGradient);

    base_cloud = remap(base_cloud, 1.0 - cloudTypeGradient, 1.0f,0.0f,1.0f);


    float cloudCoverage = texture(u_weatherTexture, texturePosCoverage2D).r; //use texture_pos.xz if coverage signal should move
//    float cloudCoverage = texture(u_weatherTexture, texture_pos.xz*u_texScaleCoverage).r; //use texture_pos.xz if coverage signal should move
    cloudCoverage = getFullCloudCoverage(cloudCoverage);
    //anvil deformation
    if(u_useDetails)
    cloudCoverage = pow(cloudCoverage, remap(heightGradient, 0.5f, 1.f, mix(1.0f, 0.5f, u_shapeModifier), 1.f));

    cloudCoverage = clamp(cloudCoverage,0.f,1.f);
    float base_cloud_with_coverage  = remap(base_cloud, 1.0f - cloudCoverage, 1.0, 0.0, 1.0); //-1 um cloud coverage umzudrehen

        if(base_cloud_with_coverage > 0.f)
        {
//          vec2 curlNoise = texture(u_curlnoise1,texture_pos.xz).rg;
//         texture_pos.xz += curlNoise * (1.0f-heightGradient) * u_texScaleCurl * 0.1f;
//       base_cloud_with_coverage = remap(base_cloud_with_coverage,highFreqNoiseModifier * u_detailLevel,1.0f,0.0f,1.0f);
         float highFreqNoiseFbm = texture(u_erosionNoise, texture_pos * u_texScaleErosion).x;
         float highFreqNoiseModifier = mix(highFreqNoiseFbm, 1.0f- highFreqNoiseFbm, clamp(heightGradient*u_detailLevel,0.f,1.0f));
         base_cloud_with_coverage = remap(base_cloud_with_coverage,highFreqNoiseModifier,1.0f,0.0f,1.0f);
        }

    base_cloud_with_coverage = clamp(base_cloud_with_coverage,0.f,1.f);
    return base_cloud_with_coverage;
}

vec4 raymarch_sphere(vec3 ray_start_pos, vec3 ray_end_pos, vec3 ray_dir)
{
    //compute length from ray through the volume
    float rayDistanceInVolume = length(ray_end_pos - ray_start_pos);
    //calculate amount of marching steps for ray in specific range (i.e. [64;128])
    float dynamicMarchingSteps  = mix(float(u_marchingStepsMin),float(u_marchingStepsMax),clamp(((length(rayDistanceInVolume)-u_skyVolumeThickness)/u_skyVolumeThickness),0.f,1.f));

    float dt = rayDistanceInVolume / dynamicMarchingSteps;
    float step = 0.f;

    //color and light parameters
    vec4 outcolor = vec4(0.f);
    vec3 lightColor = vec3(1.0f);
    vec3 skycolor_h = vec3(0.f);
    vec3 skycolor_r = vec3(0.f);
    vec3 skycolor_m = vec3(0.f);
    vec3 betaR = u_betaR * 0.000001f ;
    vec3 betaM = vec3(u_betaM * 0.000001f);

    float T = 1.f;
    float opticalDepthR = 0.f;
    float opticalDepthH = 0.f;
    float opticalDepthM = 0.f;
//    float powder = 1.0f;
    //init phase function parameters
    float cos = dot(normalize(-u_lightDirection),normalize(ray_dir));
    float rayleighFactor = rayleigh(cos);
    float henyeyFactor = mix(henyeyGreenstein(cos, 0.75f), henyeyGreenstein(cos,-0.5f), u_hgCoeff);
    float mieFactor = henyeyGreenstein(cos, 0.99f);

    //init params for lightning effect
    vec3 incidentLightFromLightning= vec3(0.f);

    for(int i = 0; i<dynamicMarchingSteps; i++)
    {
         if(outcolor.a >= 1.f) break;
         vec3 samplePosition = ray_start_pos + step * ray_dir;
         vec3 texture_pos =  samplePosition * u_cloudScale;

         float heightGradient = height_fraction(samplePosition);

         float density = cloud_density(texture_pos * u_texScaleNoise, heightGradient);
//         texture_pos += u_windDirection * u_time * u_windSpeed;
//         float absorbtion = texture(u_curlnoise1,texture_pos.xz).g;
//         absorbtion = remap(absorbtion,0.f,1.f,0.75f,1.5f);
         //Beer-Lambert Gesetz
         float Ti = exp(-u_absorptionCloud * dt * density);
         T *= Ti;

         if (T < 0.001f) break;

         float heightInVolume = length(samplePosition)- u_skyVolumeStartHeight;
         opticalDepthR += exp(-heightInVolume / u_vh)  * dt;
         opticalDepthM += exp(-heightInVolume / 1.2f)  * dt;
         vec3 lightSphereIntersection = raySphereIntersection(samplePosition,-u_lightDirection, u_planetCenter, u_skyVolumeEndHeight);
         vec3 lightOutPos = samplePosition - u_lightDirection * lightSphereIntersection.x;
         float lightDistance = length(samplePosition - lightOutPos);
         float dtLight = lightDistance*0.5f/u_lightMarchingSteps;
         float stepLight = 0.f;
         float opticalDepthLightR = 0.f;
         float opticalDepthLightH = 0.f;
         float opticalDepthLightM = 0.f;
         vec3 incidentLight = vec3(0.f);
         float T_light = 1.0f;
         int j;
         for (j = 0; j < u_lightMarchingSteps; j++)
         {
            vec3 samplePositionLight = samplePosition + stepLight * (-u_lightDirection);
            float heightLight = length(samplePositionLight) - u_skyVolumeStartHeight;
            if(heightLight >0.f)
            {
            opticalDepthLightR += exp(-heightLight/ u_vh) * dtLight; //TODO
            opticalDepthLightM += exp(-heightLight/ 1.2f) * dtLight;

            // only compute Lightcolor from Beer-Lambert Law when cloud density at the sample is > 0
            if (density > 0.01f)
            {
            vec3 texture_pos =  samplePositionLight *  u_cloudScale;
            float heightGradient = height_fraction(samplePositionLight);
            float densityFromLight = cloud_density(texture_pos * u_texScaleNoise , heightGradient);

            incidentLight += T_light * (u_lightColor * henyeyFactor + u_lightColor * rayleighFactor);

            //Beer-Lambert Gesetz
            float Ti = exp(-u_absorptionCloud * dtLight * densityFromLight);
            T_light *= Ti;

            opticalDepthLightH += Ti;
            }

            stepLight += dtLight;
            }
         }
         if(j == u_lightMarchingSteps)
         {
            vec3 tau = betaR * (opticalDepthR + opticalDepthLightR) + betaM * (opticalDepthM + opticalDepthLightM);
            vec3 attenuation = vec3(exp(-tau));
            skycolor_r += attenuation * opticalDepthR;
            skycolor_m += attenuation * opticalDepthM;
         }

        if(u_lightning && density > u_lightningThreshold && u_absorptionCloud > 1.3f)
        {
            float T_lightning = 1.0f;
            float stepLightning = 0.f;
            vec3 lightNingPos = samplePosition + u_lightningPos;
            vec3 sampleDirection = samplePosition - lightNingPos;
//            vec3 sampleDirection = samplePosition - u_lightningPos;
            float dtLighning = distance(samplePosition,lightNingPos) / 3.f;
            float cosLightning = dot(lightNingPos,normalize(ray_dir));
            float henyeyLightning = henyeyGreenstein(cosLightning,0.f);
            if (density > 0.01f)
                    {
            for (int s = 0; s < 3; s++)
            {
              vec3 samplePositionLightning = samplePosition + stepLightning * sampleDirection;
              vec3 texture_pos_lightning =  samplePositionLightning *  u_cloudScale;
              float heightGradient = height_fraction(samplePositionLightning);
              float densityFromLightning = cloud_density(texture_pos_lightning * u_texScaleNoise , heightGradient);
              incidentLightFromLightning = T_lightning * vec3(0.8f,0.9f,1.f) * henyeyLightning;

              float Ti = exp(-u_absorptionCloud * dtLighning * densityFromLightning);
              T_lightning *= Ti;

              stepLightning += dtLighning;
                 }
            }
        }
         float depth = 1.f - exp(-pow(distance(samplePosition,u_cameraPosition)* 1.f/u_ambientAmount,2)) ;
         if(u_useAmbient)
             outcolor.rgb += T * incidentLight * density * dt + T * ambientLight(heightGradient) * density  * depth + T  * density * u_lightningStrength * incidentLightFromLightning;
         else
             outcolor.rgb += T * incidentLight * density * dt + T * density * u_lightningStrength * incidentLightFromLightning;

        outcolor.a += (1.0f-Ti) * (1.0f - outcolor.a);

        step += dt ;
    }
//     vec4 sky = vec4(skycolor_r * betaR * rayleighFactor * 1.f/2.5f , 1.0f);
//     vec4 sky = vec4( betaM * 1.1f * skycolor_m * mieFactor, 1.0f);
//     vec4 sky = vec4(skycolor_r * betaR * rayleighFactor * u_testValue + skycolor_h * henyeyFactor, 1.0f);
     vec4 sky_r = vec4(skycolor_r * betaR * rayleighFactor,1.0f);
     vec4 sky_m = vec4(skycolor_m * betaM * 1.1f * mieFactor, 1.0f);
     sky_r *= u_sunpower;
     sky_m *= u_sunpower_mie;
     vec4 sky = sky_r + sky_m;

    if(u_useBackClouds)
    {
    float backClouds = texture(u_backCloudTex, ray_end_pos.xz *0.0025f - u_time  * 0.005f).r;
    sky.rgb = mix(sky.rgb, u_lightColor, backClouds*0.3f);
    }
    outcolor = outcolor + sky * (1.f - outcolor.a);
    outcolor = clamp(outcolor,0.0f,1.0f);
    if(u_applyFog)
    {
         return getFogColor(outcolor, ray_start_pos,ray_dir, 1.f);
    }

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

    vec3 rayHitsPlanet = raySphereIntersection(u_cameraPosition, ray_dir, u_planetCenter, u_planetRadius);
    vec3 rayHitsSkyBottom = raySphereIntersection(u_cameraPosition, ray_dir, u_planetCenter, u_skyVolumeStartHeight);
    vec3 rayHitsSkyTop = raySphereIntersection(u_cameraPosition, ray_dir, u_planetCenter, u_skyVolumeEndHeight);

    vec3 posRayHitPlanet = u_cameraPosition + ray_dir * rayHitsPlanet.x;
    vec3 posRayHitSkyBottom = u_cameraPosition + ray_dir * rayHitsSkyBottom.x;
    vec3 posRayHitSkyTop = u_cameraPosition + ray_dir * rayHitsSkyTop.x;

    float distanceCameraPlanet = distance(u_cameraPosition, u_planetCenter);
    vec4 outputColor = vec4(0.f);
    if(distanceCameraPlanet < u_skyVolumeStartHeight) //camera is between planet and sky volume
    {
      if (rayHitsPlanet.z > 0)  //planet in the way of the ray
      {
//       outputColor = raymarch_sphere(u_cameraPosition,rayHitsSkyBottom.x,rayHitsSkyBottom.y,ray_dir);
//       outputColor = mix(outputColor,vec4(0.61f,0.29f,0.0f,1.f),0.5);
//         outputColor = vec4(0.61f,0.29f,0.0f,1.f);
        outputColor = getFogColor(vec4(0.77f,0.74f,0.71f,1.f), posRayHitPlanet, ray_dir, 50.f );
      }
      else
       outputColor = raymarch_sphere(posRayHitSkyBottom,posRayHitSkyTop,ray_dir);
    }
     else if (distanceCameraPlanet > u_skyVolumeEndHeight) //camera is above the sky volume // #1. enter volume, leave volume to inside
    {                                                                                       // #2. enter volume, leave volume to outside again
       if (rayHitsSkyTop.z == 0)                                                             //no hit with sky volume
          outputColor = vec4(0.f);
       else
       {
        if(rayHitsSkyTop.z == 2 && rayHitsSkyBottom.z == 0)
             outputColor = raymarch_sphere( posRayHitSkyTop, (u_cameraPosition + ray_dir * rayHitsSkyTop.y), ray_dir);    // #2
        else
             outputColor = raymarch_sphere( posRayHitSkyTop, posRayHitSkyBottom, ray_dir); //#1
       }
    }
     else  //camera is inside the skyvolume
    {
        if (rayHitsSkyBottom.z > 0) //hit with lower sky volume shell
            outputColor = raymarch_sphere( u_cameraPosition, posRayHitSkyBottom, ray_dir);
        else
            outputColor = raymarch_sphere( u_cameraPosition, posRayHitSkyTop, ray_dir);
    }

    fragcolor = outputColor;
}