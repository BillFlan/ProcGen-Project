// #ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
#define GL_GLEXT_PROTOTYPES
// #endif

#include <GLFW/glfw3.h>
#include "CSCIx229.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "stb_image.h"
#include "Mesh.h"
#include <signal.h>
#include <vector>
#include <iostream>

#define PI 3.14159265

// window info
int width = 1000;
int height = 700;

// Stuff for changing the view
glm::vec3 cameraPosition = glm::vec3 (0.0f, 3.0f, 6.0f);
glm::vec3 direction;
float horizontalAngle = 3.14f;
float verticalAngle = 0;
float fieldOfView = 45.0f;

// Map generation variables
float verticalScale = 2.2f; // tallest possible value
float verticalShift = 0.2f; // shift up by this amount
float scale = 3.5f; // scale of initial map
int octaves = 4;
float lacunarity = 3;
float persistence = 0.6;
float seed = 150.0;
glm::vec2 gridoffset = glm::vec2(0.0f,0.0f);
int animateOffset = 0;

double programTime; //current time

glm::vec3 lightPosition = glm::vec3(1.0,1.0,1.0); // where the light at


// get the view and projection based on current position, angles, and FOV
glm::mat4 getProjectionViewMatrix(){
  // direction that the camera is looking at
  direction = glm::vec3 (cos(verticalAngle) * sin(horizontalAngle),
						 sin(verticalAngle),
						 cos(verticalAngle) * cos(horizontalAngle));

  // Right vector
  glm::vec3 right = glm::vec3 (sin(horizontalAngle - 3.14f/2.0f),
					 0,
					 cos(horizontalAngle - 3.14f/2.0f)
					 );

  // Up vector : perpendicular to both direction and right
  glm::vec3 up = glm::cross( right, direction );

  // Projection matrix : 45&deg; Field of View, appropriate ratio, display range : 0.1 unit <-> 100 units
  glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(fieldOfView), (float) width / (float) height, 0.1f, 100.0f);

  glm::mat4 ViewMatrix = glm::lookAt(
									 cameraPosition, // Where we are
									 cameraPosition + direction, // Where we're looking
									 up // The up direction
									 );

  return ProjectionMatrix*ViewMatrix;
}

// same as above but only the view matrix (need for Phong lighting)
glm::mat4 getViewMatrix(){
// direction that the camera is looking at
  glm::vec3 direction = glm::vec3 (cos(verticalAngle) * sin(horizontalAngle),
						 sin(verticalAngle),
						 cos(verticalAngle) * cos(horizontalAngle));

  // Right vector
  glm::vec3 right = glm::vec3 (sin(horizontalAngle - 3.14f/2.0f),
					 0,
					 cos(horizontalAngle - 3.14f/2.0f)
					 );

  // Up vector : perpendicular to both direction and right
  glm::vec3 up = glm::cross( right, direction );

  glm::mat4 ViewMatrix = glm::lookAt(
									 cameraPosition, // Where we are
									 cameraPosition + direction, // Where we're looking
									 up // The up direction
									 );

  return ViewMatrix;
}

// Callback for glfw resizing
void resize(GLFWwindow* window, int w, int h){
  //width = w;
  //height = h;
  //glViewport(0, 0, w, h);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  //glViewport(0, 0, width, height);
}

