#version 330

//!< in-variables
in vec2 passUV;

//!< uniform-variables
uniform sampler2D cloudOutputTexture;
uniform sampler2D alphaTexture;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 u_sunPositionWorld;
uniform vec3 u_cameraPosition;
uniform float u_density;
uniform float u_weight;
uniform float u_decay;
uniform int u_numSamples;
uniform float u_exposure;

//!< out-variables
out vec4 pixelColor;
void main()
{
    vec2 uv = passUV;
    vec4 originalColor = texture(cloudOutputTexture, uv);
    vec3 cameraLookAt = -normalize(vec3(view[0][2],view[1][2],view[2][2]));
    vec3 sunToCamera = normalize(u_sunPositionWorld);

    float blend = dot(sunToCamera,cameraLookAt);
    if(blend < 0.f)
        pixelColor = originalColor;
     else{
        vec4 sunpos_ndc = projection * view * vec4(u_sunPositionWorld + u_cameraPosition,1.0f); //sunposition in normalized device coordinates
        vec2 sunpos_ss = clamp((sunpos_ndc.xy+1.0f)/2.f,0.f,1.f);                               //sunposition in screenspace
        vec2 deltaTexCoord = ((uv - sunpos_ss) / float(u_numSamples)) * u_density;

        float illuminationDecay = 1.f;
        vec3 accumulatedColor = vec3(0.f);

        for(int i = 0; i < u_numSamples; i++)
        {
            vec4 sampleColor = texture(cloudOutputTexture, uv);
            sampleColor.rgb *= 1.0f-texture(alphaTexture,uv).a; // only compute where cloudedges appear
            sampleColor *= illuminationDecay * u_weight;
            illuminationDecay *= u_decay;
            accumulatedColor += sampleColor.rgb;
            uv -= deltaTexCoord;
        }
         vec4 finalColor =   originalColor + vec4(accumulatedColor * u_exposure,1.0f) * blend;
        pixelColor = finalColor;
    }
}