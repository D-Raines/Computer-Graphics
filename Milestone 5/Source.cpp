#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#include <cstdlib>			// for realloc and free

/* add stb_image.h file from OpenGLSample folder for Milestone 5 */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility


// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>


/* add camera header file : had to copy from OpenGLSample folder to Module Assignmnet folder, then
	righ-click module Assignment (in solution explorer), select Add, then Existing Item. Then select the camera.h file.
	Then right-click on project, select properties, go to C\C++ under configuration --> General --> Additional Include Directories,
		then add project folder
	* Note:	this will also make sure image files are read for project
*/
#include "camera.h"




// Used to make spheres
#include <cmath>
#include <vector>
#include <corecrt_math_defines.h>



using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "Milestone 5"; // Macro for window title

	// Variables for window width and height
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;


	// Used for camera view
	Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
	float lastX = WINDOW_WIDTH / 2.0f;
	float lastY = WINDOW_HEIGHT / 2.0f;
	bool firstMouse = true;
	// timing
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;


	// Used to set the projection to perspective or orthographic: milestone 4
	bool orthoView = false;	// false = perspective, true = orthographic


/*
	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nIndices;    // Number of indices of the mesh
	};
*/

	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbo;			// Handles for the vertex buffer object
		GLuint nVertices;   // Number of vertices of the mesh
	};

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;

	
	// Mesh data
	GLMesh gMesh;
	GLfloat* meshVerts = nullptr;		// added to append other shapes from functions
	int meshSize = 0;					// keeps track of number of vertices created


	// Texture
	GLuint gTextureId;
//	glm::vec2 gUVScale(5.0f, 5.0f);		// commented out for scaling texture image to map to object (used '[' and ']' to find correct scale)
	glm::vec2 gUVScale(1.0f, 1.0f);
	GLint gTexWrapMode = GL_REPEAT;
	GLfloat numberOfHorizonalImagesInPicture = 5.0f;												// used to reference number of horizonal images in all_surfaces.jpg
	GLfloat imageHeightSpacing = (1.0f / numberOfHorizonalImagesInPicture);
	GLfloat half_image_height = 1.0f / (numberOfHorizonalImagesInPicture * 2);						// half a horizonal images height


	// Shader program
	GLuint gProgramId;


	// Variables for the sphere
	const GLuint numStacks = 48;
	const GLuint numSlices = 48;
	const GLfloat sphereRadius = 0.30f;
	GLuint sphereVao[2], sphereVbo[2];
	const GLfloat separation = 0.7f; // Distance between the two spheres
	struct Vertex {
		GLfloat position[3];
		GLfloat color[4];			// Used for solid RGBA color
		GLfloat texCoord[2];		// Used for horizonal image in all_surfaces.jpg
	};



}


/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void URender(GLfloat shift, GLMesh meshName);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
void createSphere(GLfloat xOffset, GLuint sphereIndex);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);			// added for camera
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);		// added for camera
bool UCreateTexture(const char* filename, GLuint& textureId);						// added for texture
void UDestroyTexture(GLuint textureId);												// added for texture



/*
// Vertex Shader Source Code 
const GLchar* vertexShaderSource = GLSL(440,
	layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1
out vec4 vertexColor; // variable to transfer color data to the fragment shader

//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
	vertexColor = color; // references incoming color data
}
);
*/

/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
	layout(location = 0) in vec3 position;				// Vertex data from Vertex Attrib Pointer 0
	layout(location = 1) in vec4 color;					// Color data from Vertex Attrib Pointer 1
	layout(location = 2) in vec2 textureCoordinate;		// added for texture from image

	out vec4 vertexColor;								// variable to transfer color data to the fragment shader
	out vec2 vertexTextureCoordinate;					// coordinates for image

	// Global variables for the  transform matrices
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main()
	{
		gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
		vertexColor = color; // references incoming color data
		vertexTextureCoordinate = textureCoordinate;
	}
);

