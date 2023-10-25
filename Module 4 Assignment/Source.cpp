#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

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
	const char* const WINDOW_TITLE = "Module 4 Assignment"; // Macro for window title

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
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint ebo;			// Handle for the element buffer object used for wireframe
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nIndices;    // Number of indices of the mesh
	};



	// Main GLFW window
	GLFWwindow* gWindow = nullptr;
	// Triangle mesh data
	GLMesh gMesh;
	GLMesh gWireframe;
	// Shader program
	GLuint gProgramId;
	// Shader program wireframe
	GLuint gWireframeProgramId;
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
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);	// added for module 4 assignment


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1
out vec4 vertexColor; // variable to transfer color data to the fragment shader
// Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
	vertexColor = color; // references incoming color data
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
in vec4 vertexColor; // Variable to hold incoming color data from vertex shader
out vec4 fragmentColor;
void main()
{
	fragmentColor = vec4(vertexColor);
}
);



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
//void URender()
void URender(GLfloat shift, GLMesh meshName)
{
	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// camera/view transformation - added for module 4 assignment
	glm::mat4 view = camera.GetViewMatrix();

/*		commented out for module 4 assignment
	// 1. Scales the object by 2
//	glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
	glm::mat4 scale = glm::scale(glm::vec3(4.0f, 4.0f, 4.0f));	// Scale by 4
	// 2. Rotates shape by 15 degrees in the x axis
	glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(0.3f, 1.0f, 0.0f));	// rotate around the y axis 45 degrees and 13.5 degrees around the x axis
	// 3. Place object at the origin
//	glm::mat4 translation = glm::translate(glm::vec3(-2.0f, 0.0f, 0.0f));	// move to the left
	glm::mat4 translation = glm::translate(glm::vec3(shift, 0.0f, 0.0f));	// move to the left
	// Model matrix: transformations are applied right-to-left order
	glm::mat4 model = translation * rotation * scale;

	// Transforms the camera: move the camera back (z axis)
	glm::mat4 view = glm::translate(glm::vec3(0.0f, 0.0f, -3.0f));	// move camera up (positve z-axis is like walking backwards)

	// Creates a orthographic projection
	glm::mat4 projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
*/

	// Creates a perspective projection
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);


	// Set the shader to be used
	glUseProgram(gProgramId);

	// Retrieves and passes transform matrices to the Shader program
	GLint modelLoc = glGetUniformLocation(gProgramId, "model");
	GLint viewLoc = glGetUniformLocation(gProgramId, "view");
	GLint projLoc = glGetUniformLocation(gProgramId, "projection");

//	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));	// commented out for module 4 assignment
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Activate the VBO and EBO contained within the wireframe's VAO
	glBindVertexArray(meshName.vao);


	/* Added for module 4 */
	// 1. Scales the object by 2
	glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
	// 2. Rotates shape by 45 degrees in the y axis
	glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	// 3. Place object at the origin
	glm::mat4 translation = glm::translate(glm::vec3(shift, 0.0f, 0.0f));	// move to the left
	// Model matrix: transformations are applied right-to-left order
	glm::mat4 model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles (used for solid pyramid)
	glDrawElements(GL_TRIANGLES, meshName.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle



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
	GLfloat verts[] = {
		// Vertex Positions		// Colors
		-0.25f, 0.0f, 0.25f,	1.0f, 0.0f, 0.0f, 1.0f,		// left-front-corner: red			indice	0
		0.25f, 0.0f, 0.25f,		0.0f, 1.0f, 0.0f, 1.0f,		// right-front-corner: green		indice	1
		0.0f, 0.5f, 0.0f,		0.0f, 0.0f, 1.0f, 1.0f,		// center tip:	blue				inidce	2
		-0.25f, 0.0f, -0.25f,	0.0f, 1.0f, 0.0f, 1.0f,		// left-back-corner: green			indice	3
		0.25f, 0.0f, -0.25f,	1.0f, 0.0f, 0.0f, 1.0f		// right-back-corner: red			indice	4
	};

	// Index data to share position data
	GLushort indices[] = {
		0, 1, 2,  // Triangle 1
		2, 3, 4,  // Triangle 2
		0, 1, 3,  // Triangle 3
		1, 3, 4,  // Triangle 4
		1, 2, 4,  // Triangle 5
		0, 2, 3   // Triangle 6
	};


	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerColor = 4;
	glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(mesh.vao);

	// Create 2 buffers: first one for the vertex data; second one for the indices
	glGenBuffers(2, mesh.vbos);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU
	mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);
}


void UDestroyMesh(GLMesh& mesh)
{
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(2, mesh.vbos);
	glDeleteBuffers(1, &mesh.ebo);
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
	UDestroyMesh(gWireframe);

	// Release shader program
	UDestroyShaderProgram(gProgramId);
	UDestroyShaderProgram(gWireframeProgramId);

	exit(EXIT_SUCCESS); // Terminates the program successfully
}