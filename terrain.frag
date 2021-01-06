#version 400 core
#define MAX_REGIONS 10

//in vec4 gl_Position;
in vec4 gridPos;
in vec4 Position;


// Structure to define regions of the terrain
// Since this is passed as an std140 it has to have an evenly spaced layout
struct Region{
  int id; // unique id for each region
  float Height; // associated maximum height
  float scale;  // texture scale
  float pad2; // to make it n*sizeof(vec4)
  vec4 Color;
};

// uniform block of colors/regions
layout(std140) uniform colorBlock{
  Region terrainRegions[MAX_REGIONS];
};

// all those fun variables
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

uniform float u_vScale; // Value for normalizing
uniform float u_vShift; // Vertical shift

uniform vec2 u_offset;
uniform float u_seed;

// textures
uniform sampler2D sandTexture;
uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D snowTexture;

in vec4 gl_FragCoord;
out vec4 FragColor;


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

// Make octaves of noise
float generateOctaves(vec2 pos, int octaves, float persistence, float lacunarity, float seed, vec2 offset){
  float amplitude = 1;
  float frequency = 1;
  float noiseHeight = 0;
  float maximum = 0;

  seed = random(seed, 10000.0); // Sample from different places for each ocatve

  for(int i = 0; i < octaves; i++){
	pos = pos /scale * frequency + offset + vec2(seed); // Sample pos
	float simplexValue = snoise(pos);
	noiseHeight += simplexValue*amplitude; //Add noise
	amplitude *= persistence; // Get smaller
	frequency *= lacunarity; // Get bumpier
	seed = random(seed, 1000.0); // Find a new random spot

  }
  return (noiseHeight)/u_vScale + u_vShift;
}

//calculate the normal of each vertex
vec3 calcNormal(vec3 position){
  vec3 xOff = vec3(position.x + u_xDiff/2.0, 0, position.z); // Location shifed in x from pos
  vec3 yOff = vec3(position.x, 0, position.z  + u_yDiff/2.0); // Location shifted in y (Z in real coord) from pos
  int oct = min(3, u_oct); // Don't do more than 3 octaves, it's both ugly and costly

  // Get height of offsets
  xOff.y = generateOctaves(xOff.xz, oct, u_persistence, u_lacunarity, u_seed, u_offset);
  yOff.y = generateOctaves(yOff.xz, oct, u_persistence, u_lacunarity, u_seed, u_offset);

  // Slope in x and y direction
  vec3 xGrad = xOff - position;
  vec3 yGrad = yOff - position;

  vec3 norm = normalize(cross(xGrad, yGrad));   // Thanks linear algebra!
  return -norm;
}

// Use turbulent noise to texture the water
vec3 calcWaterNormal(vec3 position){
  vec3 xOff = vec3(position.x + 0.025, 0, position.z); // Get a location close to pos
  vec3 yOff = vec3(position.x, 0, position.z  + 0.025);

  // Take absolute value of simplex noise to get a watery look
  xOff.y = abs(snoise(xOff.xz));
  yOff.y = abs(snoise(yOff.xz));

  // Get direction
  vec3 xGrad = xOff - position;
  vec3 yGrad = yOff - position;

  vec3 norm = normalize(cross(xGrad, yGrad)); // Cross product, my best friend
  return -norm;
}

// Light info because I didn't want to pass it all in
vec3 lightPos = lightPosition; // except for the position, I did pass that in
vec3 lightColor = vec3(0.945, 0.870, 0.352);
float lightBrightness = 2.0;
float lightPower = 1.0;
const vec3 specColor = vec3(1.0, 1.0, 1.0);
float shininess = 0.3;
float Ambient = 0.5;


// light attenuation factors
float constAttenuation = 0.001;
float linearAttenuation = 1;
float quadAttenuation = 0;

// Get texture based on the region id
vec4 getTexture(int id, vec2 Pos){
  vec4 color;
  switch (id){
  case 2:
	color = texture(sandTexture, Pos/terrainRegions[2].scale);
	break;
  case 3:
	color = texture(grassTexture, Pos/terrainRegions[3].scale);
	break;
  case 4:
	color = texture(rockTexture, Pos/terrainRegions[4].scale);
	break;
  case 5:
	color = texture(snowTexture, Pos/terrainRegions[5].scale);
	break;
  default:
	color = terrainRegions[id].Color;
	break;
  }
  return color;
}

// Get region based on the height
Region getRegion(float height, int start){
  // If you are >= than the region height, you could be in that region
  Region reg = terrainRegions[0];
  for(int i = 0; i < MAX_REGIONS; i++){
	if(terrainRegions[i].id == -1) break; // don't want undefined
	if(height >= terrainRegions[i].Height){
	  reg = terrainRegions[i];
	}
  }
  return reg;
}


void main(){
  Region currentRegion; // current region for texturing

  float height = gridPos.y;
  vec4 color; //color of a region

  float variation = 0.025; //How much to wiggle the lines
  height = height + random(random(height, variation+gridPos.z),variation); //Add random variation to edges
  currentRegion = getRegion(height, 0);
  color = getTexture(currentRegion.id, gridPos.xz); // get texture for a given id

  // Calculate a per pixel normals using our noise function
  vec3 normal = calcNormal(gridPos.xyz);

  // Time to do some Phong Shading!!!
  // Make the light brighter
  lightColor *= lightBrightness;
  vec3 lightDir = lightPos - vec3(Position);
  float lightDistance = length(lightDir);
  lightDir = lightDir/lightDistance; // normalize

  float attenuation = 1.0/( //Quad makes it look hazier
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
  vec3 rgb = min(color.rgb * scatteredLight + reflectedLight, vec3(1.0));

  FragColor = vec4(rgb, color.a);

  // Do special calculations for waves
  // This is just phong again but for the water
  if(currentRegion.id <= 1){
	// Get the normal for water
	// We animate the offset through time so it looks like the water has waves
	vec3 normalWater = calcWaterNormal(vec3(gridPos.x, abs(snoise(5*gridPos.xz + float(u_Time)/5)), gridPos.z));
	shininess = 10; // Water is shiny
	lightPower = 5;
	lightColor /= lightBrightness; // It got too bright
	//linearAttenuation = 0;
	//quadAttenuation = 1;

	float waterdiffuse = max(0.0, dot(normalWater, lightDir));
	float waterspecular = max(0.0, dot(normalWater, halfVector));


	if (waterdiffuse == 0.0) waterspecular = 0.0;
	else
	  waterspecular = pow(waterspecular, shininess) * lightPower;

	vec3 waterscatteredLight = Ambient + lightColor * waterdiffuse * attenuation;
	vec3 waterreflectedLight = lightColor * waterspecular * attenuation;
	vec3 waterrgb = min(color.rgb * waterscatteredLight + waterreflectedLight, vec3(1.0));

	// Add the waves on top, so you see both the light on the ground below and the waves
	FragColor += vec4(waterrgb, 0.9);
  }
}