// GLFW callback for keyboard input
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){

  // direction that the camera is looking at
  glm::vec3 direction = glm::vec3 (cos(verticalAngle) * sin(horizontalAngle),
						 sin(verticalAngle),
						 cos(verticalAngle) * cos(horizontalAngle));

  // Right vector
  glm::vec3 right = glm::vec3 (sin(horizontalAngle - 3.14f/2.0f),
					 0,
					 cos(horizontalAngle - 3.14f/2.0f)
							   );

  // Rotate camera with arrow keys
  if (key == GLFW_KEY_LEFT && action >= 1){
	horizontalAngle += 0.1;
  }
  if (key == GLFW_KEY_RIGHT && action >= 1){
	horizontalAngle -= 0.1;
  }
  if (key == GLFW_KEY_DOWN && action >= 1){
	verticalAngle -= 0.1;
  }
  if (key == GLFW_KEY_UP && action >= 1){
	verticalAngle += 0.1;
  }

  // Move camera with WASD
  if (key == GLFW_KEY_W && action >= 1){
	cameraPosition += 0.2f*direction;
  }
  if (key == GLFW_KEY_S && action >= 1){
	cameraPosition -= 0.2f*direction;
  }
  if (key == GLFW_KEY_A && action >= 1){
	cameraPosition -= 0.2f*right;
  }
  if (key == GLFW_KEY_D && action >= 1){
	cameraPosition += 0.2f*right;
  }

  // Q/E to adjust octaves
  if (key == GLFW_KEY_Q && action >= 1){
	if(octaves > 1) octaves --;
	printf("octaves: %d\n", octaves);
  }
  if (key == GLFW_KEY_E && action >= 1){
	octaves++;
	printf("octaves: %d\n", octaves);
  }

  // Brackets to adjust scale
  if (key == GLFW_KEY_LEFT_BRACKET && action >= 1){
	scale -= 0.2f;
	printf("scale: %f\n", scale);
  }
  if (key == GLFW_KEY_RIGHT_BRACKET && action >= 1){
	scale += 0.2f;
	printf("scale: %f\n", scale);
  }

  // persistence with 1/2
  if (key == GLFW_KEY_1 && action >= 1){
	if(persistence > 0.1) persistence -= 0.1;
	printf("persistence: %f\n", persistence);
  }
  if (key == GLFW_KEY_2 && action >= 1){
	if(persistence < 0.7) persistence += 0.1;
	printf("persistence: %f\n", persistence);
  }

  // lacunarity with 3/4
  if (key == GLFW_KEY_3 && action >= 1){
	if(lacunarity > 0.1) lacunarity -= 0.1;
	printf("lacunarity: %f\n", lacunarity);
  }
  if (key == GLFW_KEY_4 && action >= 1){
	lacunarity += 0.1;
	printf("lacunarity: %f\n", lacunarity);
  }

  // vertical scale with ;/'
  if (key == 39 && action >= 1){
	if(verticalScale > 1.0) verticalScale -= 0.1;
	printf("Vertical Scale: %f\n", verticalScale);
  }
  if (key == 59 && action >= 1){
	verticalScale += 0.1;
	printf("Vertical Scale: %f\n", verticalScale);
  }

	// vertical shift with -/+
  if (key == 45 && action >= 1){
	verticalShift -= 0.1;
	printf("Vertical Shift: %f\n", verticalShift);
  }
  if (mods == GLFW_MOD_SHIFT && key == 61 && action >= 1){
	verticalShift += 0.1;
	printf("Vertical Shift: %f\n", verticalShift);
  }

  // New seed for terrain generator with spacebar
  if(key == GLFW_KEY_SPACE && action == GLFW_RELEASE){
	seed = (float) (rand()%10000);
  }
  if(key == GLFW_KEY_Z && action == GLFW_RELEASE){
	if(animateOffset == 0) animateOffset = 1;
	else animateOffset = 0;
  }

}


// Not a super usefull function, but better than nothing
static bool GLLogCall(){
  while(GLenum error = glGetError()){
	printf("Error: %x\n", error);
	return false;
  }
  return true;
}


// These are the shader import functions from example 27

/*
 *  Read text file
 */
char* ReadText(char *file)
{
   int   n;
   char* buffer;
   //  Open file
   FILE* f = fopen(file,"rt");
   if (!f) Fatal("Cannot open text file %s\n",file);
   //  Seek to end to determine size, then rewind
   fseek(f,0,SEEK_END);
   n = ftell(f);
   rewind(f);
   //  Allocate memory for the whole file
   buffer = (char*)malloc(n+1);
   if (!buffer) Fatal("Cannot allocate %d bytes for text file %s\n",n+1,file);
   //  Snarf the file
   if (fread(buffer,n,1,f)!=1) Fatal("Cannot read %d bytes for text file %s\n",n,file);
   buffer[n] = 0;
   //  Close and return
   fclose(f);
   return buffer;
}

