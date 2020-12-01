#version 400 core

#define MAX_REGIONS 10

layout(location = 0) in vec4 position;

uniform mat4 MVP;
uniform float u_xDiff;
uniform float u_yDiff;
uniform float scale;
uniform int u_oct;
uniform float u_lacunarity;
uniform float u_persistence;

uniform double u_Time;
uniform vec3 lightPosition;

out vec4 newPos;
out vec3 normalInterp;
out vec3 col;
out vec4 realPos;

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

float generateOctaves(vec2 pos, int octaves, float persistence, float lacunarity){
  float amplitude = 1;
  float frequency = 1;
  float noiseHeight = 0;
  float maximum = 0;
  for(int i = 0; i < octaves; i++){
	pos = pos/scale * frequency;
	float simplexValue = snoise(pos);
	noiseHeight += simplexValue*amplitude;

	maximum += amplitude;
	amplitude *= persistence;
	frequency *= lacunarity;
  }
  return noiseHeight/maximum;
  //return noiseHeight;
}
//calculate the normal of each vertex
vec3 calcNormal(vec3 position){
  vec3 xOff = vec3(position.x + u_xDiff/2.0, 0, position.z);
  vec3 yOff = vec3(position.x, 0, position.z  + u_yDiff/2.0);
  xOff.y = generateOctaves(xOff.xz, u_oct, u_persistence, u_lacunarity);
  yOff.y = generateOctaves(yOff.xz, u_oct, u_persistence, u_lacunarity);

  vec3 xGrad = xOff - position;
  vec3 yGrad = yOff - position;

  vec3 norm = normalize(cross(xGrad, yGrad));
  return norm;
}


void main(){
  //float max = 0.01;
  //float height = snoise(position.xy)*u_max;
  float height = generateOctaves(position.xy, u_oct, u_persistence, u_lacunarity);
  newPos = position.xzyw;
  newPos.y = height;

  // only works if there are at least 2 colors
  //gl_Position = MVP * position.xzyw;
  gl_Position = MVP * newPos;
  realPos = gl_Position;
  normalInterp = calcNormal(newPos.xyz);
}
