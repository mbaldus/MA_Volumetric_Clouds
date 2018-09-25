#version 330

//!< in-variables
in vec4 passPosition;
in vec2 passImagePos;

//!< uniforms
uniform sampler2D u_frontUVWtexture;
uniform sampler3D m_texture3d;
uniform sampler2D u_backUVWtexture;

uniform vec2 resolution;
uniform float out_alpha;
uniform float coverage;
uniform float absorption;
uniform float stepsize;
uniform int marchsteps;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 camera_position;

//!< out-variables
layout (location = 0)out vec4 fragcolor;
layout (location = 1)out vec4 front;
layout (location = 2)out vec4 back;


//vec4 raymarch2(vec3 ray_origin, vec3 ray_dir, vec2 rayInOut)
//{
//    vec4 sum = vec4(0.0f);
//    float dt = (rayInOut.y - rayInOut.x) / marchingSteps;   // berechnet gleichmäßige Abstände für Eintritt und Austritt
//
//    float step = rayInOut.x + stepSize * dt;
//    vec4 outcolor = vec4(0.f,0.f,0.f,1.f);
//    vec3 pos = ray_origin + step * ray_dir;
//
//    float T = 1.f;
//    float rhomult_dt = -3*absorption * dt;
//
//    for(int i = 0; i<marchingSteps; i++)
//    {
//
//         if(sum.a > 0.99f) break;
//                 float den = texture(m_texture3d, pos).x;
//                 float alpha = texture(m_texture3d, pos).w;
//
//                        outcolor.xyz =  vec3(den, 0, 0);
//                        outcolor.a = alpha;
////                        outcolor *= height_fraction(pos,-1.f,1.f);
//                        outcolor.a *= out_alpha;
//                        outcolor.xyz *= outcolor.a;
//                        sum = sum + outcolor*(1.0f-sum.a);
//        step += dt ;
//    }
//    sum = clamp(outcolor,0.0f,1.0f);
//
//    return sum;
//}


vec4 raymarch_uvw(vec3 ray_start, vec3 ray_end, vec3 ray_dir)
{
    vec4 sum = vec4(0.0f);
    float dt = stepsize /  length((ray_end - ray_start));

    vec4 outcolor = vec4(0.f,0.f,0.f,1.f);

    for(float t = 0; t<1.0f; t+=dt)
    {
         vec3 pos = mix(ray_start, ray_end, t);
//         if(sum.a > 0.99f) break;

         float den = texture(m_texture3d, pos).x;
         float alpha = texture(m_texture3d, pos).w;

          outcolor.xyz =  vec3(den, 0,0);
          outcolor.a = alpha;
          outcolor.a *= out_alpha;
          outcolor.xyz *= outcolor.a;
           sum = sum + outcolor*(1.0f-sum.a);
//        step += dt ;
//        pos = ray_start + step * ray_dir;
    }
    sum = clamp(sum,0.0f,1.0f);

    return sum;
}

void main()
{
    vec4 frontUVWsample = texture(u_frontUVWtexture, passImagePos );
    vec4 backUVWsample = texture(u_backUVWtexture, passImagePos);

    vec3 ray_dir = normalize(backUVWsample - frontUVWsample).xyz;

    vec4 outputColor = vec4(0.f);
    if(frontUVWsample.xyz == backUVWsample.xyz)
     outputColor = vec4(0.f);
    else
     outputColor = raymarch_uvw(frontUVWsample.xyz, backUVWsample.xyz, ray_dir);

    fragcolor = outputColor;
    //warum müssen hier die
//    front = vec4(0.f);
//    back = vec4(0.f);
}