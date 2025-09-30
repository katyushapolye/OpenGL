#version 440 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;
layout (location = 3) in mat4 instanceMatrix; //(loc3 4 5 and 6 will be the mat4 s)

out vec2 fTexCoord;
out vec3 fVertexNormal;
out vec3 fVertexNormal2; //for artistic reasons, we do some random rotations of the normals to smooth them out and create some randomness
out vec3 fFragPos;

uniform float time;


/*NOISE FUNCTIONS from the book of shaders, we use a 3d perlin noise, 2d for th*/


// 3D Simplex Noise (fast and smooth)
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 permute(vec4 x) { return mod289(((x*34.0)+1.0)*x); }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

float snoise(vec3 v) { 
    const vec2 C = vec2(1.0/6.0, 1.0/3.0);
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

    vec3 i = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx);

    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);

    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy;
    vec3 x3 = x0 - D.yyy;

    i = mod289(i); 
    vec4 p = permute(permute(permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0)) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0));

    float n_ = 0.142857142857;
    vec3 ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_);

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);

    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww;

    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);

    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m*m, vec4(dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3)));
}

float windIntensity(vec3 worldPos, float time, float gustFrequency, vec3 windDir) {
    // Normalize the wind direction
    vec3 direction = normalize(windDir);
    

    vec3 windSamplePos = worldPos * 0.05 + direction * time * 0.8;
    float baseWind = snoise(windSamplePos);
    

    vec3 turbulenceDir = normalize(direction + vec3(0.2, 0.1, 0.3));
    vec3 turbulencePos = worldPos * 0.2 + turbulenceDir * time * 2.0; 
    float turbulence = snoise(turbulencePos); 
    
    float intensity = baseWind * 0.7 + turbulence*0.3;

    
    return intensity;
}

//Matrix UBO
layout(std140, binding = 0) uniform Matrixes  {
    mat4 viewMat;
    mat4 projectionMat;
};

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
} 
//degrees

vec3 rotateY(vec3 vector, float angle) {
    float rad = radians(angle);
    
    mat3 rotationMat = mat3(
        cos(rad),0,sin(rad),
        0,1,0,
        -sin(rad),0,cos(rad)
   
    );
    
    return rotationMat * vector;
}
vec3 rotateZ(vec3 vector, float angle) {
    float rad = radians(angle);
    
    mat3 rotationMat = mat3(
        cos(rad),-sin(rad),0,
        sin(rad),cos(rad),0,
        0,0,1
   
    );
    
    return rotationMat * vector;
}

void main()
{

    //wind randomness, what is the best algortihm to create pockets and sustaining winds
   vec3 worldPos = (instanceMatrix * vec4(vPosition, 1.0)).xyz;
    
    // Wind intensity with time-based variation
    float intensity = windIntensity(worldPos, time, 0.3,vec3(-1,0,0));
    
    // Apply wind effect (consider using intensity more subtly)
    float factor = (intensity*50  + 25.0+(2*(gl_InstanceID*8931) % 7) )* (vPosition.y / 1.5);



    vec3 newPos = rotateZ(vPosition, factor);
    vec3 rotatedNormal = rotateY(vNormal,5.0);
    vec3 rotatedNormal2 = rotateY(vNormal,-5.0);

    rotatedNormal = rotateZ(vNormal, factor);
    rotatedNormal2 = rotateZ(rotatedNormal2, factor);


    gl_Position = projectionMat * viewMat * instanceMatrix * vec4(newPos, 1.0);
    fFragPos = vec3(instanceMatrix * vec4(newPos, 1.0)); // Use rotated position
    fVertexNormal = (instanceMatrix * vec4(rotatedNormal, 0.0)).xyz;
    fVertexNormal2 = (instanceMatrix * vec4(rotatedNormal2, 0.0)).xyz;
    fTexCoord = vTexCoords;
}