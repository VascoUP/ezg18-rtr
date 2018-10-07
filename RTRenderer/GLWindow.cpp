#include "GLWindow.h"

GLWindow::GLWindow()
{
	width = 800;
	height = 600;
}

GLWindow::GLWindow(GLint windowWidth, GLint windowHeight) {
	width = windowWidth;
	height = windowHeight;
}

int GLWindow::Initialize() {
	// Initialize GLFW
	if (!glfwInit()) {
		printf("GLFW initialization failed.");
		glfwTerminate();
		return 1;
	}

	// Setup GLFW window properties
	// OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Core profile = No Backwared Compatibility
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Allow forward compatibility
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLU_TRUE);

	mainWindow = glfwCreateWindow(width, height, "Test Window", NULL, NULL);
	if (!mainWindow) {
		printf("GLFW window creation failed.");
		glfwTerminate();
		return 1;
	}

	// Get Buffer size information
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	// Set context for glfw to use
	glfwMakeContextCurrent(mainWindow);

	// Allow modern extension features
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		printf("GLEW initialization failed");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST);

	// Setup viewport
	glViewport(0, 0, bufferWidth, bufferHeight);
}

GLfloat GLWindow::GetBufferWidht() {
	return bufferWidth;
}

GLfloat GLWindow::GetBufferHeight() {
	return bufferHeight;
}

bool GLWindow::GetShouldClose() {
	return glfwWindowShouldClose(mainWindow);
}

void GLWindow::SwapBuffers() {
	glfwSwapBuffers(mainWindow);
}

GLWindow::~GLWindow()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}
