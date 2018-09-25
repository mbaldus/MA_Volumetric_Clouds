#version 330

//!< in-variables
in vec4 passPosition;
in vec3 passUVW;

//!< uniforms
uniform sampler3D m_texture3d;
uniform vec2 resolution;
uniform int octave;
uniform float out_alpha;
uniform float coverage;
uniform float stepsize;
uniform int marchsteps;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 camera_position;

//!< out-variables
layout (location = 0)out vec4 fragcolor;


vec4 raymarch_hd(vec3 ray_origin, vec3 ray_dir)
{
    vec4 sum = vec4(0.0f);
    float t = 0.0f;
    vec4 outcolor = vec4(0.f,0.f,0.f,1.f);
    vec3 pos = ray_origin ;
    for(int i = 0; i<marchsteps; i++)
    {
        if(sum.a > 0.99f) break;
         vec4 den = texture(m_texture3d, pos);

         if(den.a > 0.01)
//               {
                outcolor =  den;
                //outcolor *= pos.y;
                outcolor *= coverage;
                outcolor.a *= out_alpha;
                outcolor.xyz *= outcolor.a;
                sum = sum + outcolor*(1.0f-sum.a);
//               }

//            }
        t+=stepsize;
        pos = ray_origin + t * ray_dir;
    }

    sum = clamp(sum,0.0f,1.0f);

    return sum;
}

vec4 raymarch2(vec3 ray_origin, vec3 ray_dir)
{
    vec4 sum = vec4(0.0f);
    float t = 0.0f;
    vec4 outcolor = vec4(0.f,0.f,0.f,1.f);
    vec3 pos = ray_origin ;
    for(int i = 0; i<marchsteps; i++)
    {
        if(sum.a > 0.99f) break;
         vec4 den = texture(m_texture3d, pos);

         //if(length(pos) < 0.5f)
//            sum += vec4(0.05f,0,0,1);
//            else
//            {
//         if(den.a > 0.01)
//               {
                outcolor =  den * pos.y;
                outcolor.a *= out_alpha;
                outcolor.xyz *= outcolor.a;
                sum = sum + outcolor*(1.0f-sum.a);
//               }

//            }
        t+=stepsize;
        pos = ray_origin + t * ray_dir;
    }

    sum = clamp(sum,0.0f,1.0f);

    return sum;
}
vec4 raymarch(vec3 ray_origin, vec3 ray_dir)
{
    vec3 volMin = vec3(-1.0f);
    vec3 volMax = vec3(1.f);
    vec4 sum = vec4(0.0f);
    float t = 0.0f;
    vec4 outcolor = vec4(0.f,0.f,0.f,1.f);
    vec3 pos = ray_origin ;
    for(int i = 0; i<marchsteps; i++)
    {
//        if(sum.a > 0.99f) break;
         vec4 den = texture(m_texture3d, pos);
//         if(den.a > 0.01)
//               {
                outcolor =  den ;
//                outcolor.a *= out_alpha;
//                outcolor.xyz *= outcolor.a;
                sum = sum + outcolor*(1.0f-sum.a);
//               }
        t+=stepsize;

        pos = ray_origin + t * ray_dir;
//        vec3 temp1 = sign(pos-volMin);
//        vec3 temp2 = sign(volMax-pos);
//        float inside = dot(temp1,temp2);
//        if(inside < 3.0f)
//        break;
    }

    sum = clamp(sum,0.0f,1.0f);

    return sum;
}

vec3 createRay(vec2 px, mat4 projectionInv, mat4 viewInverse)
{
    vec2 pxNDS = px*2.0f - 1.0f;
    vec3 pNDS = vec3(px,-1.0f);
    vec4 pNDSH = vec4(pNDS,1.0f);
    vec4 dirEye = normalize(projectionInv*pNDSH);
    dirEye.w = 0;

    vec3 dirWorld = (viewInverse * dirEye).xyz;

    return normalize(dirWorld);
}
void main()
{
    vec2 fragPosition = vec2((gl_FragCoord.x +0.5f) / resolution.x, (gl_FragCoord.y + 0.5f) / resolution.y );

    fragPosition.x *= resolution.x / resolution.y;

    vec3 rd = createRay(fragPosition, inverse(projection), inverse(view));
    vec3 rd2 = normalize(passUVW - camera_position);
//    vec4 outPutColor = raymarch2(passUVW,rd2);
    vec4 outPutColor = raymarch_hd(passUVW,rd2);

    fragcolor = outPutColor;
}