/*
 *  Print Shader Log
 */
void PrintShaderLog(int obj,char* file)
{
   int len=0;
   glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&len);
   if (len>1)
   {
	  int n=0;
	  char* buffer = (char *)malloc(len);
	  if (!buffer) Fatal("Cannot allocate %d bytes of text for shader log\n",len);
	  glGetShaderInfoLog(obj,len,&n,buffer);
	  fprintf(stderr,"%s:\n%s\n",file,buffer);
	  free(buffer);
   }
   glGetShaderiv(obj,GL_COMPILE_STATUS,&len);
   if (!len) Fatal("Error compiling %s\n",file);
}

/*
 *  Print Program Log
 */
void PrintProgramLog(int obj)
{
   int len=0;
   glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&len);
   if (len>1)
   {
	  int n=0;
	  char* buffer = (char *)malloc(len);
	  if (!buffer) Fatal("Cannot allocate %d bytes of text for program log\n",len);
	  glGetProgramInfoLog(obj,len,&n,buffer);
	  fprintf(stderr,"%s\n",buffer);
   }
   glGetProgramiv(obj,GL_LINK_STATUS,&len);
   if (!len) Fatal("Error linking program\n");
}

/*
 *  Create Shader
 */
int CreateShader(GLenum type,char* file)
{
   //  Create the shader
   int shader = glCreateShader(type);
   //  Load source code from file
   char* source = ReadText(file);
   glShaderSource(shader,1,(const char**)&source,NULL);
   free(source);
   //  Compile the shader
   fprintf(stderr,"Compile %s\n",file);
   glCompileShader(shader);
   //  Check for errors
   PrintShaderLog(shader,file);
   //  Return name
   return shader;
}

/*
 *  Create Shader Program
 */
int CreateShaderProg(char* VertFile,char* FragFile)
{
   //  Create program
   int prog = glCreateProgram();
   //  Create and compile vertex shader
   int vert = CreateShader(GL_VERTEX_SHADER  ,VertFile);
   //  Create and compile fragment shader
   int frag = CreateShader(GL_FRAGMENT_SHADER,FragFile);
   //  Attach vertex shader
   glAttachShader(prog,vert);
   //  Attach fragment shader
   glAttachShader(prog,frag);
   //  Link program
   glLinkProgram(prog);
   //  Check for errors
   PrintProgramLog(prog);
   //  Return name
   return prog;
}


// loads a texture and returns it's buffer
GLuint LoadTexture(const char* filename, GLuint texture){
  int x,y,n;

  //OpenGL reads images from top to bottom
  stbi_set_flip_vertically_on_load(1);
  unsigned char* data = stbi_load(filename, &x, &y, &n, 4);

  if(texture == 0){
	glGenTextures(1, &texture);
  }
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  glBindTexture(GL_TEXTURE_2D, 0);

  if(data){ // delete the data
	stbi_image_free(data);
  }

  return texture;

}
// Define regions of the terrain, used to color the map
struct Region{
  int id = -1; // unique id for each region
  float Height; // associated start height
  float scale = 1.0; // texture scale
  float pad2;  // Needs to be n*sizeof(vec4)
  glm::vec4 Color;

};


int init(GLFWwindow *&window){ // Gives you the window
  /* Initialize the library */
  if (!glfwInit())
	return -1;

  // Make sure we are using the minimum version (dumb mac stuff)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  /* Create a windowed mode window and its OpenGL context */
  window = glfwCreateWindow(width, height, "Asher Farr CSCI4229 Final", NULL, NULL);
  if (!window)
	{
	  glfwTerminate();
	  return -1;
	}

  glfwSwapInterval(1); // swap on vsync

  // Make sure we rezise correctly
  glfwSetWindowSizeCallback(window, resize);
  //Maybe this will fix the scaling?
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  // Make the keys right
  glfwSetKeyCallback(window, key_callback);
  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  // Enable restart
  glEnable(GL_PRIMITIVE_RESTART);

  glPrimitiveRestartIndex(INT_MAX); //restart at maximum int
  //	glEnable(GL_CULL_FACE);

  //glCullFace(GL_FRONT);
  glEnable(GL_DEPTH_TEST);

  return 1;
}