/*
// Fragment Shader Source Code
const GLchar* fragmentShaderSource = GLSL(440,
	in vec4 vertexColor; // Variable to hold incoming color data from vertex shader

out vec4 fragmentColor;

void main()
{
	fragmentColor = vec4(vertexColor);
}
);
*/

/* Fragment Shader Source Code */
const GLchar* fragmentShaderSource = GLSL(440,
	in vec4 vertexColor;					// Variable to hold incoming color data from vertex shader
	in vec2 vertexTextureCoordinate;		// added for texture assignment
	out vec4 fragmentColor;
	uniform sampler2D uTexture;				// added for texture assignment
	uniform vec2 uvScale;					// added for texture assignment
	void main()
	{
		fragmentColor = texture(uTexture, vertexTextureCoordinate * uvScale);		// added for texture
	}
);



// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it. Used for texture assignment 
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
	for (int j = 0; j < height / 2; ++j)
	{
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; --i)
		{
			unsigned char tmp = image[index1];
			image[index1] = image[index2];
			image[index2] = tmp;
			++index1;
			++index2;
		}
	}
}

// Function to append an array of GLfloat to the global GLfloat array meshVerts and increase meshSize
void appendArrayToGlobalArray(const GLfloat* localArray, int localArraySize)
{
	/* Used to add verts[] cerated in UCreateMesh() to global meshVerts[] */
	
	// Allocate memory for the new elements
	GLfloat* newArray = (GLfloat*)std::realloc(meshVerts, (meshSize + localArraySize) * sizeof(GLfloat));

	if (newArray) {
		// Memory reallocation was successful
		meshVerts = newArray;

		// Copy the new elements from the local array to the global array
		std::memcpy(meshVerts + meshSize, localArray, localArraySize * sizeof(GLfloat));
		meshSize += localArraySize;
	}
	else {
		// Memory reallocation failed
		std::cerr << "Memory reallocation failed." << std::endl;
		// Handle the error as needed
	}
}



// Function to append a single GLfloat value to the global GLfloat meshVerts and increase meshSize
void appendValueToGlobalArray(GLfloat value) {
	/* Used to add GLfloat values to global meshVerts[] in createSphere() */

	// Allocate memory for the new element in the global array
	GLfloat* newArray = (GLfloat*)std::realloc(meshVerts, (meshSize + 1) * sizeof(GLfloat));

	if (newArray) {
		// Memory reallocation was successful
		meshVerts = newArray;

		// Add the new value to the end of the global array
		meshVerts[meshSize] = value;
		meshSize++;
	}
	else {
		// Memory reallocation failed
		std::cerr << "Memory reallocation failed." << std::endl;
		// Handle the error as needed
	}
}


// Function to create a sphere with silver color
void createSphere(GLfloat xOffset, GLuint sphereIndex) {
	std::vector<Vertex> vertices;

	// Set up coordinates for mirror surface for texture (top horizonal image in all_surfaces.jpg)
	GLfloat sphereImageNumber = 5.0f;									// Indicates the 5th horizonal image, from the bottom-up
	GLfloat side_sphere = 0.0f;											// middle of horizonal image
	GLfloat height_sphere = (imageHeightSpacing * (sphereImageNumber - 1.0f) ) + 0.05f;  // 5% up from bottom of 5th image


	for (GLuint i = 0; i <= numStacks; ++i) {
		GLfloat phi = static_cast<GLfloat>(M_PI) / 2 - i * static_cast<GLfloat>(M_PI) / numStacks;
		GLfloat y = sphereRadius * std::sin(phi);

		for (GLuint j = 0; j <= numSlices; ++j) {
			GLfloat theta = j * 2 * static_cast<GLfloat>(M_PI) / numSlices;
			GLfloat x = sphereRadius * std::cos(phi) * std::cos(theta) + xOffset;
			GLfloat z = sphereRadius * std::cos(phi) * std::sin(theta);

			// Assign position to createSphere()
			Vertex vertex;
			vertex.position[0] = x;
			vertex.position[1] = y;
			vertex.position[2] = z;

			// Assign a color to the vertex - silver color
			vertex.color[0] = 0.49f;
			vertex.color[1] = 0.49f;
			vertex.color[2] = 0.49f;
			vertex.color[3] = 1.0f;

			// Assign an image text to the sphere - 5th horizonal image
			// Generate and Bind the Texture
			// Set Texture Parameters
			vertex.texCoord[0] = side_sphere;
			vertex.texCoord[1] = height_sphere;

			vertices.push_back(vertex);
		}
	}

	glGenVertexArrays(1, &sphereVao[sphereIndex]);
	glBindVertexArray(sphereVao[sphereIndex]);
	glGenBuffers(1, &sphereVbo[sphereIndex]);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVbo[sphereIndex]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	// Define the vertex attributes for vertex positions and colors (use for single and multiple spheres)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));		// position

//	glEnableVertexAttribArray(1);
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));		// color

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));		// texture
	glEnableVertexAttribArray(2);
}



// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
	// GLFW: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation
	// ---------------------
	*window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);

	/* Navigation Functions */
	// setup mouse input
	glfwSetCursorPosCallback(*window, UMousePositionCallback);
	glfwSetScrollCallback(*window, UMouseScrollCallback);
	// tell GLFW to capture our mouse
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// GLEW: initialize
	// ----------------
	// Note: if using GLEW version 1.13 or earlier
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult)
	{
		std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
		return false;
	}

	// Displays GPU OpenGL version
	cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

	return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
	static const float cameraSpeed = 2.5f;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);


///////////////		Works when commented out	///////////////////
	/* Used to switch from perspective or ortho views.
		Used two keys do to timing of a single key press being teated as multiple */
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		orthoView = false;
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		orthoView = true;
////////////////////////////////////////////////////////////////////


	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		//cout << "You pressed W! ";
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		//cout << "You pressed S! ";
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	};
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		//cout << "You pressed A! ";
		camera.ProcessKeyboard(LEFT, deltaTime);
	};
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		//cout << "You pressed D! ";
		camera.ProcessKeyboard(RIGHT, deltaTime);
	};
	// Add code to move the camera up and down the y-axis (had to add functionality in camera.h)
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		//cout << "You pressed Q! ";
		camera.ProcessKeyboard(UP, deltaTime);
	};
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		//cout << "You pressed E! ";
		camera.ProcessKeyboard(DOWN, deltaTime);
	};

	/* Added for Milestone 5 */
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && gTexWrapMode != GL_REPEAT)
	{
		glBindTexture(GL_TEXTURE_2D, gTextureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);
		gTexWrapMode = GL_REPEAT;
		cout << "Current Texture Wrapping Mode: REPEAT" << endl;
	}
	else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && gTexWrapMode != GL_MIRRORED_REPEAT)
	{
		glBindTexture(GL_TEXTURE_2D, gTextureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);
		gTexWrapMode = GL_MIRRORED_REPEAT;
		cout << "Current Texture Wrapping Mode: MIRRORED REPEAT" << endl;
	}
	else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_EDGE)
	{
		glBindTexture(GL_TEXTURE_2D, gTextureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		gTexWrapMode = GL_CLAMP_TO_EDGE;
		cout << "Current Texture Wrapping Mode: CLAMP TO EDGE" << endl;
	}
	else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_BORDER)
	{
		float color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
		glBindTexture(GL_TEXTURE_2D, gTextureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glBindTexture(GL_TEXTURE_2D, 0);
		gTexWrapMode = GL_CLAMP_TO_BORDER;
		cout << "Current Texture Wrapping Mode: CLAMP TO BORDER" << endl;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
	{
		gUVScale += 0.1f;
		cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
	{
		gUVScale -= 0.1f;
		cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
	}

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}



// Functioned called to render a frame
void URender(GLfloat shift, GLMesh meshName)
{
	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the frame and z buffers
//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// black background
	glClearColor(0.0f, 0.25f, 0.25f, 1.0f);	// turquoise background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	/* MVP */							// Added this section for Milestone 5 assignment
	// 1. Scales the object by 75%
	glm::mat4 scale = glm::scale(glm::vec3(0.75f, 0.75f, 0.75f));
	// 2. Rotates shape by 45 degrees in the y axis
	glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	// 3. Place object at the origin
	glm::mat4 translation = glm::translate(glm::vec3(shift, 0.0f, 0.0f));	// move to the left
	// Model matrix: transformations are applied right-to-left order
	glm::mat4 model = translation * rotation * scale;



	// camera/view transformation
	glm::mat4 view = camera.GetViewMatrix();


// Creates a perspective or orthographic projection
	glm::mat4 projection;
	if (orthoView == false) {
		// Creates a perspective projection - works & commented out for switching between views
		projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}
	else if (orthoView == true) {
		// Creates an orthographic projection - works & commented out for switching between views
		projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
	}


	// Set the shader to be used
	glUseProgram(gProgramId);

	// Retrieves and passes transform matrices to the Shader program
	GLint modelLoc = glGetUniformLocation(gProgramId, "model");
	GLint viewLoc = glGetUniformLocation(gProgramId, "view");
	GLint projLoc = glGetUniformLocation(gProgramId, "projection");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	// Used for texture
	GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));	
	
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(gMesh.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId);
	glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

	// Deactivate the Vertex Array Object 
	glBindVertexArray(0);


	/* Draw spheres */
	// Draw first sphere in chinese box
	glBindVertexArray(sphereVao[0]);	// use for first sphere (left side)
	glDrawArrays(GL_TRIANGLE_FAN, 0, (numSlices + 1) * 2 * (numStacks - 1) * 3);	// produces a sosphere
	// Draw second sphere in chinese box
	glBindVertexArray(sphereVao[1]);	// use for secnod sphere (right side)		
	glDrawArrays(GL_TRIANGLE_FAN, 0, (numSlices + 1) * 2 * (numStacks - 1) * 3);


	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.

}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
	GLfloat leftSide = -0.8f;
	GLfloat rightSide = 0.8f;
	GLfloat topSide = 0.15f;
	GLfloat bottomSide = -0.3f;
	GLfloat frontSide = rightSide / 2.0f;
	GLfloat backSide = leftSide / 2.0f;
	GLfloat topLidThickness = (topSide - bottomSide) / 2.0f;		// 50% height of box
	GLfloat topOfFlipLid = bottomSide + topLidThickness;			// up 50% in height from bottom of box
	GLfloat backOfFlipLid = backSide - (frontSide - backSide);

	GLfloat halfLengthPlane = (rightSide - leftSide) * 3.5f;
	GLfloat planeLeftSide = -halfLengthPlane;
	GLfloat planeRightSide = halfLengthPlane;
	GLfloat halfWidthPlane = halfLengthPlane / 2.0f;
	GLfloat planeFarSide = -halfWidthPlane;
	GLfloat planeNearSide = halfWidthPlane;


	/* 1st horizonal image: green fabric pattern for Front and Back of Chinese Box */
	GLfloat frontImageleftSide = 0.0f;
	GLfloat frontImageRightSide = 1.0f;
	GLfloat frontImageTopSide = imageHeightSpacing;													// top of 1st horizonal iamge
	GLfloat frontImageBottomSide = 0.0f;															// bottom of 1st horizonal image
	
	/* 2nd horizonal image: Red inside fabric of Chinese Box */
	GLfloat red_cloth_image_number = 2.0f;
	GLfloat insideImageleftSide = 0.0f;
	GLfloat insideImageRightSide = 1.0f;
	GLfloat insideImageBackSide = imageHeightSpacing * red_cloth_image_number;						// top of 2nd horizonal image
	GLfloat insideImageFrontSide = imageHeightSpacing + 0.01f;										// bottom of 2nd horizonal image
	
	/* 3rd horizonal image: sides, top, and bottom of Chinese Box */
	GLfloat left_right_side_box_image = 3.0f;														// this indicates the 3rd horizonal image (bottom-up)
	// Top and Bottom of Chinese Box
	GLfloat left_TopBottom = 0.0f;
	GLfloat right_TopBottom = 1.0f;
	GLfloat top_TopBottom = 1.0f;
	GLfloat bottom_TopBottom = imageHeightSpacing * left_right_side_box_image;
	// Left and Right Side of Chinese Box w/ open lid
	GLfloat side_ImageleftSide = 0.0f;
	GLfloat side_ImageRightSide = 0.5f;
	/* Divide 3rd image in half for lid */
	// Lower half of 3rd image
	GLfloat side_ImageBottomSide = (imageHeightSpacing * (left_right_side_box_image - 1.0f)) + 0.01f;			// bottom of bottom/lower 50% height of 3rd image
	GLfloat side_ImageTopSide = (imageHeightSpacing * (left_right_side_box_image - 1.0f)) + half_image_height;	// top of bottom/lower 50% height of 3rd image
	// Upper half of 3rd image:	Left and Right Side of top lid (upper 50% of side height)
	GLfloat sideTopLid_ImageTopSide = imageHeightSpacing * left_right_side_box_image;							// top of top/upper 50% of 3rd image		
	GLfloat sideTopLid_ImageBottomSide = (sideTopLid_ImageTopSide - half_image_height) + 0.01f;					// bottom of top/upper 50% of 3rd image
	
	/* 4th horizonal image: plane/tray texture */
	GLfloat tray_image = 4.0f;
	// Tray that all object rest on
	GLfloat left_side_tray = 0.0f;
	GLfloat right_side_tray = 1.0f;
	GLfloat near_side_tray = (imageHeightSpacing * (tray_image - 1.0f)) + .01f;				// bottom of 4th image
	GLfloat far_side_tray = imageHeightSpacing * tray_image;								// top of 4th image


	// Position and texture data (each rectangle is 2 triangles)
	GLfloat verts[] = {
		// Vertex Positions												// Texture Coordinates
		// Box for Chinese Meditation Balls
		leftSide, bottomSide, frontSide,								frontImageleftSide, frontImageBottomSide,		// bottom Left			Vertex 0 // Front Face of Box
		rightSide, bottomSide, frontSide,								frontImageRightSide, frontImageBottomSide,		// bottom Right			Vertex 1
		leftSide, topSide, frontSide,									frontImageleftSide, frontImageTopSide,			// top Left				Vertex 2
		rightSide, topSide, frontSide,									frontImageRightSide, frontImageTopSide, 		// top Right			Vertex 3
		leftSide, topSide, frontSide,									frontImageleftSide, frontImageTopSide,			// top Left				Vertex 2
		rightSide, bottomSide, frontSide,								frontImageRightSide, frontImageBottomSide,		// bottom Right			Vertex 1

		leftSide, bottomSide, backSide,									frontImageleftSide, frontImageBottomSide,		// bottom Left			Vertex 4 // Back Face of Box
		leftSide, topSide, backSide,									frontImageleftSide, frontImageTopSide,			// top Left				Vertex 5
		rightSide, topSide, backSide,									frontImageRightSide, frontImageTopSide, 		// top Right			Vertex 6
		rightSide, topSide, backSide,									frontImageRightSide, frontImageTopSide, 		// top Right			Vertex 6
		leftSide, bottomSide, backSide,									frontImageleftSide, frontImageBottomSide,		// bottom Left			Vertex 4
		rightSide, bottomSide, backSide,								frontImageRightSide, frontImageBottomSide, 		// bottom Right			Vertex 7

		leftSide, bottomSide, frontSide,								left_TopBottom, bottom_TopBottom,				// bottom Left			Vertex 0	// Bottom of Face
		leftSide, bottomSide, backSide,									left_TopBottom, top_TopBottom,					// bottom Left			Vertex 4
		rightSide, bottomSide, backSide,								right_TopBottom, top_TopBottom, 				// bottom Right			Vertex 7
		leftSide, bottomSide, frontSide,								left_TopBottom, bottom_TopBottom,				// bottom Left			Vertex 0
		rightSide, bottomSide, backSide,								right_TopBottom, top_TopBottom, 				// bottom Right			Vertex 7
		rightSide, bottomSide, frontSide,								right_TopBottom, bottom_TopBottom,				// bottom Right			Vertex 1

		rightSide, topOfFlipLid, backSide,								right_TopBottom, top_TopBottom,					// bottom close right	Vertex 12	// top of box (bottom of flip lid)
		rightSide, topOfFlipLid, backOfFlipLid,							right_TopBottom, bottom_TopBottom,				// bottom far right		Vertex 13
		leftSide, topOfFlipLid, backSide,								left_TopBottom, top_TopBottom,					// bottom close left	Vertex 15
		rightSide, topOfFlipLid, backOfFlipLid,							right_TopBottom, bottom_TopBottom,				// bottom far right		Vertex 13
		leftSide, topOfFlipLid, backSide,								left_TopBottom, top_TopBottom,					// bottom close left	Vertex 15
		leftSide, topOfFlipLid, backOfFlipLid,							left_TopBottom, bottom_TopBottom,				// bottom far left		Vertex 16

		leftSide, bottomSide, frontSide,								side_ImageRightSide, side_ImageBottomSide,		// bottom Left			Vertex 0	// left-side of Box
		leftSide, topSide, frontSide,									side_ImageRightSide, side_ImageTopSide,			// top Left				Vertex 2
		leftSide, topSide, backSide,									side_ImageleftSide, side_ImageTopSide,			// top Left				Vertex 5
		leftSide, bottomSide, frontSide,								side_ImageRightSide, side_ImageBottomSide,		// bottom Left			Vertex 0
		leftSide, topSide, backSide,									side_ImageleftSide, side_ImageTopSide,			// top Left				Vertex 5
		leftSide, bottomSide, backSide,									side_ImageleftSide, side_ImageBottomSide,		// bottom Left			Vertex 4

		rightSide, bottomSide, frontSide,								side_ImageleftSide, side_ImageBottomSide,		// bottom Right			Vertex 1	// right-side of box
		rightSide, topSide, frontSide,									side_ImageleftSide, side_ImageTopSide, 			// top Right			Vertex 3
		rightSide, topSide, backSide,									side_ImageRightSide, side_ImageTopSide, 		// top Right			Vertex 6
		rightSide, bottomSide, frontSide,								side_ImageleftSide, side_ImageBottomSide,		// bottom Right			Vertex 1
		rightSide, topSide, backSide,									side_ImageRightSide, side_ImageTopSide, 		// top Right			Vertex 6
		rightSide, bottomSide, backSide,								side_ImageRightSide, side_ImageBottomSide, 		// bottom Right			Vertex 7

		rightSide, topSide, backOfFlipLid,								side_ImageRightSide, sideTopLid_ImageBottomSide,	// top far right		Vertex 14	// flip lid right-side
		rightSide, topOfFlipLid, backSide,								side_ImageleftSide, sideTopLid_ImageTopSide,		// bottom close right	Vertex 12
		rightSide, topOfFlipLid, backOfFlipLid,							side_ImageRightSide, sideTopLid_ImageTopSide,		// bottom far right		Vertex 13
		rightSide, topSide, backOfFlipLid,								side_ImageRightSide, sideTopLid_ImageBottomSide,	// top far right		Vertex 14
		rightSide, topOfFlipLid, backSide,								side_ImageleftSide, sideTopLid_ImageTopSide,		// bottom close right	Vertex 12
		rightSide, topSide, backSide,									side_ImageleftSide,  sideTopLid_ImageBottomSide,	// top Right			Vertex 6

		leftSide, topOfFlipLid, backSide,								side_ImageRightSide, sideTopLid_ImageTopSide,		// bottom close left	Vertex 15	// flip lid left-side
		leftSide, topSide, backSide,									side_ImageRightSide,  sideTopLid_ImageBottomSide,	// top Left				Vertex 5
		leftSide, topOfFlipLid, backOfFlipLid,							side_ImageleftSide, sideTopLid_ImageTopSide,		// bottom far left		Vertex 16
		leftSide, topSide, backSide,									side_ImageRightSide,  sideTopLid_ImageBottomSide,	// top Left				Vertex 5
		leftSide, topSide, backOfFlipLid,								side_ImageleftSide,  sideTopLid_ImageBottomSide,	// top far left			Vertex 17
		leftSide, topOfFlipLid, backOfFlipLid,							side_ImageleftSide, sideTopLid_ImageTopSide,		// bottom far left		Vertex 16
		
		leftSide, topSide, backOfFlipLid,								left_TopBottom, sideTopLid_ImageBottomSide,			// top far left			Vertex 17	// flip lid back-side
		leftSide, topOfFlipLid, backOfFlipLid,							left_TopBottom, sideTopLid_ImageTopSide,			// bottom far left		Vertex 16
		rightSide, topOfFlipLid, backOfFlipLid,							right_TopBottom, sideTopLid_ImageTopSide,			// bottom far right		Vertex 13
		rightSide, topSide, backOfFlipLid,								right_TopBottom, sideTopLid_ImageBottomSide,		// top far right		Vertex 14
		rightSide, topOfFlipLid, backOfFlipLid,							right_TopBottom, sideTopLid_ImageTopSide,			// bottom far right		Vertex 13
		leftSide, topSide, backOfFlipLid,								left_TopBottom, sideTopLid_ImageBottomSide,			// top far left			Vertex 17

		leftSide + 0.01f, topOfFlipLid + 0.01f, backSide,				insideImageleftSide, insideImageFrontSide,		// close left corner	Vertex 18	// inside flip lid of box
		leftSide + 0.01f, topOfFlipLid + 0.01f, backOfFlipLid,			insideImageleftSide, insideImageBackSide,		// far left corner		Vertex 19
		rightSide - 0.01f, topOfFlipLid + 0.01f, backOfFlipLid,			insideImageRightSide, insideImageBackSide, 		// far right corner		Vertex 20
		leftSide + 0.01f, topOfFlipLid + 0.01f, backSide,				insideImageleftSide, insideImageFrontSide,		// close left corner	Vertex 18
		rightSide - 0.01f, topOfFlipLid + 0.01f, backOfFlipLid,			insideImageRightSide, insideImageBackSide, 		// far right corner		Vertex 20
		rightSide - 0.01f, topOfFlipLid + 0.01f, backSide,				insideImageRightSide, insideImageFrontSide, 	// close right corner	Vertex 21

		leftSide + 0.01f, (topSide / 3.0f) * 2.0f, backSide + 0.01f,	insideImageleftSide, insideImageBackSide, 		// back Left			Vertex 8	// shelf inside of Box
		leftSide + 0.01f, (topSide / 3.0f) * 2.0f, frontSide - .01f,	insideImageleftSide, insideImageFrontSide,		// front Left			Vertex 9
		rightSide - 0.01f, (topSide / 3.0f) * 2.0f, backSide + 0.01f,	insideImageRightSide, insideImageBackSide, 		// back Right			Vertex 10
		rightSide - 0.01f, (topSide / 3.0f) * 2.0f, frontSide - 0.01f,	insideImageRightSide, insideImageFrontSide, 	// front Right			Vertex 11
		rightSide - 0.01f, (topSide / 3.0f) * 2.0f, backSide + 0.01f,	insideImageRightSide, insideImageBackSide, 		// back Right			Vertex 10
		leftSide + 0.01f, (topSide / 3.0f) * 2.0f, frontSide - .01f,	insideImageleftSide, insideImageFrontSide,		// front Left			Vertex 9

		// Tray
		planeLeftSide, bottomSide - 0.01f, planeNearSide,				left_side_tray, near_side_tray,					// bottom Left			Vertex 22	// Plane (Tray holder)
		planeRightSide, bottomSide - 0.01f, planeNearSide,				right_side_tray, near_side_tray,				// bottom Right			Vertex 23
		planeLeftSide, bottomSide - 0.01f, planeFarSide,				left_side_tray, far_side_tray,					// top Left				Vertex 24
		planeRightSide, bottomSide - 0.01f, planeFarSide,				right_side_tray, far_side_tray,					// top Right			Vertex 25
		planeRightSide, bottomSide - 0.01f, planeNearSide,				right_side_tray, near_side_tray,				// bottom Right			Vertex 23
		planeLeftSide, bottomSide - 0.01f, planeFarSide,				left_side_tray, far_side_tray					// top Left				Vertex 24
	};


