#version 400 core

#define MAX_REGIONS 10

layout(location = 0) in vec4 position;

uniform mat4 MVP; // Model view projection
uniform mat4 MV; // Model view
uniform float u_xDiff; //distance between x points
uniform float u_yDiff; //distance between y points
uniform float scale; // scale of the noise
uniform int u_oct; // number of octaves
uniform float u_lacunarity; // lacunarity
uniform float u_persistence; // persistence

uniform double u_Time; // current program time
uniform vec3 lightPosition;

//out vec4 gl_Position; // Position after transformed by noise
out vec3 normalInterp; // interpolated normals
out vec3 col; // color
out vec4 gridPos; // position inputed by grid mesh
out vec4 Position; // position in view space without projection

// Structure to define regions of the terrain
struct Region{
  int id; // unique id for each region
  float Height; // associated maximum height
  float pad1;  // to make it n*sizeof(vec4)
  float pad2;
  vec4 Color;
};

layout(std140) uniform colorBlock{
  Region terrainRegions[MAX_REGIONS];
};


// Simplex 2D noise
// Written by Ashima Arts and Stefan Gustavson
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v){
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
		   -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
  + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
	dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}
// return random value
float random(float x, float maximum){
  return maximum*fract(sin(x)*100000.0);
}

float generateOctaves(vec2 pos, int octaves, float persistence, float lacunarity, float seed, vec2 offset){
  float amplitude = 1;
  float frequency = 1;
  float noiseHeight = 0;
  float maximum = 0;
  //vec2 octaveOffsets[octaves];
  seed = random(seed, 10000.0);
  for(int i = 0; i < octaves; i++){
	pos = pos /scale * frequency + offset + vec2(seed);
	float simplexValue = snoise(pos);
	noiseHeight += simplexValue*amplitude;

	maximum += amplitude;
	amplitude *= persistence;
	frequency *= lacunarity;
	seed = random(seed, 1000.0);
  }
  return noiseHeight/maximum;
  //return noiseHeight;
}
//calculate the normal of each vertex
vec3 calcNormal(vec3 position){
  vec3 xOff = vec3(position.x + u_xDiff/2.0, 0, position.z);
  vec3 yOff = vec3(position.x, 0, position.z  + u_yDiff/2.0);
  xOff.y = generateOctaves(xOff.xz, u_oct, u_persistence, u_lacunarity, 150,vec2(0,0) );
  yOff.y = generateOctaves(yOff.xz, u_oct, u_persistence, u_lacunarity, 150,vec2(0,0) );

  vec3 xGrad = xOff - position;
  vec3 yGrad = yOff - position;

  vec3 norm = normalize(cross(xGrad, yGrad));
  return -norm;
}


void main(){
  //float max = 0.01;
  //float height = snoise(position.xy)*u_max;
  float height = generateOctaves(position.xy, u_oct, u_persistence, u_lacunarity, 150,vec2(0,0) );
  gridPos = position.xzyw;
  gridPos.y = height;
  // only works if there are at least 2 colors
  //gl_Position = MVP * position.xzyw;
  gl_Position = MVP * gridPos;
  Position = MV*gridPos;
  normalInterp = calcNormal(gridPos.xyz);
}