// create a sphere
Mesh sphere(float radius, int sectorCount, int stackCount){

	float x, y, z, xy;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
	float s, t;                                     // vertex texCoord

	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	std::vector<Vertex> vertices;
	for(int i = 0; i <= stackCount; ++i)
	  {
		stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);             // r * cos(u)
		z = radius * sinf(stackAngle);              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for(int j = 0; j <= sectorCount; ++j)
		  {
			Vertex vert;
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
			vert.Position = glm::vec3(x,y,z);

			// normalized vertex normal (nx, ny, nz)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			vert.Normal = glm::vec3(nx,ny,nz);

			// vertex tex coord (s, t) range between [0, 1]
			s = (float)j / sectorCount;
			t = (float)i / stackCount;
			vert.TexCoords = glm::vec2(s,t);
			vertices.push_back(vert);
		  }
	  }

	// generate CCW index list of sphere triangles
	std::vector<unsigned int> indices;
	int k1, k2;
	for(int i = 0; i < stackCount; ++i)
	  {
		k1 = i * (sectorCount + 1);     // beginning of current stack
		k2 = k1 + sectorCount + 1;      // beginning of next stack

		for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		  {
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if(i != 0)
			  {
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			  }

			// k1+1 => k2 => k2+1
			if(i != (stackCount-1))
			  {
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			  }
		  }
	  }

	std::vector<Texture> textures;
	Mesh sphereMesh(vertices, indices, textures);
	return sphereMesh;
}

int main(void)

