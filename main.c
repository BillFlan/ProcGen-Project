//#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "CSCIx229.h"

int cols, rows;
int width = 640;
int height = 480;
int scale = 20;
int main(void)

{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/* cols = width/scale; */
	/* rows = height/scale; */
	/* GLuint idxs[(rows-1)*(2*(cols-1) + 3)]; */
	int sizePerSide = 5;
	float minPos = -0.5;
	float posRange = 1.0;
	int indexCount;
	int floatsPerVertex = 2;
	int xLen = sizePerSide;
	int yLen = sizePerSide;


	// Generate vertices for the plane
	GLfloat grid[xLen*yLen*floatsPerVertex];
	int offset = 0;
	for(int y =0; y < yLen; y++){
	  for(int x = 0; x < xLen; x++){
		float xRatio = x / (float) (xLen-1);

		// build from top down
		float yRatio = 1.0 - (y/ (float) (yLen -1 ));

		float xPos = minPos + xRatio*posRange;
		float yPos = minPos + yRatio*posRange;

		grid[offset++] = xPos;
		grid[offset++] = yPos;
	  }
	}
	for(int i = 0; i < xLen*yLen*floatsPerVertex; i+=2){
	  printf("(%f, %f)\n", grid[i], grid[i+1]);
	}
	//	GLuint PRIMITIVE_RESTART = 0xFFFF;
	//	glEnable(GL_PRIMITIVE_RESTART);

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

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid), grid, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*floatsPerVertex, 0);

	/*Loop until the user closes the window*/
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawElements(GL_TRIANGLE_STRIP, ind, GL_UNSIGNED_INT, idxs);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
