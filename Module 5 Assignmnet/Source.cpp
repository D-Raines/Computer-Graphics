#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

/* add stb_image.h file from OpenGLSample folder to Module 5 Assignmnet folder*/
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions: added for module 5 assignment

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>


/* add camera header file : had to copy from OpenGLSample folder to Module 4 Assignmnet folder, then
	righ-click module 4 Assignment, select Add, then Existing Item. Then select the camera.h file */
#include "camera.h"		// added for module 4 assignment



using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif


// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "Module 5 Assignment"; // Macro for window title

	// Variables for window width and height
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;


	// camera - added for module 4 assignment
	Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
	float lastX = WINDOW_WIDTH / 2.0f;
	float lastY = WINDOW_HEIGHT / 2.0f;
	bool firstMouse = true;
	// timing
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;



	// Stores the GL data relative to a given mesh
/*
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint ebo;			// Handle for the element buffer object used for wireframe
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nIndices;    // Number of indices of the mesh
	};
*/
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbo;			// Handles for the vertex buffer object
		GLuint nVertices;   // Number of indices of the mesh
	};


	// Main GLFW window
	GLFWwindow* gWindow = nullptr;
	// Triangle mesh data
	GLMesh gMesh;

	// Texture: added for module 5 assignment
	GLuint gTextureId;
	glm::vec2 gUVScale(5.0f, 5.0f);
	GLint gTexWrapMode = GL_REPEAT;

	// Shader program
	GLuint gProgramId;

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
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);			// added for module 4 assignment
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);		// added for module 4 assignment
bool UCreateTexture(const char* filename, GLuint& textureId);						// added for module 5 assignment
void UDestroyTexture(GLuint textureId);												// added for module 5 assignment


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
	layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
	layout(location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1
	layout(location = 2) in vec2 textureCoordinate;		// added for module 5 assignment

	out vec4 vertexColor; // variable to transfer color data to the fragment shader
	out vec2 vertexTextureCoordinate;  // added for module 5 assignment

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


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
	in vec4 vertexColor;					// Variable to hold incoming color data from vertex shader
	in vec2 vertexTextureCoordinate;		// added for module 5 assignment

	out vec4 fragmentColor;

	uniform sampler2D uTexture;			// added for module 5 assignment
	uniform vec2 uvScale;				// added for module 5 assignment

	void main()
	{
//		fragmentColor = vec4(vertexColor);
		fragmentColor = texture(uTexture, vertexTextureCoordinate * uvScale);		// added for module 5 assignment
	}
);

/* Added for module 5 assignment */
// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
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

	// setup mouse input - added for module 4 assignment
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
	static const float cameraSpeed = 2.5f;	// add for module 5 assignment

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

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

	/* Added for Module 5 Assignment */
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





// Function called to render a frame
void URender(GLfloat shift, GLMesh meshName)
{
	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 1. Scales the object by 2
	glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
	// 2. Rotates shape by 45 degrees in the y axis
	glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(0.0f, 1.0f, 0.0f));	// rotate around the y axis 45 degrees and 13.5 degrees around the x axis
	// 3. Place object at the origin
	glm::mat4 translation = glm::translate(glm::vec3(shift, 0.0f, 0.0f));	// move to object along x-axis (left or right)
	// Model matrix: transformations are applied right-to-left order
	glm::mat4 model = translation * rotation * scale;

//	// Transforms the camera: move the camera back (z axis)
//	glm::mat4 view = glm::translate(glm::vec3(0.0f, 0.0f, -3.0f));				// commented out for module 5 assignment
	
	// camera/view transformation: added for module 5 assignment
	glm::mat4 view = camera.GetViewMatrix();