{
	GLFWwindow* window;


	if(init(window) == -1) return -1;

	// Make our world grid
	int sizePerSide = 64; // Number of subdivisions in grid
	float minPos = -5.0f; // minimum location
	float posRange = 10.0f; // length
	int floatsPerVertex = 2;
	int xLen = sizePerSide;
	int yLen = sizePerSide;
	float xDiff = posRange/xLen; // x distance between points
	float yDiff = posRange/yLen; // y distance between points

	// Generate vertices for the plane
	GLfloat grid[xLen*yLen*floatsPerVertex];
	int offset = 0;
	for(int y = 0; y < yLen; y++){
	  for(int x = 0; x < xLen; x++){
		float xRatio = x / (float) (xLen-1);

		// build from top down
		float yRatio = 1.0 - (y/ (float) (yLen -1 ));

		float xPos = minPos + (xRatio*posRange);
		float yPos = minPos + (yRatio*posRange);

		grid[offset++] = xPos;
		grid[offset++] = yPos;
	  }
	}

	// Now choose indices to create triangle strips

	int ind = 0;
	int numStripsRequired = yLen - 1;
	int numDegensRequired = 2 * (numStripsRequired - 1);
	int verticesPerStrip = 2 * xLen;

	GLuint idxs[(verticesPerStrip * numStripsRequired) + numDegensRequired];

	for(int y = 0; y < yLen-1; y++){
	  for (int x = 0; x < xLen; x++) {
		// One part of the strip
		idxs[ind++] = ((y * yLen) + x); // Add one from current row
		idxs[ind++] = (((y + 1) * yLen) + x); //Add one from the next row
	   }
	  idxs[ind++] = INT_MAX; // Restart the strip, we moving to the next row
	}

	// Initialize the Vertex Array, Vertex Buffer, and the Element Array Buffer
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid), grid, GL_STATIC_DRAW);

	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*floatsPerVertex, 0);

	glm::mat4 VP = getProjectionViewMatrix(); // current view and projection
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);

	glm::mat4 MVP = VP*Model; //Transform the grid into the appropriate coords
	glm::mat4 MV = getViewMatrix() * Model;
	int prog = CreateShaderProg((char *)"terrain.vert", (char *)"terrain.frag"); // compile terrain shaders
	glUseProgram(prog);

	// Get all the uniform locations so I can pass them to the buffer
	int location = glGetUniformLocation(prog, "MVP");
	int mvLoc = glGetUniformLocation(prog, "MV");
	int eyeDirLoc = glGetUniformLocation(prog, "eyeDirection");
	int octLoc = glGetUniformLocation(prog, "u_oct");
	int u_xDiffLoc = glGetUniformLocation(prog, "u_xDiff");
	int u_yDiffLoc = glGetUniformLocation(prog, "u_yDiff");
	int sLoc = glGetUniformLocation(prog, "scale");
	int vScaleLoc = glGetUniformLocation(prog, "u_vScale");
	int vShiftLoc = glGetUniformLocation(prog, "u_vShift");
	int persistenceLoc = glGetUniformLocation(prog, "u_persistence");
	int lacunarityLoc = glGetUniformLocation(prog, "u_lacunarity");
	int timeLoc = glGetUniformLocation(prog, "u_Time");
	int lightLoc = glGetUniformLocation(prog, "lightPosition");
	int offsetLoc = glGetUniformLocation(prog, "u_offset");
	int seedLoc = glGetUniformLocation(prog, "u_seed");

	// uniforms and shader for light sphere
	int lightProg = CreateShaderProg((char *)"light.vert", (char *)"light.frag");
	int location2 = glGetUniformLocation(lightProg, "MVP");
	int lightLoc2 = glGetUniformLocation(lightProg, "lightPosition");

	// Create the Regions of the terrain that will get passed as a uniform buffer object
	int MAX_REGIONS = 10;
	// needs to be in acending height order
	Region terrainRegions[MAX_REGIONS]; // Array of regions to pass
	int rIndex = -1; // number of defined regions


	// Deep Ocean -inf -> -0.5
	rIndex++;
	terrainRegions[rIndex].id = rIndex;

	terrainRegions[rIndex].Height = -10; //
	terrainRegions[rIndex].Color = glm::vec4(0.047, 0.333, 0.729, 0.8); // Blue

	// Water texture is procedural, so no textures to bind

	// Ocean -0.5 -> 0
	rIndex++;
	terrainRegions[rIndex].id = rIndex;
	terrainRegions[rIndex].Height = -0.5;
	terrainRegions[rIndex].Color = glm::vec4(0.047, 0.333, 0.729, 0.8); // Blue

	// Beach 0 -> 0.075
	rIndex++;
	terrainRegions[rIndex].id = rIndex;
	terrainRegions[rIndex].Height = 0.0;
	terrainRegions[rIndex].Color = glm::vec4(0.858, 0.662, 0.482, 1.0); // Beach color

	glActiveTexture(GL_TEXTURE0); // set sand texture to slot 0
	GLuint sandTexture = LoadTexture((char*) "./textures/sand_512.jpg", 0); // sand texture
	glBindTexture(GL_TEXTURE_2D, sandTexture);

	// Land 0.075 -> 0.5
	rIndex++;
	terrainRegions[rIndex].id = rIndex;
	terrainRegions[rIndex].Height = 0.075;
	terrainRegions[rIndex].Color = glm::vec4(0.282, 0.603, 0.156, 1.0f); // Green
	terrainRegions[rIndex].scale = 0.25;

	glActiveTexture(GL_TEXTURE1); // grass on 1
	GLuint grassTexture = LoadTexture((char*) "./textures/grass_512.jpg", 0); // grass texture
	glBindTexture(GL_TEXTURE_2D, grassTexture);

	// Mountain 0.5 -> 0.75
	rIndex++;
	terrainRegions[rIndex].id = rIndex;
	terrainRegions[rIndex].Height = 0.5;
	terrainRegions[rIndex].Color = glm::vec4(0.521, 0.533, 0.552, 1.0f); // Grey

	glActiveTexture(GL_TEXTURE2); // rock on 2
	GLuint rockTexture = LoadTexture((char *) "./textures/rock_512.jpg", 0);
	glBindTexture(GL_TEXTURE_2D, rockTexture);

	// Snow Caps 0.75 -> inf
	rIndex++;
	terrainRegions[rIndex].id = rIndex;
	terrainRegions[rIndex].Height = 0.75;
	terrainRegions[rIndex].Color = glm::vec4(1, 1,1, 1.0f); // White

	glActiveTexture(GL_TEXTURE3); //snow on 3
	GLuint snowTexture = LoadTexture((char *) "./textures/snow_512.jpg", 0);
	glBindTexture(GL_TEXTURE_2D, snowTexture);


	// Pass textures on to the shader
	glUniform1i(glGetUniformLocation(prog, "sandTexture"), 0); // set it in shader
	glUniform1i(glGetUniformLocation(prog, "grassTexture"), 1); // set it in shader
	glUniform1i(glGetUniformLocation(prog, "rockTexture"), 2); // set it in shader
	glUniform1i(glGetUniformLocation(prog, "snowTexture"), 3); // set it in shader

	// Uniform buffer object to pass regions (this took me a shocking amount of time to figure out)
	unsigned int ubo; //uniform buffer object
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Region)*MAX_REGIONS, NULL, GL_STATIC_DRAW);
	unsigned int blockLocation = glGetUniformBlockIndex(prog, "colorBlock");


	glUniformBlockBinding(prog, blockLocation, 1); // bind to index 1
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo); //hook it up baby
	GLLogCall(); // check for errors
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(terrainRegions), terrainRegions); // Fill buffer with terrain region data (has to be sized correctly)

	//Create the sun
	Mesh lightSphere = sphere(1.0f, 36, 18);
	/*Loop until the user closes the window*/
	float lightOrbitRadius = 15.0;
	double lastTime = 0;
	programTime = glfwGetTime();
	double deltaTime = 0;
	double lightAngle = PI;
	while (!glfwWindowShouldClose(window))
	{
	  glfwGetWindowSize(window, &width, &height);
	  // Time stuff for animating
	  glUseProgram(prog);
	  programTime = glfwGetTime(); //set time
	  deltaTime = (programTime - lastTime);
	  lastTime = programTime;

	  glUniform1d(timeLoc, programTime);

	  // Sun takes longer time to go over the scene than under (just to keep it interesting)
	  if(lightPosition.y < 0) lightAngle += deltaTime;
	  else lightAngle += deltaTime/10;

	  lightPosition = glm::vec3(lightOrbitRadius*cos(lightAngle), lightOrbitRadius*sin(lightAngle), 6.0); // Orbit about the Z axis

	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	  //fun stuff
	  if(animateOffset == 1){
		gridoffset.x += deltaTime/2;
	  }
	  glUniform1f(seedLoc, seed);
	  glUniform2fv(offsetLoc, 1, &gridoffset[0]);

	  VP = getProjectionViewMatrix(); // current view and projection
	  MVP = VP*Model; // Combine with model matrix
	  glUniformMatrix4fv(location, 1, GL_FALSE, &MVP[0][0]); // Send MVP to shaders
	  glUniformMatrix4fv(mvLoc, 1, GL_FALSE, &MV[0][0]); // Send MV to shader

	  // Send terrain variables
	  glUniform1i(octLoc, octaves);
	  glUniform1f(u_xDiffLoc, xDiff);
	  glUniform1f(u_yDiffLoc, yDiff);
	  glUniform1f(sLoc, scale);
	  glUniform1f(persistenceLoc, persistence);
	  glUniform1f(lacunarityLoc, lacunarity);
	  glUniform3fv(lightLoc, 1, &lightPosition[0]);
	  glUniform1f(vScaleLoc, verticalScale);
	  glUniform1f(vShiftLoc, verticalShift);
	  glUniform3fv(eyeDirLoc, 1, &direction[0]);

	  // Actually draw the terrain
	  glBindVertexArray(vao);
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	  glDrawElements(GL_TRIANGLE_STRIP, ind, GL_UNSIGNED_INT,0); // Draw the plane

	  // Draw the sphere
	  lightSphere.Draw(lightProg, GL_TRIANGLES);
	  glUniformMatrix4fv(location2, 1, GL_FALSE, &MVP[0][0]);
	  glUniform3fv(lightLoc2, 1, &lightPosition[0]);



	  GLLogCall(); // Check errors

	  /* Swap front and back buffers */
	  glfwSwapBuffers(window);

	  /* Poll for and process events */
	  glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
