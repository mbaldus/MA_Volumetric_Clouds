#version 330
#define M_PI 3.1415926535897932384626433832795

//!< in-variables
in vec4 passWorldPosition;

//!< Camera and Scene Settings
uniform float u_time;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec2 u_resolution;
uniform vec3 u_cameraPosition;
uniform bool u_applyFog;
uniform float u_fogValue;


//!< Raymarching Settings
uniform sampler3D u_perlinWorleyBase;
uniform sampler3D u_erosionNoise;
uniform sampler2D u_weatherTexture;
uniform sampler2D u_cloudHeightTex;
uniform sampler2D u_highAltCloudTex;
uniform vec3 u_planetCenter;             //center of the planet
uniform int u_planetRadius;             //radius of the planet
uniform int u_cloudVolumeStartHeight;     //radius from planet center to start of sky volume
uniform int u_cloudVolumeThickness;       //thickness of the sky volume where clouds appear
uniform int u_cloudVolumeEndHeight;       //radius from planet center to end of sky volume
uniform int u_skyDomeSize;       //radius from planet center to end of sky volume
uniform float u_detailLevel;
uniform int u_marchingStepsMin;
uniform int u_marchingStepsMax;
uniform int u_lightMarchingSteps;
uniform vec3 u_texScaleNoise;
uniform float u_texScaleErosion;
uniform float u_texScaleCoverage;

//!< Cloudsettings
uniform float u_coverage;
uniform int u_coverageChannel;
uniform float u_extinctionCloud;
uniform float u_cloudType;
uniform bool u_useCloudHeightFromTex;
uniform bool u_useWind;
uniform bool u_useDetails;
uniform float u_windSpeed;
uniform vec3 u_windDirection;
uniform float u_cloudScale;

//!< Light
uniform vec3 u_lightDirection;
uniform vec3 u_lightColor;
uniform bool u_doLightMarch;
uniform float u_hgCoeff;
uniform bool u_useAmbient;
uniform bool u_useHighAltClouds;
uniform float u_ambientAmount;

//!< Sky
uniform float u_highAltCloudStrength;
uniform vec3 u_betaR;
uniform float u_betaM;
uniform float u_sunpower;
uniform float u_sunpower_mie;
uniform float u_fraction;
uniform float u_testvalue;

//!< Lightning
uniform bool u_lightning;
uniform float u_lightningThreshold;
uniform vec3 u_lightningPos;
uniform float u_lightningStrength;

vec4 stratusGradient = vec4(0.f, 0.1f, 0.2f, 0.25f);
vec4 stratoCumulusGradient = vec4(0.f, 0.13f, 0.318f, 0.745f);
vec4 cumulusGradient = vec4(0.0f, 0.1f, 0.78f, 1.0f);
vec4 planetColor =  vec4(0.77f,0.74f,0.71f,1.f);
//!< out-variables
layout (location = 0)out vec4 fragcolor;
layout (location = 1)out vec4 alphaTexture;

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

// utility function that maps a value from one range to another
float remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
    return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}
//returns t0_t1_numhits
//t0_t1_numhits.x : t0
//t0_t1_numhits.y : t1
//t0_t1_numhits.z : number of hits with sphere [0,2]
//http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
vec3 raySphereIntersection(vec3 rayOrigin, vec3 rayDir, vec3 sphereCenter, float sphereRadius)
{
    vec3 t0_t1_numhits = vec3(0.f);
	vec3 l = rayOrigin - sphereCenter;
	float a = dot(rayDir,rayDir);
	float b = 2.0f * dot(rayDir, l);
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
	else if(sqrt(discriminate*discriminate) - 0.00005f <= 0.0f)
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
			    t0_t1_numhits.z = 0.f;          //if t1 is also negtaive return 0 hits
				return t0_t1_numhits;
			}
			t0_t1_numhits.z = 1.f;
			return t0_t1_numhits;
		}
		t0_t1_numhits.z = 2.0f;
		return t0_t1_numhits;
	}
}