//	// Creates a orthographic projection
//	glm::mat4 projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);	// commented out for module 5 assignment

	// Creates a perspective projection
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);


	// Set the shader to be used
	glUseProgram(gProgramId);

	// Retrieves and passes transform matrices to the Shader program
	GLint modelLoc = glGetUniformLocation(gProgramId, "model");
	GLint viewLoc = glGetUniformLocation(gProgramId, "view");
	GLint projLoc = glGetUniformLocation(gProgramId, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	/* Added for Module 5 Assignment */
	GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

	// Activate the VBO and EBO contained within the wireframe's VAO
	glBindVertexArray(meshName.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);						// added for module 5 assignment
	glBindTexture(GL_TEXTURE_2D, gTextureId);			// added for module 5 assignment


	// Draws the triangles (used for solid pyramid)
//	glDrawElements(GL_TRIANGLES, meshName.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
	glDrawArrays(GL_TRIANGLES, 0, meshName.nVertices);	// Draws the triangle


	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
	// Specifies normalized device coordinates (x,y,z) and color for square vertices
	// Triangle mesh data
/*	Commented out for Module 5 assignment
	GLfloat verts[] = {
		// Vertex Positions		// Colors
		-0.25f, 0.0f, 0.25f,	1.0f, 0.0f, 0.0f, 1.0f,		// left-front-corner: red			indice	0
		0.25f, 0.0f, 0.25f,		0.0f, 1.0f, 0.0f, 1.0f,		// right-front-corner: green		indice	1
		0.0f, 0.5f, 0.0f,		0.0f, 0.0f, 1.0f, 1.0f,		// center tip:	blue				inidce	2
		-0.25f, 0.0f, -0.25f,	0.0f, 1.0f, 0.0f, 1.0f,		// left-back-corner: green			indice	3
		0.25f, 0.0f, -0.25f,	1.0f, 0.0f, 0.0f, 1.0f		// right-back-corner: red			indice	4
	};

	//	Commented out for Module 5 assignment
	GLfloat verts[] = {
		// Vertex Positions		// Colors
		-0.25f, 0.0f, 0.25f,	0.0f, 0.0f,					// left-front-corner				indice	0
		0.25f, 0.0f, 0.25f,		1.0f, 0.0f,					// right-front-corner				indice	1
		0.0f, 0.5f, 0.0f,		0.5f, 1.0f,					// center tip						inidce	2
		-0.25f, 0.0f, -0.25f,	0.0f, 0.0f,					// left-back-corner					indice	3
		0.25f, 0.0f, -0.25f,	1.0f, 0.0f					// right-back-corner				indice	4
	};
*/

	GLfloat verts[] = {
		// Vertex Positions		// Texture Coordinates
		// Front Face
		-0.25f, 0.0f, 0.25f,	0.0f, 0.0f,					// left-front-corner
		0.25f, 0.0f, 0.25f,		1.0f, 0.0f,					// right-front-corner
		0.0f, 0.5f, 0.0f,		0.5f, 1.0f,					// center tip
		// Rear Face
		-0.25f, 0.0f, -0.25f,	0.0f, 0.0f,					// left-back-corner
		0.25f, 0.0f, -0.25f,	1.0f, 0.0f,					// right-back-corner
		0.0f, 0.5f, 0.0f,		0.5f, 1.0f,					// center tip
		// Left Face
		-0.25f, 0.0f, -0.25f,	0.0f, 0.0f,					// left-back-corner
		-0.25f, 0.0f, 0.25f,	1.0f, 0.0f,					// left-front-corner
		0.0f, 0.5f, 0.0f,		0.5f, 1.0f,					// center tip
		// Right Face
		0.25f, 0.0f, -0.25f,	0.0f, 0.0f,					// right-back-corner
		0.25f, 0.0f, 0.25f,		1.0f, 0.0f,					// right-front-corner
		0.0f, 0.5f, 0.0f,		0.5f, 1.0f,					// center tip
		// Bottom back triangle
		-0.25f, 0.0f, -0.25f,	0.0f, 1.0f,					// left-back-corner
		0.25f, 0.0f, -0.25f,	1.0f, 1.0f,					// right-back-corner
		0.25f, 0.0f, 0.25f,		1.0f, 0.0f,					// right-front-corner
		// Bottom front triangle
		-0.25f, 0.0f, 0.25f,	0.0f, 0.0f,					// left-front-corner
		-0.25f, 0.0f, -0.25f,	0.0f, 1.0f,					// left-back-corner
		0.25f, 0.0f, 0.25f,		1.0f, 0.0f					// right-front-corner

	};

/*
	// Index data to share position data
	GLushort indices[] = {
		0, 1, 2,  // Triangle 1
		2, 3, 4,  // Triangle 2
		0, 1, 3,  // Triangle 3
		1, 3, 4,  // Triangle 4
		1, 2, 4,  // Triangle 5
		0, 2, 3   // Triangle 6
	};
*/

	const GLuint floatsPerVertex = 3;
//	const GLuint floatsPerColor = 4;		// commented out for module 5 assignment
	const GLuint floatsPerUV = 2;			// added for module 5 assignment
//	mesh.nIndices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerUV));		// added for module 5 assignment
	mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerUV));		// added for module 5 assignment

	glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(mesh.vao);

/*	// used when there are indices
	// Create 2 buffers: first one for the vertex data; second one for the indices
	glGenBuffers(2, mesh.vbos);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU
	mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
*/
	
	// Create VBO
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU


//	// Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
//	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each	// commented out for module 5 assignment
	// Strides between vertex coordinates
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
//	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}

void UDestroyMesh(GLMesh& mesh)
{
	/*
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(2, mesh.vbos);
	glDeleteBuffers(1, &mesh.ebo);
	*/

	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(1, &mesh.vbo);
}


/* Generate and load the texture: added for module 5 assignment */
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
	UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object used for 3D pyramid

	// Create the shader program
	if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
		return EXIT_FAILURE;



	/* Added for Module 5 Assignment */
	// Load texture
//	const char* texFilename = "../../resources/textures/smiley.png";
	const char* texFilename = "brick_image_2.jpg";
//	const char* texFilename = "smiley.png";
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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

		// Render this frame (3D pyramid)
		URender(0.0f, gMesh); // center and render the 3D pyramid



		glfwPollEvents();
	}

	// Release mesh data
	UDestroyMesh(gMesh);

	// Release texture: added for module 5 assignment
	UDestroyTexture(gTextureId);


	// Release shader program
	UDestroyShaderProgram(gProgramId);


	exit(EXIT_SUCCESS); // Terminates the program successfully
}