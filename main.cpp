// #ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
// #endif

#include <GLFW/glfw3.h>
#include "CSCIx229.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <signal.h>

#define ASSERT(x) if (!(x)) raise(SIGTRAP);


int cols, rows;
int width = 1000;
int height = 700;
int scale = 20;

// Stuff for changing the view
glm::vec3 cameraPosition = glm::vec3 (0.0f, 0.25f, 1.0f);
float horizontalAngle = 3.14f;
float verticalAngle = 0;
float fieldOfView = 45.0f;

float maxHeight = 0.01;
// get the view and projection based on current position, angles, and FOV
glm::mat4 getProjectionViewMatrix(){
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

  // Projection matrix : 45&deg; Field of View, appropriate ratio, display range : 0.1 unit <-> 100 units
  glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(fieldOfView), (float) width / (float) height, 0.1f, 100.0f);

  glm::mat4 ViewMatrix = glm::lookAt(
									 cameraPosition, // Where we are
									 cameraPosition + direction, // Where we're looking
									 up // The up direction
									 );

  return ProjectionMatrix*ViewMatrix;
}

// Callback for glfw resizing
void resize(GLFWwindow* window, int w, int h){
  width = w;
  height = h;
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

  if (key == GLFW_KEY_W && action >= 1){
	cameraPosition += 0.1f*direction;
  }
  if (key == GLFW_KEY_S && action >= 1){
	cameraPosition -= 0.1f*direction;
  }
  if (key == GLFW_KEY_A && action >= 1){
	cameraPosition -= 0.1f*right;
  }
  if (key == GLFW_KEY_D && action >= 1){
	cameraPosition += 0.1f*right;
  }
  if (key == GLFW_KEY_Q && action >= 1){
	maxHeight -= 0.005;
  }
  if (key == GLFW_KEY_E && action >= 1){
	maxHeight += 0.005;
  }
}

// Error code based on The Cherno's
// Get rid of all the errors
static void GLClearError(){
  while(glGetError() != GL_NO_ERROR);
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

int main(void)

{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	// Make sure we are using the minimum version (dumb mac stuff)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	// Make sure we rezise correctly
	glfwSetWindowSizeCallback(window, resize);
	// Make the keys right
	glfwSetKeyCallback(window, key_callback);
	/* Make the window's context current */
	glfwMakeContextCurrent(window);


	int sizePerSide = 64;
	float minPos = -0.5;
	float posRange = 1.0;
	int floatsPerVertex = 2;
	int xLen = sizePerSide;
	int yLen = sizePerSide;

	// Generate vertices for the plane
	GLfloat grid[xLen*yLen*floatsPerVertex];
	int offset = 0;
	for(int y = 0; y < yLen; y++){
	  for(int x = 0; x < xLen; x++){
		float xRatio = x / (float) (xLen-1);

		// build from top down
		float yRatio = 1.0 - (y/ (float) (yLen -1 ));

		float xPos = minPos + xRatio*posRange;
		float yPos = minPos + yRatio*posRange;

		grid[offset++] = xPos;
		//grid[offset++] = ((float) (sin(offset)))/2;
		grid[offset++] = yPos;
	  }
	}

	//Now choose indices

	int ind = 0;
	int numStripsRequired = yLen - 1;
	int numDegensRequired = 2 * (numStripsRequired - 1);
	int verticesPerStrip = 2 * xLen;

	GLuint idxs[(verticesPerStrip * numStripsRequired) + numDegensRequired];

	for(int y = 0; y < yLen-1; y++){
	  if(y > 0){
		// create degenerate vertices in first row
		idxs[ind++] = y*yLen;
	  }
	  for (int x = 0; x < xLen; x++) {
		// One part of the strip
		idxs[ind++] = ((y * yLen) + x);
		idxs[ind++] = (((y + 1) * yLen) + x);
	  }
	  if (y < yLen - 2) {
		// Degenerate end: repeat last vertex
		idxs[ind++] = (((y + 1) * yLen) + (xLen - 1));
	  }
	}

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

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glm::mat4 VP = getProjectionViewMatrix(); // current view and projection
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);

	glm::mat4 MVP = VP*Model;
	int prog = CreateShaderProg((char *)"terrain.vert", (char *)"terrain.frag");
	glUseProgram(prog);

	int location = glGetUniformLocation(prog, "MVP");
	int maxLoc = glGetUniformLocation(prog, "u_max");

	/*Loop until the user closes the window*/
	while (!glfwWindowShouldClose(window))
	{

	  /* Render here */
	  glClear(GL_COLOR_BUFFER_BIT);
	  GLClearError();

	  VP = getProjectionViewMatrix(); // current view and projection
	  MVP = VP*Model; // Combine with model matrix
	  glUniformMatrix4fv(location, 1, GL_FALSE, &MVP[0][0]); // Send MVP to shaders
	  glUniform1f(maxLoc, maxHeight);
	  glDrawElements(GL_TRIANGLE_STRIP, ind, GL_UNSIGNED_INT,0); // Draw the plane
	  ASSERT(GLLogCall()); // Check errors

	  /* Swap front and back buffers */
	  glfwSwapBuffers(window);

	  /* Poll for and process events */
	  glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