vec4 getFogColor(vec4 inColor, vec3 rayStartPos, vec3 rayDir, float mult)
{
        float fogAmount = 1.0f - exp(-distance(u_cameraPosition, rayStartPos) * mult/u_fogValue);
        fogAmount = pow(fogAmount,4);
        vec3  fogColor  = u_lightColor;
    return vec4(mix(inColor.rgb, fogColor, fogAmount),inColor.a);
}

/*
* Höhengradient, skaliert Wolkendichte über Höhe
* position: position des raysamples
*/
float getHeightFraction(vec3 position)
{
    float height_fraction = (length(position) - u_cloudVolumeStartHeight) / u_cloudVolumeThickness;
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
    return emptySky * 0.0f + coverageMap * cloudCoverageFromTex*0.75 + fullCoverage * 1.f;
}

float getCloudTypeHeightGradient(float cloudType, float heightGradient)
{
    vec4 mixedGradients = getGradientFromCloudType(cloudType);
    float cloudTypeGradient = smoothstep(mixedGradients.x, mixedGradients.y, heightGradient) - smoothstep(mixedGradients.z,mixedGradients.w,heightGradient);
    return cloudTypeGradient;
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

// Inscattering from Sky
vec4 getSkyInscattering()
{
    vec3 spherePointPos = normalize(passWorldPosition.xyz) * (u_skyDomeSize);
    vec3 ray_dir_sky = spherePointPos - u_cameraPosition;
    vec3 betaR = u_betaR * u_fraction;
    vec3 betaM = vec3(u_betaM * u_fraction);
    float s = length(ray_dir_sky);
    float theta = dot(normalize(-u_lightDirection),normalize(ray_dir_sky));
    float rayleighCoeff = rayleigh(theta);
    float mieCoeff = henyeyGreenstein(theta, 0.95f);
    vec3 L_InScattering = (rayleighCoeff * u_sunpower + mieCoeff*u_sunpower_mie) + u_lightColor * (1.f - exp(-(betaR + betaM)*s));
    return vec4(L_InScattering,1.f);
}

float getCloudDensity(vec3 texturePos, float heightGradient)
{
    vec3 original_texture_pos = texturePos;
    if (u_useWind)
        texturePos += u_windDirection * u_time * u_windSpeed;

    vec2 texturePosCoverage2D = texturePos.xz*u_texScaleCoverage * 0.25f + 0.05 * original_texture_pos.xz;
                                                                          //offsetmotion
    //load densities from 3D Perlin Worley Noise Texture
    float baseCloud = texture(u_perlinWorleyBase, texturePos).r;

    //load cloud height from texture
    float cloudTypeGradient = 0.f;
    float  heightValue = texture(u_cloudHeightTex,texturePosCoverage2D*2.f).r;

    if(u_useCloudHeightFromTex)
         cloudTypeGradient = getCloudTypeHeightGradient(heightValue, heightGradient);
    else
         cloudTypeGradient = getCloudTypeHeightGradient(u_cloudType, heightGradient);

    baseCloud*=cloudTypeGradient;

    //get coverage from weather texture
    float cloudCoverage;
    switch (u_coverageChannel)
     {
        case(0): //red channel
            cloudCoverage = texture(u_weatherTexture, texturePosCoverage2D).r;
            break;
        case(1): //green channel
            cloudCoverage = texture(u_weatherTexture, texturePosCoverage2D).g;
            break;
        case(2): //blue channel
            cloudCoverage = texture(u_weatherTexture, texturePosCoverage2D).b;
            break;
     }

    cloudCoverage = getFullCloudCoverage(cloudCoverage);

    cloudCoverage = clamp(cloudCoverage,0.f,1.f);
    float baseCloudWithCoverage  = remap(baseCloud, 1.0f - cloudCoverage, 1.0, 0.0, 1.0); //-1 um cloud coverage umzudrehen

    //erode edges of the cloud
    if(u_useDetails)
    {
        if(baseCloudWithCoverage > 0.f)
        {
         float highFreqNoiseFbm = texture(u_erosionNoise, texturePos * u_texScaleErosion).x;
         float highFreqNoiseModifier = mix(highFreqNoiseFbm, 1.0f- highFreqNoiseFbm, clamp(heightGradient*u_detailLevel,0.f,1.0f));
         baseCloudWithCoverage = remap(baseCloudWithCoverage,highFreqNoiseModifier,1.0f,0.0f,1.0f);
        }
    }

    baseCloudWithCoverage *= cloudCoverage;
    baseCloudWithCoverage = clamp(baseCloudWithCoverage,0.f,1.f);
    return baseCloudWithCoverage;
}

vec4 raymarch(vec3 rayStartPos, vec3 rayEndPos, vec3 rayDir, vec3 skyDomePos, bool renderSky)
{
    //compute length from ray through the volume
    float rayDistanceInVolume = length(rayEndPos - rayStartPos);
    //calculate amount of marching steps for ray in specific range (i.e. [64;128])
    float dynamicMarchingSteps  = mix(float(u_marchingStepsMin),float(u_marchingStepsMax),clamp(((rayDistanceInVolume-u_cloudVolumeThickness)/u_cloudVolumeThickness),0.f,1.f));

    float dt = rayDistanceInVolume / float(u_marchingStepsMax);
    float step = 0.f;

    //color and light parameters
    vec4 outColor = vec4(0.f);
    vec3 lightColor = vec3(1.0f);

    float T = 1.f;
    //init phase function parameters
    float cos = dot(normalize(-u_lightDirection),normalize(rayDir));
    float rayleighFactor = rayleigh(cos);
    float henyeyFactor = mix(henyeyGreenstein(cos, 0.75f), henyeyGreenstein(cos,-0.5f), u_hgCoeff);
    float mieFactor = henyeyGreenstein(cos, 0.99f);

    //init params for lightning effect
    float cosLightning = dot(u_lightningPos,normalize(rayDir));
    float henyeyLightning = henyeyGreenstein(cosLightning,0.f);
    vec3 incidentLightFromLightning= vec3(0.f);
    for(int i = 0; i<u_marchingStepsMax; i++)
    {
         if(outColor.a >= 1.f) break;

         vec3 samplePosition = rayStartPos  + step * rayDir;
         vec3 texturePos =  samplePosition * u_cloudScale * u_texScaleNoise;

         float heightGradient = getHeightFraction(samplePosition);

         float cloudDensity = getCloudDensity(texturePos, heightGradient);

         //Beer-Lambert Gesetz
         float Ti = exp(-u_extinctionCloud * cloudDensity * dt );
         T *= Ti;

         if (T < 0.01f) break;

         float heightInVolume = length(samplePosition)- u_cloudVolumeStartHeight;
         vec3 lightSphereIntersection = raySphereIntersection(samplePosition,-u_lightDirection, u_planetCenter, u_cloudVolumeEndHeight);
         vec3 lightOutPos = samplePosition - u_lightDirection * lightSphereIntersection.x;
         float lightDistance = length(samplePosition - lightOutPos);
         float dtLight = lightDistance*0.5/u_lightMarchingSteps;
         float stepLight = 0.f;
         vec3 incidentLight = vec3(0.f);
         float T_light = 1.0f;
         int j;
         for (j = 0; j < u_lightMarchingSteps; j++)
         {
            vec3 samplePositionLight = samplePosition + stepLight * (-u_lightDirection);
            float heightLight = length(samplePositionLight) - u_cloudVolumeStartHeight;
            if(heightLight > 0.f)
            {

            // only compute Lightcolor from Beer-Lambert Law when cloud density at the sample is > 0
            if (cloudDensity > 0.01f)
            {
            vec3 texturePos =  samplePositionLight *  u_cloudScale;
            float heightGradient = getHeightFraction(samplePositionLight);
            float densityFromLight = getCloudDensity(texturePos * u_texScaleNoise , heightGradient);

            incidentLight += T_light * (u_lightColor * henyeyFactor + u_lightColor * rayleighFactor);
            //Beer-Lambert Gesetz
            float Ti = exp(-u_extinctionCloud * dtLight * densityFromLight) ;
            T_light *= Ti;
            if (T_light < 0.01f) break;
            }

            stepLight += dtLight;
            }
         }

        if(u_lightning && cloudDensity > u_lightningThreshold && u_extinctionCloud > 1.f)
        {
            float T_lightning = 1.0f;
            float stepLightning = 0.f;
            vec3 sampleDirection = samplePosition - u_lightningPos;
            float dtLighning = distance(samplePosition,u_lightningPos) / 2.f;
            for (int s = 0; s < 4; s++)
            {
              vec3 samplePositionLightning = samplePosition + stepLightning * sampleDirection;
              vec3 texturePosLightning =  samplePositionLightning *  u_cloudScale;
              float heightGradient = getHeightFraction(samplePositionLightning);
              float densityFromLightning = getCloudDensity(texturePosLightning * u_texScaleNoise , heightGradient);
              incidentLightFromLightning = T_lightning * vec3(0.8f,0.9f,1.f) * henyeyLightning;

              float Ti = exp(-u_extinctionCloud * dtLighning * densityFromLightning);
              T_lightning *= Ti;

              stepLightning += dtLighning;
              }
            }

         if(u_useAmbient)
             outColor.rgb += T * cloudDensity * dt *(incidentLight + (u_ambientAmount * vec3(1.0f, 1.0f, 1.0f)+(u_lightningStrength * incidentLightFromLightning)));
         else
             outColor.rgb += T * incidentLight * cloudDensity * dt + T * cloudDensity * u_lightningStrength * incidentLightFromLightning;

        outColor.a += (1.0f-Ti) * (1.0f - outColor.a);

        step += dt ;
    }

    //save alpha value in FBO-Texture for postprocessing
    alphaTexture = vec4(1.0,0.f,0.f,outColor.a);
    if(renderSky) //return sky as background
    {
            vec4 L_InScattering = getSkyInscattering();
            vec4 sky = L_InScattering;
            if(u_useHighAltClouds)
            {
            float highAltClouds = texture(u_highAltCloudTex, skyDomePos.xz  * 0.0005f + u_windDirection.xz * u_time * u_windSpeed * 0.15f).r;
            highAltClouds *= texture(u_cloudHeightTex, skyDomePos.xz*0.0005).r; //mix with some perlin noise from height texture
            sky.rgb = mix(sky.rgb, u_lightColor, highAltClouds*u_highAltCloudStrength);
            }
            outColor = outColor + sky * (1.f - outColor.a);
    }
    outColor = clamp(outColor,0.0f,1.0f);

    if(u_applyFog)
        return getFogColor(outColor, rayStartPos,rayDir, 1.f);

    return outColor;
}

void main()
{
    vec2 fragPosition = vec2((gl_FragCoord.x) / u_resolution.x, (gl_FragCoord.y) / u_resolution.y );

    vec3 rayDir = createRay(fragPosition, inverse(projection), inverse(view));

    vec3 rayHitsPlanet = raySphereIntersection(u_cameraPosition, rayDir, u_planetCenter, u_planetRadius);
    vec3 rayHitsCloudBottom = raySphereIntersection(u_cameraPosition, rayDir, u_planetCenter, u_cloudVolumeStartHeight);
    vec3 rayHitsCloudTop = raySphereIntersection(u_cameraPosition, rayDir, u_planetCenter, u_cloudVolumeEndHeight);
    vec3 rayHitsSkyDome = raySphereIntersection(u_cameraPosition, rayDir, u_planetCenter, u_skyDomeSize);

    vec3 posRayHitPlanet = u_cameraPosition + rayDir * rayHitsPlanet.x;
    vec3 posRayHitCloudBottom = u_cameraPosition + rayDir * rayHitsCloudBottom.x;
    vec3 posRayHitCloudTop = u_cameraPosition + rayDir * rayHitsCloudTop.x;
    vec3 posRayHitSkyDome = u_cameraPosition + rayDir * rayHitsSkyDome.x;

    float distanceCameraPlanet = distance(u_cameraPosition, u_planetCenter);
    vec4 outputColor = vec4(0.f);
    //Camera Config 1 - camera is between planet and cloud volume
    if(distanceCameraPlanet < u_cloudVolumeStartHeight)
    {
      if (rayHitsPlanet.z > 0)  //planet in the way of the ray
      {
        outputColor = getFogColor(planetColor, posRayHitPlanet, rayDir, 2.f );
        alphaTexture = vec4(0.f,1.0,0.0f,1.f); //save alpha of planet to alpha target texture
      }
      else                      //camera pointing towards sky
       outputColor = raymarch(posRayHitCloudBottom,posRayHitCloudTop,rayDir,posRayHitSkyDome,true);
    }
    //Camera Config 2 - camera is inside the cloudvolume
     else if (distanceCameraPlanet > u_cloudVolumeStartHeight && distanceCameraPlanet < u_cloudVolumeEndHeight)                                         // #1. enter volume, leave volume to inside
     {
          if (rayHitsCloudBottom.z ==0)  //look outwards
                     outputColor = raymarch(u_cameraPosition, u_cameraPosition + rayDir * rayHitsCloudTop.x, rayDir, posRayHitSkyDome, true);
          else if (rayHitsCloudBottom.z == 2 && rayHitsPlanet.z == 0)
          {
                     outputColor = raymarch(u_cameraPosition, u_cameraPosition + rayDir * rayHitsCloudBottom.x, rayDir, posRayHitSkyDome, true);
                     vec4 outputColor2 = raymarch(u_cameraPosition + rayDir * rayHitsCloudBottom.y, u_cameraPosition + rayDir * rayHitsCloudTop.x, rayDir,posRayHitSkyDome,true);
                     outputColor = outputColor + outputColor2 * (1.f-outputColor.a);
          }
          else if(rayHitsCloudBottom.z == 2 && rayHitsPlanet.z > 0)
          {
                     outputColor = raymarch(u_cameraPosition, u_cameraPosition + rayDir * rayHitsCloudBottom.x, rayDir, vec3(0), false);
                     outputColor = outputColor + getFogColor(planetColor, posRayHitPlanet, rayDir, 1.f ) * (1.f-outputColor.a);
          }
//          else if(rayHitsCloudBottom.z > 0)//hit with lower sky volume shell %TODO überprüfen
//                  {
//                     if(rayHitsCloudBottom.z == 2  && rayHitsPlanet.z ==0) //ray enters skyvolume again
//                     {
//                         outputColor = raymarch(u_cameraPosition, posRayHitCloudBottom, ray_dir,vec3(0),false);
//                         vec4 outputColor2 = raymarch(u_cameraPosition + ray_dir * rayHitsCloudBottom.y, u_cameraPosition + ray_dir * rayHitsCloudTop.x, ray_dir,posRayHitSkyDome,true);
//                         outputColor = outputColor + outputColor2 * (1.f-outputColor.a);
//                     }
//                     else //ray hits planet
//                     {
//                           outputColor = raymarch( u_cameraPosition, posRayHitCloudBottom, ray_dir,vec3(0),false);
//                           outputColor = outputColor + getFogColor(planetColor, posRayHitPlanet, ray_dir, 1.f ) * (1.f-outputColor.a);
//                     }
//                  }                                                                             // #2. enter volume, leave volume to outside again
       }
    //Camera Config 3 camera is above the cloudvolume
     else
     {
         if(rayHitsCloudTop.z == 0) //over cloud layer
                  outputColor = getSkyInscattering();
         else if (rayHitsCloudTop.z == 2 && rayHitsCloudBottom.z ==0)
                  outputColor = raymarch(posRayHitCloudTop, posRayHitCloudTop + rayDir * rayHitsCloudTop.y, rayDir,posRayHitSkyDome, true);
         else if(rayHitsCloudBottom.z == 2 && rayHitsPlanet.z ==0)
              {
                   outputColor = raymarch(posRayHitCloudTop, posRayHitCloudBottom, rayDir,vec3(0),false);
                   vec4 outputColor2 = raymarch(u_cameraPosition + rayDir * rayHitsCloudBottom.y, u_cameraPosition + rayDir * rayHitsCloudTop.y, rayDir,posRayHitSkyDome,true);
              }else
              {
                   outputColor = raymarch( posRayHitCloudTop, posRayHitCloudBottom, rayDir,vec3(0), false); //#1
                   outputColor = outputColor + getFogColor(planetColor, posRayHitPlanet, rayDir, 1.f ) * (1.f-outputColor.a);
              }
       }
    fragcolor = outputColor;
}

