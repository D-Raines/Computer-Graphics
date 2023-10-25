#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library


/* add camera header file : had to copy from OpenGLSample folder to Module 4 Assignmnet folder, then
	righ-click module 4 Assignment, select Add, then Existing Item. Then select the camera.h file.
	Then right-click on project, select properties, go to C\C++ under configuration --> General --> Additional Include Directories, 
		then add project folder
*/
#include "camera.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>


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
	const char* const WINDOW_TITLE = "Milestone 4"; // Macro for window title

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

	
	// Used to set the projection to perspective or orthographic
	bool orthoView = false;	// false = perspective, true = orthographic


	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nIndices;    // Number of indices of the mesh
	};

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;

	// Triangle mesh data
	GLMesh gMesh;
	
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
		GLfloat color[4];
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



/* Vertex Shader Source Code*/
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


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
	in vec4 vertexColor; // Variable to hold incoming color data from vertex shader

	out vec4 fragmentColor;

	void main()
	{
		fragmentColor = vec4(vertexColor);
	}
);



// Function to create a sphere
void createSphere(GLfloat xOffset, GLuint sphereIndex) {
	std::vector<Vertex> vertices;

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));


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
	* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);

	/* Navigation Functions */
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

	/* Used to switch from perspective or ortho views.
		Used two keys do to timing of a single key press being teated as multiple */
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		orthoView = false;
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		orthoView = true;

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



