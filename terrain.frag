#version 400 core
#define MAX_REGIONS 10

//in vec4 gl_Position;
in vec4 gridPos;
in vec4 Position;
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
uniform mat4 MV;
uniform double u_Time;
uniform float u_xDiff;
uniform float u_yDiff;
uniform vec3 cameraPosition;
uniform vec3 lightPosition;
uniform vec3 eyeDirection;
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
  int oct = min(3, u_oct);

  xOff.y = generateOctaves(xOff.xz, oct, u_persistence, u_lacunarity, 150.0, vec2(0,0));
  yOff.y = generateOctaves(yOff.xz, oct, u_persistence, u_lacunarity, 150.0, vec2(0,0));

  vec3 xGrad = xOff - position;
  vec3 yGrad = yOff - position;

  vec3 norm = normalize(cross(xGrad, yGrad));
  return -norm;
}

// uniform block of colors/regions
layout(std140) uniform colorBlock{
  Region terrainRegions[MAX_REGIONS];
};

vec3 lightPos = lightPosition;
vec3 lightColor = vec3(1.0, 1.0, 1.0);
float lightBrightness = 2.0;
float lightPower = 1.0;
//const vec3 ambientColor = vec3(0.1, 0.0, 0.0);
//const vec3 diffuseColor = vec3(0.5, 0.0, 0.0);
const vec3 specColor = vec3(1.0, 1.0, 1.0);
const float shininess = 0.3;
float Ambient = 0.5;

//vec3 lightDirection = vec3(0,-5,0);
out vec4 FragColor;
in vec3 normalInterp;

Region currentRegion;

// light attenuation factors
float constAttenuation = 0.001;
float linearAttenuation = 1;
float quadAttenuation = 0;



void main(){
  //float height = (gridPos.y + 1.0)/2.0;
  float height = gridPos.y;
  vec4 color; //color of a region
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
  vec3 normal = calcNormal(gridPos.xyz);


  //vec3 normal = normalInterp;
  lightColor *= lightBrightness;
  vec3 lightDir = lightPos - vec3(Position);
  float lightDistance = length(lightDir);
  //lightDistance = lightDistance * lightDistance;
  lightDir = lightDir/lightDistance;

  float attenuation = 1.0/(
					constAttenuation +
					linearAttenuation * lightDistance +
					quadAttenuation * lightDistance * lightDistance

						   );

  vec3 halfVector = normalize(lightDir + eyeDirection);

  float diffuse = max(0.0, dot(normal, lightDir));
  float specular = max(0.0, dot(normal, halfVector));


  if (diffuse == 0.0) specular = 0.0;
  else
	specular = pow(specular, shininess) * lightPower;

  vec3 scatteredLight = Ambient + lightColor * diffuse * attenuation;
  vec3 reflectedLight = lightColor * specular * attenuation;
  vec3 rgb = min(color.rgb * scatteredLight + reflectedLight,
				 vec3(1.0)); FragColor = vec4(rgb, color.a);

  FragColor = vec4(rgb, color.a);
 // color = vec4(min(colorLinear, 1);
  //color = vec4(colorLinear,1.0);

 if(currentRegion.id <= 1) FragColor = currentRegion.Color;
  //color = terrainRegions[int(mod(height*23,10))].Color;
  //color = (1.0,1.0,1)
  //color = terrainRegions[int(mod(gridPos.x*5,10))].Color;
}