/////////////////////////////////////////////////////////////////////////////////////////////////////
	/* Add shapes to make objects by calling their functions */

	// Make spheres
	createSphere(-separation / 2.0f, 0);	// use for spheres with spacing (left sphere)
	createSphere(separation / 2.0f, 1);		// use for spheres with spacing (right sphere)


/////////////////////////////////////////////////////////////////////////////////////////////////////


	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerUV = 2;
	mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerUV));		// added for texture
	glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time - original w/o "&" in parameter
	glBindVertexArray(mesh.vao);


	// Create VBO
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU - original that works w/ non-texture spheres


	// Strides between vertex coordinates
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);	// Vertex position
	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
	glEnableVertexAttribArray(2);	// Vertex UV
}



void UDestroyMesh(GLMesh& mesh)
{
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(1, &mesh.vbo);

	// Delete the VBO and the VAO for the spheres - works for two spheres with spacing
	glDeleteVertexArrays(2, sphereVao);
	glDeleteBuffers(2, sphereVbo);
}


/* Generate and load the texture */
bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image)
	{
		flipImageVertically(image, width, height, channels);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		return true;
	}

	// Error loading the image
	return false;
}

void UDestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	// Create a Shader program object.
	programId = glCreateProgram();

	// Create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrive the shader source
	glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

	// Compile the vertex shader, and print compilation errors (if any)
	glCompileShader(vertexShaderId); // compile the vertex shader
	// check for shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		return false;
	}

	glCompileShader(fragmentShaderId); // compile the fragment shader
	// check for shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	// Attached compiled shaders to the shader program
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	glLinkProgram(programId);   // links the shader program
	// check for linking errors
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		return false;
	}

	glUseProgram(programId);    // Uses the shader program
	return true;
}


void UDestroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}







int main(int argc, char* argv[])
{
	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;

	// Create the mesh
	UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

	// Create the shader program
	if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
		return EXIT_FAILURE;

	// Load texture
	const char* texFilename = "all_surfaces.jpg";
	if (!UCreateTexture(texFilename, gTextureId))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}


	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	glUseProgram(gProgramId);
	// We set the texture as texture unit 0
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);


	// Sets the background color of the window to black (it will be implicitely used by glClear)
//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);		// black background
	glClearColor(0.0f, 0.25f, 0.25f, 1.0f);		// turquoise background



	// render loop
	// -----------
	while (!glfwWindowShouldClose(gWindow))
	{
		// per-frame timing
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;


		// input
		// -----
		UProcessInput(gWindow);

		// Render this frame (box & lid, with 2 spheres)
		URender(0.0f, gMesh);

		glfwPollEvents();
	}

	// Release mesh data
	UDestroyMesh(gMesh);

	
	// Release texture
	UDestroyTexture(gTextureId);


	// Release shader program
	UDestroyShaderProgram(gProgramId);

	exit(EXIT_SUCCESS); // Terminates the program successfully
}



