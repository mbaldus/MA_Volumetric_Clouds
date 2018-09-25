#version 330

//!< in-variables
//in vec4 passPosition;
//in vec3 passUVW;

//!< uniforms
uniform vec3 camera_position;
uniform vec2 resolution;
uniform float fieldOfView;
uniform int marchsteps;
uniform float stepsize;
uniform float out_alpha;
uniform float frequency;
uniform float deltatime;

uniform sampler3D m_texture;

//!< out-variables
layout (location = 0)out vec4 fragcolor;

//https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

/*
*   fractal Brownian Motion (fBM)
*
*   adding octaves of noises with different frequencies
*   amplitude of noise is decreased in every octave
*
*/
float fractal_noise(vec3 p)
{
    float f = 0.0;
    // add animation
    p = p - vec3(1.0, 0.0, 0.0) *  deltatime * 0.05;
    p = p * 7.0;

    //f = amplitude * noise; increment frequencies
    f += 0.50000 * noise(p); p = frequency * p;
	f += 0.25000 * noise(p); p = frequency * p;
	f += 0.12500 * noise(p); p = frequency * p;
	f += 0.06250 * noise(p); p = frequency * p;
    f += 0.03125 * noise(p);

    return f;
}

float density(vec3 pos)
{
    float den = 2.0* fractal_noise(pos * 0.25);// - 2.0 + (pos.y - 1.0);
    den = clamp(den, 0.0, 1.0);
    return den;
}



vec4 raymarch(vec3 ray_origin, vec3 ray_dir)
{
    vec4 sum = vec4(0.0f);
    float t = 0.0f;
    vec4 outcolor = vec4(0.f,0.f,0.f,1.f);
    vec3 pos = ray_origin ;
    for(int i = 0; i<marchsteps; i++)
    {
        if(sum.a > 0.99f) break;
         float den = density(pos);
         if(den > 0.01)
               {
                outcolor = vec4( den );
                outcolor.a *= out_alpha;
                outcolor.xyz *= outcolor.a;
                sum = sum + outcolor*(1.0f-sum.a);
               }
        t+=stepsize;

        pos=ray_origin + t * ray_dir;
    }

    sum = clamp(sum,0.0f,1.0f);

    return sum;
}

void main()
{
    //position of pixel on screen (in [0,1])
    vec2 fragPosition = (2.0f * (gl_FragCoord.xy / resolution)) - 1.0f;  // [-1, 1]
    fragPosition.x *= resolution.x / resolution.y;                       // right ratio for x

    vec3 rd = normalize(vec3(fragPosition.xy, 0) - camera_position);

    vec4 outPutColor = raymarch(camera_position,rd);

//    fragcolor = vec4(outPutColor.xyz,1.0f);
    fragcolor = vec4(outPutColor) ;
}