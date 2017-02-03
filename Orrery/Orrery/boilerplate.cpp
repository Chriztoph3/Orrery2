// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
//	Edited by Christopher Barber 10110661 April 2016
// ==========================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <vector>
#include <algorithm>
#include <math.h>
#include "glm\glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Camera.h"
#include "structs.h"
#include "glcorearb.h"
#include "soil/SOIL.h"

// specify that we want the OpenGL core profile before including GLFW headers



#ifdef _WIN32
#include "glew-2.0.0\glew-2.0.0\include\GL\glew.h"
#include "glew-2.0.0\glew-2.0.0\src\glew.c"

#else
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#endif 
#include "GLFW/include/GLFW/glfw3.h"
#include "glad\include\glad\glad.h"
using namespace std;

// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

//global variables

MyTexture earthTexture;
MyTexture sunTexture;
MyTexture moonTexture;
MyTexture starTexture;
MyShader shader;
MyGeometry sphere;

float timeScale = 100000.0f;
float sizeScale = 10000000.0f;
bool isRotating = false;
float deltaTime = 0.0f;
const float MOUSE_SENSITIVITY = 1.0f/100.0f;
double oldXPos;
double oldYPos;
const string texturePath = "./SolarSystem/";
const float WINDOW_WIDTH = 1024;
const float WINDOW_HEIGHT = 1024;

bool isPaused = false;

Camera camera(45.0f, WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100000.0f);

float ChangeRadiusScale(float radius)
{
	float r = powf(log2(radius) / 20, 2);
	return r;
}

float ChangeDistanceScale(float distance, float scale, float offset)
{
	float d = distance / scale + offset;
}




bool InitializeTexture(MyTexture *texture, const string &imageFileName)
{
	int w, h;
	unsigned char *pixels = SOIL_load_image((texturePath+imageFileName).c_str(), &w, &h, 0, SOIL_LOAD_RGB);

	// SOIL_load_image will return NULL if it fails
	if (!pixels) {
		return false;
	}

    // store the image width and height into the texture structure
    texture->width = w;
    texture->height = h;

    // create a texture name to associate our image data with
    if (!texture->textureName)
        glGenTextures(1, &texture->textureName);

    // bind the texture as a "rectangle" to access using image pixel coordinates
    glBindTexture(GL_TEXTURE_2D, texture->textureName);

    // send image pixel data to OpenGL texture memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->width, texture->height,
                 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
                 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);

    // unbind this texture
    glBindTexture(GL_TEXTURE_2D, 0);

	SOIL_free_image_data(pixels);

    return !CheckGLErrors();
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering



// load, compile, and link shaders, returning true if successful
bool InitializeShaders(MyShader *shader, string vertexShader, string fragmentShader)
{
    // load shader source from files
    string vertexSource = LoadSource(vertexShader);
    string fragmentSource = LoadSource(fragmentShader);
    if (vertexSource.empty() || fragmentSource.empty()) return false;

    // compile shader source into shader objects
    shader->vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
    shader->fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    // link shader program
    shader->program = LinkProgram(shader->vertex, shader->fragment);

    // check for OpenGL errors and return false if error occurred
    return !CheckGLErrors();
}

