#version 400 core
#define MAX_REGIONS 10

in vec4 newPos;
in vec4 realPos;
uniform float u_max;
in vec3 col;

// Structure to define regions of the terrain
struct Region{
  int id; // unique id for each region
  float Height; // associated maximum height
  float pad1;  // to make it n*sizeof(vec4)
  float pad2;
  vec4 Color;
};

uniform int u_oct;
uniform float u_lacunarity;
uniform float u_persistence;
uniform float scale;
uniform mat4 MVP;
uniform double u_Time;
uniform float u_xDiff;
uniform float u_yDiff;
uniform vec3 cameraPosition;
uniform vec3 lightPosition;
in vec4 gl_FragCoord ;


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

// uniform block of colors/regions
layout(std140) uniform colorBlock{
  Region terrainRegions[MAX_REGIONS];
};

vec3 lightPos = lightPosition;
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float lightPower = 40.0;
//const vec3 ambientColor = vec3(0.1, 0.0, 0.0);
//const vec3 diffuseColor = vec3(0.5, 0.0, 0.0);
const vec3 specColor = vec3(1.0, 1.0, 1.0);
const float shininess = 30.0;

//vec3 lightDirection = vec3(0,-5,0);
out vec4 color;
in vec3 normalInterp;

Region currentRegion;

void main(){
  //float height = (newPos.y + 1.0)/2.0;
  float height = newPos.y;
  for(int i = 0; i < MAX_REGIONS; i++){
	if(terrainRegions[i].id == -1) break; // don't want undefined
	if(height <= terrainRegions[i].Height){
	  color = terrainRegions[i].Color;
	  currentRegion = terrainRegions[i];
	  break;
	}
  }
  vec3 ambientColor = vec3(color)/1.5;
  vec3 diffuseColor = vec3(color);

  // color = vec4 (height, height, height, 1.0);
  // wikipedia blinn-phong code
  //vec3 normal = calcNormal(newPos.xyz);


  vec3 normal = -normalInterp;
  vec3 lightDir = lightPos - realPos.xyz;
  float distance = length(lightDir);
  distance = distance * distance;
  lightDir = normalize(lightDir);

  float lambertian = max(dot(lightDir, normal), 0.0);
  float specular = 0.0;

  if (lambertian > 0.0) {

	vec3 viewDir = normalize(-realPos.xyz);

	// this is blinn phong
	vec3 halfDir = normalize(lightDir + viewDir);
	float specAngle = max(dot(halfDir, normal), 0.0);
	specular = pow(specAngle, shininess);
  }
  vec3 colorLinear = ambientColor +
					 diffuseColor * lambertian * lightColor * lightPower / distance +
					 specColor * specular * lightColor * lightPower / distance;

 // color = vec4(min(colorLinear, 1);
  color = vec4(colorLinear,1.0);

 if(currentRegion.id <= 1) color = currentRegion.Color;
  //color = terrainRegions[int(mod(height*23,10))].Color;
  //color = (1.0,1.0,1)
  //color = terrainRegions[int(mod(realPos.x*5,10))].Color;
}