// Functioned called to render a frame
//void URender()
void URender(GLfloat shift, GLMesh meshName)
{
	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the frame and z buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// camera/view transformation
	glm::mat4 view = camera.GetViewMatrix();


	// Creates a perspective projection
	glm::mat4 projection;
	if (orthoView == false){
		// Creates a perspective projection - works & commented out for switching between views
		projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}
	else if (orthoView == true){
		// Creates an orthographic projection - works & commented out for switching between views
		projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
	}


	// Set the shader to be used
	glUseProgram(gProgramId);

	// Retrieves and passes transform matrices to the Shader program
	GLint modelLoc = glGetUniformLocation(gProgramId, "model");
	GLint viewLoc = glGetUniformLocation(gProgramId, "view");
	GLint projLoc = glGetUniformLocation(gProgramId, "projection");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(gMesh.vao);


	/* MVP */
	// 1. Scales the object by 75%
	glm::mat4 scale = glm::scale(glm::vec3(0.75f, 0.75f, 0.75f));
	// 2. Rotates shape by 45 degrees in the y axis
	glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	// 3. Place object at the origin
	glm::mat4 translation = glm::translate(glm::vec3(shift, 0.0f, 0.0f));	// move to the left
	// Model matrix: transformations are applied right-to-left order
	glm::mat4 model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


	// Draws the Box and lid
	glDrawElements(GL_TRIANGLES, gMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

	// Draw spheres
	glBindVertexArray(sphereVao[0]);	// use for first sphere (left side)
	glDrawArrays(GL_TRIANGLE_FAN, 0, (numSlices + 1) * 2 * (numStacks - 1) * 3);	// produces a solid sphere
	glBindVertexArray(sphereVao[1]);	// use for secnod sphere (right side)		
	glDrawArrays(GL_TRIANGLE_FAN, 0, (numSlices + 1) * 2 * (numStacks - 1) * 3);


	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.

}


// Implements the UCreateMesh function - create the box and lid, and plane (tray)
void UCreateMesh(GLMesh& mesh)
{
	GLfloat leftSide = -0.8f;
	GLfloat rightSide = 0.8f;
	GLfloat topSide = 0.15f;
	GLfloat bottomSide = -0.3f;
	GLfloat frontSide = rightSide / 2.0f;
	GLfloat backSide = leftSide / 2.0f;
	GLfloat topLidThickness = (topSide - bottomSide) / 2.0f;
	GLfloat topOfFlipLid = bottomSide + topLidThickness;
	GLfloat backOfFlipLid = backSide - (frontSide - backSide);

	GLfloat halfLengthPlane = (rightSide - leftSide) * 3.5f;
	GLfloat planeLeftSide = -halfLengthPlane;
	GLfloat planeRightSide = halfLengthPlane;
	GLfloat halfWidthPlane = halfLengthPlane / 2.0f;
	GLfloat planeFarSide = -halfWidthPlane;
	GLfloat planeNearSide = halfWidthPlane;


	// Position and Color data
	GLfloat verts[] = {
		// Vertex Positions												// Colors (r,g,b,a)
		// Box for Chinese Meditation Balls
		leftSide, bottomSide, frontSide,								0.0f, 1.0f, 0.0f, 1.0f,		// bottom Left			Vertex 0 // Front Face of Box
		rightSide, bottomSide, frontSide,								1.0f, 0.0f, 0.0f, 1.0f,		// bottom Right			Vertex 1
		leftSide, topSide, frontSide,									0.1f, 0.1f, 0.5f, 1.0f,		// top Left				Vertex 2
		rightSide, topSide, frontSide,									0.5f, 0.5f, 0.5f, 1.0f,		// top Right			Vertex 3
		leftSide, bottomSide, backSide,									0.25f, 0.25f, 0.5f, 1.0f,	// bottom Left			Vertex 4 // Back Face of Box
		leftSide, topSide, backSide,									0.5f, 1.0f, 0.5f, 1.0f,		// top Left				Vertex 5
		rightSide, topSide, backSide,									0.0f, 0.5f, 1.0f, 1.0f,		// top Right			Vertex 6
		rightSide, bottomSide, backSide,								1.0f, 1.0f, 0.0f, 1.0f,		// bottom Right			Vertex 7
		leftSide + 0.01f, (topSide / 3.0f) * 2.0f, backSide + 0.01f,	1.0f, 0.0f, 1.0f, 1.0f,		// back Left			Vertex 8 // Top inside shelf of Box
		leftSide + 0.01f, (topSide / 3.0f) * 2.0f, frontSide - .01f,	1.0f, 0.0f, 1.0f, 1.0f,		// front Left			Vertex 9
		rightSide - 0.01f, (topSide / 3.0f) * 2.0f, backSide + 0.01f,	1.0f, 0.0f, 1.0f, 1.0f,		// back Right			Vertex 10
		rightSide - 0.01f, (topSide / 3.0f) * 2.0f, frontSide - 0.01f,	1.0f, 0.0f, 1.0f, 1.0f,		// front Right			Vertex 11
		rightSide, topOfFlipLid, backSide,								0.0f, 0.5f, 1.0f, 1.0f,		// bottom close right	Vertex 12	// flip lid of box
		rightSide, topOfFlipLid, backOfFlipLid,							0.0f, 0.25f, 1.0f, 1.0f,	// bottom far right		Vertex 13
		rightSide, topSide, backOfFlipLid,								0.0f, 0.5f, 0.5f, 1.0f,		// top far right		Vertex 14
		leftSide, topOfFlipLid, backSide,								0.5f, 1.0f, 0.5f, 1.0f,		// bottom close left	Vertex 15
		leftSide, topOfFlipLid, backOfFlipLid,							0.0f, 0.0f, 1.0f, 1.0f,		// bottom far left		Vertex 16
		leftSide, topSide, backOfFlipLid,								0.0f, 0.1f, 0.5f, 1.0f,		// top far left			Vertex 17
		leftSide + 0.01f, topOfFlipLid + 0.01f, backSide,				1.0f, 0.0f, 1.0f, 1.0f,		// close left corner	Vertex 18	// inside flip lid of box
		leftSide + 0.01f, topOfFlipLid + 0.01f, backOfFlipLid,			1.0f, 0.0f, 1.0f, 1.0f,		// far left corner		Vertex 19
		rightSide - 0.01f, topOfFlipLid + 0.01f, backOfFlipLid,			1.0f, 0.0f, 1.0f, 1.0f,		// far right corner		Vertex 20
		rightSide - 0.01f, topOfFlipLid + 0.01f, backSide,				1.0f, 0.0f, 1.0f, 1.0f,		// close right corner	Vertex 21

		// plane
		planeLeftSide, bottomSide - 0.01f, planeNearSide,				0.0f, 0.2f, 0.3f, 1.0f,		// bottom Left			Vertex 22 // Plane (Tray holder)
		planeRightSide, bottomSide - 0.01f, planeNearSide,				0.0f, 0.2f, 0.3f, 1.0f,		// bottom Right			Vertex 23
		planeLeftSide, bottomSide - 0.01f, planeFarSide,					0.0f, 0.2f, 0.3f, 1.0f,	// top Left				Vertex 24
		planeRightSide, bottomSide - 0.01f, planeFarSide,				0.0f, 0.2f, 0.3f, 1.0f,		// top Right			Vertex 25
	};



	// Index data to share position data
	GLushort indices[] = {
		0, 1, 2,	// Triangle 1: Front Face
		1, 2, 3,	// Triangle 2: Front Face
		4, 5, 7,	// Triangle 3: Back Face
		5, 6, 7,	// Triangle 4: Back Face
		0, 2, 5,	// Triangle 5: left Face
		0, 4, 5,	// Triangle 6: left Face
		0, 1, 4,	// Triangle 7: bottom Face
		1, 4, 7,	// Triangle 8: bottom Face
		1, 3, 6,	// Triangle 9: right Face
		1, 6, 7,	// Triangle 10: right Face
		8, 9, 10,	// Triangle 11: inside box top
		9, 10, 11,	// Triangle 12: inside box top
		6, 12, 14,	// Triangle 13: flip lid
		12, 13, 14,	// Triangle 14: flip lid
		13, 14, 16, // Triangle 15: flip lid
		14, 16, 17, // Triangle 16: flip lid
		5, 15, 16,	// Triangle 17: flip lid
		5, 16, 17,	// Triangle 18: flip lid
		12, 13, 16, // Triangle 19: flip lid
		12, 15, 16,	// Triangle 20: flip lid
		18, 19, 20, // Triangle 21: inside flip lid
		18, 20, 21, // Triangle 22: inside flip lid
		22, 24, 25, // Triangle 23: plane
		22, 23, 25, // Triangle 24: plane
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
	glEnableVertexAttribArray(0);	// Vertex position

	glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);	// Vertex color
}



void UDestroyMesh(GLMesh& mesh)
{
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(2, mesh.vbos);


	// Delete the VBO and the VAO for the spheres - works for two spheres with spacing
	glDeleteVertexArrays(2, sphereVao);
	glDeleteBuffers(2, sphereVbo);


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


	// Create the spheres
	createSphere(-separation / 2.0f, 0);	// use for two spheres with spacing (left sphere)
	createSphere(separation / 2.0f, 1);		// use for two spheres with spacing (right sphere)



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

		// Render this frame (box & lid, with 2 spheres)
		URender(0.0f, gMesh);

		glfwPollEvents();
	}

	// Release mesh data
	UDestroyMesh(gMesh);

	// Release shader program
	UDestroyShaderProgram(gProgramId);

	exit(EXIT_SUCCESS); // Terminates the program successfully
}