// deallocate shader-related objects
void DestroyShader(MyShader *shader)
{
    // unbind any shader programs and destroy shader objects
    glUseProgram(0);
    glDeleteProgram(shader->program);
    glDeleteShader(shader->vertex);
    glDeleteShader(shader->fragment);
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data



bool InitializeSphere(MyGeometry *geometry, int latEdges, int longEdges)
{
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> vertexCoords;
	
	float dTheta = (2 * 3.1415926535) / longEdges;
	float dPhi = 3.1415926535 / latEdges;
	
	for (int i = 0; i < latEdges; i++)
	{
		// angles in radians of latitude points on sphere
		float p1 = 3.1415926535*2 - ((i + 1) * dPhi);
		float p2 = p1 + dPhi;
		
		// v texture coords for points on sphere
		float v1 = 1 - (p1 + 3.1415926535*2) / 3.1415926535;
		float v2 = 1 - (p2 + 3.1415926535*2) / 3.1415926535;
		
		for (int j = 0; j < longEdges; j++)
		{
			// angles in radians of longitude points on sphere
			float t1 = j * dTheta;
			float t2 = t1 + dTheta;
			
			// u texture coords for points on sphere
			float u1 = t1 / (2 * 3.1415926535);
			float u2 = t2 / (2 * 3.1415926535);
			
			float x1 = cos(p1) * sin(t1);
			float z1 = cos(p1) * cos(t1);
			float y1 = sin(p1);
			
			float x2 = cos(p2) * sin(t1);
			float z2 = cos(p2) * cos(t1);
			float y2 = sin(p2);
			
			float x3 = cos(p2) * sin(t2);
			float z3 = cos(p2) * cos(t2);
			float y3 = sin(p2);
			
			float x4 = cos(p1) * sin(t2);
			float z4 = cos(p1) * cos(t2);
			float y4 = sin(p1);
			
			// add first triangle
			vertices.push_back(x1); vertices.push_back(y1); vertices.push_back(z1);
			vertices.push_back(x2); vertices.push_back(y2); vertices.push_back(z2); 
			vertices.push_back(x4); vertices.push_back(y4); vertices.push_back(z4);
			vertexCoords.push_back(u1); vertexCoords.push_back(v1);
			vertexCoords.push_back(u1); vertexCoords.push_back(v2);
			vertexCoords.push_back(u2); vertexCoords.push_back(v1);
			
			// add second triangle
			vertices.push_back(x4); vertices.push_back(y4); vertices.push_back(z4);
			vertices.push_back(x2); vertices.push_back(y2); vertices.push_back(z2);
			vertices.push_back(x3); vertices.push_back(y3); vertices.push_back(z3);
			vertexCoords.push_back(u2); vertexCoords.push_back(v1);
			vertexCoords.push_back(u1); vertexCoords.push_back(v2);
			vertexCoords.push_back(u2); vertexCoords.push_back(v2);
		}
	}
	
	geometry->elementCount = vertices.size()/3;

    // these vertex attribute indices correspond to those specified for the
    // input variables in the vertex shader
    const GLuint VERTEX_INDEX = 0;
    const GLuint VERTEX_COORDS_INDEX = 1;
    const GLuint VERTEX_NORMAL_INDEX = 2;

    //-----------
    // add texture index
    //-----------

    // create an array buffer object for storing our vertices
    glGenBuffers(1, &geometry->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    // create another one for storing our vertexcoords
    glGenBuffers(1, &geometry->textureCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->textureCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.size() * sizeof(GLfloat), &vertexCoords[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &geometry->normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
    //-------------------------
    // generate bind and buffer texture coordinate data
    //-------------------------

    // create a vertex array object encapsulating all our vertex attributes
    glGenVertexArrays(1, &geometry->vertexArray);
    glBindVertexArray(geometry->vertexArray);

    // associate the position array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glVertexAttribPointer(VERTEX_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(VERTEX_INDEX);
    
    glBindBuffer(GL_ARRAY_BUFFER, geometry->normalBuffer);
    glVertexAttribPointer(VERTEX_NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(VERTEX_NORMAL_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, geometry->textureCoordBuffer);
    glVertexAttribPointer(VERTEX_COORDS_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(VERTEX_COORDS_INDEX);

    // assocaite the colour array with the vertex array object
    
    //-----------------
    // Set up vertex attribute info for textures
    //-----------------

    // unbind our buffers, resetting to default state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // check for OpenGL errors and return false if error occurred
    return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(MyGeometry *geometry)
{
    // unbind and destroy our vertex array object and associated buffers
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &geometry->vertexArray);
    glDeleteBuffers(1, &geometry->vertexBuffer);
    glDeleteBuffers(1, &geometry->textureCoordBuffer);
    glDeleteBuffers(1, &geometry->normalBuffer);
}


void DestroyTextures(MyTexture *texture)
{
	glDeleteTextures(1, &texture->textureName);
}



void InitGL()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(1.0, 0.0, 0.0, 1.0);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(Planet *planet, MyShader *shader, glm::mat4 P, bool isStar)
{
	glm::mat4 projectionMatrix = camera.GetProjectionMatrix();
	glm::mat4 viewMatrix = camera.GetViewMatrix();
	
	glm::mat4 Rl = planet->localRotationMatrix;
	glm::mat4 Ro = planet->orbitalRotationMatrix;
	glm::mat4 IRo = glm::transpose(planet->orbitalRotationMatrix);
	glm::mat4 A = planet->axialTiltMatrix;
	glm::mat4 T = planet->translationMatrix;
	glm::mat4 S = planet->scaleMatrix;
	
	glm::mat4 modelMatrix = P * Ro * T * S * A * IRo * Rl;
	
    // clear screen to a dark grey colour
    
    // bind our shader program and the vertex array object containing our
    // scene geometry, then tell OpenGL to draw our geometry
    glUseProgram(shader->program);
    GLint projectionMatrixLocation = glGetUniformLocation(shader->program, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	
	GLint viewMatrixLocation = glGetUniformLocation(shader->program, "viewMatrix");
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	
	GLint modelMatrixLocation = glGetUniformLocation(shader->program, "modelMatrix");
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	
	GLint texLocation = glGetUniformLocation(shader->program, "texture");
	glUniform1i(texLocation, 0);
	
	GLint starLocation = glGetUniformLocation(shader->program, "isStar");
	glUniform1i(starLocation, isStar);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, planet->texture->textureName);

    glBindVertexArray(sphere.vertexArray);

    glDrawArrays(GL_TRIANGLES, 0, sphere.elementCount);

    // reset state to default (no shader or geometry bound)
    glBindVertexArray(0);
    glUseProgram(0);

    // check for an report any OpenGL errors
    CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
        glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
		timeScale -= 100000;
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	{
		timeScale += 100000;
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		isPaused = !isPaused;
	}	
}

void CursorCallback(GLFWwindow* window, double xpos, double ypos)
{
	
	if(isRotating == true)
	{
		float xOffsetR = xpos - oldXPos;
		float yOffsetR = oldYPos - ypos;

		oldXPos = xpos;
		oldYPos = ypos;
		camera.ChangeAngles(xOffsetR * MOUSE_SENSITIVITY, yOffsetR * MOUSE_SENSITIVITY);
	}

}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		glfwGetCursorPos(window, &oldXPos, &oldYPos);
        isRotating = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		isRotating = false;
	}
	
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ChangeRadius(-yoffset);
}




// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
    // initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initilize, TERMINATING" << endl;
        return -1;
    }
    glfwSetErrorCallback(ErrorCallback);

    // attempt to create a window with an OpenGL 4.1 core profile context
    GLFWwindow *window = 0;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Chris's Awesome Orrery", 0, 0);
    
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, CursorCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetScrollCallback(window, ScrollCallback);
    glfwMakeContextCurrent(window);

    // query and print out information about our OpenGL environment
    QueryGLVersion();
    InitGL();
#ifdef _WIN32
	// Intialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		cout << "glew init failed" << endl;
		return -1;
	}
	//RendererUtility::
	CheckGLErrors();
#endif
    InitializeSphere(&sphere, 40, 80);

    // call function to load and compile shader programs
    
    if (!InitializeShaders(&shader, "vertex.glsl", "fragment.glsl"))
	{
        cout << "Program could not initialize shaders, TERMINATING" << endl;
        return -1;
    }

    // load and initialize the texture
    if(!InitializeTexture(&earthTexture, "texture_earth_surface.jpg") ||
		!InitializeTexture(&sunTexture, "texture_sun.jpg") ||
		!InitializeTexture(&moonTexture, "texture_moon.jpg")||
		!InitializeTexture(&starTexture, "strx.png"))
		
	{
        cout << "Failed to load textures!" << endl;
		return -1;
	}
	
	Planet stars(10000.0f, 0.0f, 0.0f, 0.0f, 0.0f, &starTexture);
	Planet sun(ChangeRadiusScale(695500.0f), 0.0f, 600.0f, 0.0f, 7.25f, &sunTexture);
	Planet earth(ChangeRadiusScale(6371.0f), ChangeDistanceScale(149600000.0f, sizeScale, 0), 24.0f, 8760.0f, 23.4f, &earthTexture);
	Planet moon(ChangeRadiusScale(1737.0f), ChangeDistanceScale(385000.0f, sizeScale, 4 * earth.radius), 648.0f, 648.0f, 6.687f, &moonTexture);
	
	glfwSetTime(0);
	double lastTime = glfwGetTime();

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
		double currTime = glfwGetTime();
		deltaTime = currTime - lastTime;
		lastTime = currTime;
		
		float updateDelta = deltaTime * timeScale;
		
		if (!isPaused)
		{
			sun.Update(updateDelta);
			earth.Update(updateDelta);
			moon.Update(updateDelta);
		}
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		RenderScene(&stars, &shader, glm::mat4(), true);
		RenderScene(&sun, &shader, glm::mat4(), true);
		
        // call function to draw our scene
        RenderScene(&earth, &shader, glm::mat4(), false);
        
        RenderScene(&moon, &shader, earth.globalTransform, false);

        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    // clean up allocated resources before exit
    DestroyGeometry(&sphere);
   
	
    glfwDestroyWindow(window);
    glfwTerminate();

    cout << "Goodbye!" << endl;
    return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
    // query opengl version and renderer information
    string version  = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    string glslver  = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

    cout << "OpenGL [ " << version << " ] "
         << "with GLSL [ " << glslver << " ] "
         << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
        case GL_INVALID_ENUM:
            cout << "GL_INVALID_ENUM" << endl; break;
        case GL_INVALID_VALUE:
            cout << "GL_INVALID_VALUE" << endl; break;
        case GL_INVALID_OPERATION:
            cout << "GL_INVALID_OPERATION" << endl; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
        case GL_OUT_OF_MEMORY:
            cout << "GL_OUT_OF_MEMORY" << endl; break;
        default:
            cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
    string source;

    ifstream input(filename.c_str());
    if (input) {
        copy(istreambuf_iterator<char>(input),
             istreambuf_iterator<char>(),
             back_inserter(source));
        input.close();
    }
    else {
        cout << "ERROR: Could not load shader source from file "
             << filename << endl;
    }

    return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
    // allocate shader object name
    GLuint shaderObject = glCreateShader(shaderType);

    // try compiling the source as a shader of the given type
    const GLchar *source_ptr = source.c_str();
    glShaderSource(shaderObject, 1, &source_ptr, 0);
    glCompileShader(shaderObject);

    // retrieve compile status
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
        cout << "ERROR compiling shader:" << endl << endl;
        cout << source << endl;
        cout << info << endl;
    }

    return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();

    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);

    // try linking the program with given attachments
    glLinkProgram(programObject);

    // retrieve link status
    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
        cout << "ERROR linking shader program:" << endl;
        cout << info << endl;
    }

    return programObject;
}
