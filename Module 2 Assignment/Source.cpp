
#include <glad/glad.h>      // glad library
#include <GLFW/glfw3.h>     // GLFW library
#include <glm/glm.hpp>		// glm library
#include <iostream>         // cout, cerr



// Create a Vertex Shader for the Triangle to be drawn
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec3 aColor;	// the color variable has attribute position 1
	out vec3 ourColor;						// output a color to the fragment shader
    void main()
    {
        gl_Position = vec4(aPos, 1.0);
		ourColor = aColor;					// set ourColor to the input color we got from the vertex data
    }
)";


// Create a Fragment Shader for the Triangle to be drawn
const char* fragmentShaderSource = R"(
	#version 330 core
	out vec4 FragColor;
	in vec3 ourColor;						// the input variable from the vertex shader (same name and same type)
	void main()
	{
		FragColor = vec4(ourColor, 1.0);	// set the fragment color
	}
)";




/* Declare Variables */
const unsigned int SCR_WIDTH = 800;				// Declare Window Width Size
const unsigned int SCR_HEIGHT = 600;			// Declare Window Height Size
const char* windowName = "Module 2 Assignment";	// Declare Window Name



// main function. Entry point to the OpenGL program
int main(void)
{


	// Set coordinates for 1st triangle
	GLfloat upperTriangle[] = {
		-1.0f, 1.0f, 0.0f,	// upper left-corner
		-1.0f, 0.0f, 0.0f,	// middle left-side
		-0.5f, 0.0f, 0.0f	// middle, between center and left-side
	};
	// Set colors for 1st triangle
	GLfloat upperTriangleColors[] = {
		1.0f, 0.0f, 0.0f,		// upper left-corner: red
		0.0f, 0.0f, 1.0f,		// middle left-side: blue
		0.0f, 1.0f, 0.0f		// middle, between center and left-side: green
	};
	

	// Set coordinates for 2nd triangle
	GLfloat lowerTriangle[] = {
		-0.5f, 0.0f, 0.0f,		// middle, between center and left-side
		0.0f, 0.0f, 0.0f,		// middle, center
		0.0f, -1.0f, 0.0f,		// middle, bottom
	};
	// Set colors for 2nd triangle
	GLfloat lowerTriangleColors[] = {
		0.0f, 1.0f, 0.0f,		// middle, between center and left-side: green
		1.0f, 0.0f, 0.0f,		// middle, center: red
		0.0f, 1.0f, 0.0f		// middle, bottom: green
	};




	/* Initialize the library */
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (!glfwInit())			// Initialize GLFW. If it fails, notify user
	{
		std::cout << "GLFW initialization failed" << std::endl;
		return -1;
	}
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif




	/* Create the Window Context */
	/* glfwCreateWindow (int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)	*/
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, windowName, NULL, NULL);
	if (!window)				// If window fails to be created, notify user
	{
		std::cout << "GLFW window creation failed" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);	// Make the window the current context
	gladLoadGL();  // glad: load all OpenGL function pointers - This is needs to be called before other functions or errors will occur
	/* glViewport — set the viewport (Tell OpenGL the size of the window)
		void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
			x, y = Specify the lower left corner of the viewport rectangle, in pixels. The initial value is (0,0).
//			width, height = Specify the width and height of the viewport.
			When a GL context is first attached to a window, width and height are set to the dimensions of that window.

		glViewport specifies the affine transformation of from normalized device coordinates to window coordinates.
			Let Xn Yn Zn Wn be the coordinates of a vertex specified in normalized device coordinates.
			Let Xw Yw Zw Ww be the resulting window coordinates. Then the viewport transformation is defined as follows:
				Xw = (x + 1) * width / 2 + x
				Yw = (y + 1) * height / 2 + y
				Zw = (Zn + 1) / 2
				Ww = Wn
		where x and y specify the lower left corner of the viewport rectangle,
			and width and height are the width and height of the viewport, respectively.
		Viewport width and height are silently clamped to a range that depends on the implementation.
		To query this range, call glGetIntegerv with arguments GL_MAX_VIEWPORT_DIMS.
			Note that the viewport transformation may also depend on the depth range.
		Viewport width and height are silently clamped to a range that depends on the implementation.
		To query this range, call glGetIntegerv with arguments GL_MAX_VIEWPORT_DIMS.
			Note that the viewport transformation may also depend on the depth range.
	*/
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);  // Specify the viewport of OpenGL window
	// adjust the viewport when the window is resized
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); });




	/* Create four VBOs (positions and colors of triangles) */
	unsigned int VBO[4];
	glGenBuffers(4, VBO);	// Generate VBO




	// Bind upper triangle position to VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);	// Bind VBO
	/* glBufferData — creates and initializes a buffer object's data store
			void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
		STREAM = The data store contents will be modified once and used at most a few times.
			STATIC = The data store contents will be modified once and used many times.
			DYNAMIC = The data store contents will be modified repeatedly and used many times.
			DRAW = The data store contents are modified by the application, and used as the source for GL drawing and image specification commands.
			READ = The data store contents are modified by reading data from the GL, and used to return that data when queried by the application.
			COPY = The data store contents are modified by reading data from the GL, and used as the source for GL drawing and image specification commands. */
	glBufferData(GL_ARRAY_BUFFER, sizeof(upperTriangle), upperTriangle, GL_STATIC_DRAW); // Copy vertices to VBO
	// Bind upper triangle color to VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);	// Bind VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(upperTriangleColors), upperTriangleColors, GL_STATIC_DRAW); // Copy vertices to VBO

	// Bind lower triangle position to VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);	// Bind VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(lowerTriangle), lowerTriangle, GL_STATIC_DRAW); // Copy vertices to VBO
	// Bind lower triangle color to VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);	// Bind VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(lowerTriangleColors), lowerTriangleColors, GL_STATIC_DRAW); // Copy vertices to VBO


	/* Create VAO */
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);	// Generate VAO
	glBindVertexArray(VAO);		// Bind VAO
	


	/* Create Vertex Shader object */
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);	// Attach vertex shader source code to vertex shader object
	glCompileShader(vertexShader); // Compile vertex shader
	// Check for compile time errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);	// Check if vertex shader compiled successfully
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);	// If vertex shader failed to compile, print error message
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}


	/* Create Fragment Shader */
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);


	/* Create Shader Program */
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();	// Create shader program
	glAttachShader(shaderProgram, vertexShader);	// Attach vertex shader to shader program
	glAttachShader(shaderProgram, fragmentShader);	// Attach fragment shader to shader program
	glLinkProgram(shaderProgram);	// Link shader program
	// Check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);	// Check if shader program linked successfully
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);	// If shader program failed to link, print error message
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}



	/* Loop until the user closes the window (Game Loop) */
	while (!glfwWindowShouldClose(window))
	{
		/* void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
			glClearColor specifies the red, green, blue, and alpha values used by glClear to clear fixed- and floating-point color buffers.
				Unsigned normalized fixed point RGBA color buffers are cleared to color values derived by clamping each component of the
				clear color to the range [0,1], then converting the (possibly sRGB converted and/or dithered) color to fixed-point.
		*/
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// Set back buffer color (background color) to black and opaque

		/*  clear buffers to preset values
			 glClear(GLbitfield mask);
			 mask = Bitwise OR of masks that indicate the buffers to be cleared.
				The three masks are GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, and GL_STENCIL_BUFFER_BIT.
		*/
		glClear(GL_COLOR_BUFFER_BIT);		// Render here. Clear the color buffer


		/* Set Up Vertex Attribute Pointers */
		// Set up upper triangle position attribute
		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// Set up upper triangle position attribute
		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		/* Draw upper triangles */
		glUseProgram(shaderProgram);		// Use shader program
		glBindVertexArray(VAO);				// Bind VAO
		glDrawArrays(GL_TRIANGLES, 0, 3);	// Draw triangle


		// Set up lower triangle position attribute
		glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);
		// Set up lower triangle color attribute
		glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
		/* glVertexAttribPointer - define an array of generic vertex attribute data
			Parameters:
			1st: Specifies the index of the generic vertex attribute to be modified.
				2nd: Specifies the number of components per generic vertex attribute. Must be 1, 2, 3, 4.
					Additionally, the symbolic constant GL_BGRA is accepted
				3rd: Specifies the data type of each component in the array.
					The symbolic constants GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, and GL_UNSIGNED_INT are
					accepted by glVertexAttribPointer and glVertexAttribIPointer. Additionally GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE,
					GL_FIXED, GL_INT_2_10_10_10_REV, GL_UNSIGNED_INT_2_10_10_10_REV and GL_UNSIGNED_INT_10F_11F_11F_REV are accepted
					by glVertexAttribPointer.
				4th: specifies whether fixed-point data values should be normalized (GL_TRUE)
					or converted directly as fixed-point values (GL_FALSE)
				5th: Specifies the byte offset between consecutive generic vertex attributes. If stride is 0, the generic
					vertex attributes are understood to be tightly packed in the array.
				6th: Specifies a offset of the first component of the first generic vertex attribute in the array in the data store
				of the buffer currently bound to the GL_ARRAY_BUFFER target. The initial value is 0. */
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(3);
		/* Draw lower triangles */
		glUseProgram(shaderProgram);		// Use shader program
		glBindVertexArray(VAO);				// Bind VAO
		glDrawArrays(GL_TRIANGLES, 0, 3);	// Draw triangle


		// Swap front and back buffers
		glfwSwapBuffers(window);


		// Poll to process events (window resizing)
		glfwPollEvents();
	}

	// Clean up
	glDeleteBuffers(4, VBO);				// Delete VBO
	glDeleteVertexArrays(1, &VAO);			// Delete VAO
	glDeleteProgram(shaderProgram);			// Delete shader program
	glfwDestroyWindow(window);				// Destroy window
	glfwTerminate();						// Terminate GLFW
	return 0;								// Exit program
}