#pragma once
#include "glm/gtc/matrix_transform.hpp"

#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include "GLFW/glfw3.h"

struct MyTexture
{
    // OpenGL names for array buffer objects, vertex array object
    GLuint  textureName;

    // dimensions of the image stored in this texture
    GLuint  width, height;

    // initialize object names to zero (OpenGL reserved value)
    MyTexture() : textureName(0), width(0), height(0)
    {}
};

struct MyShader
{
    // OpenGL names for vertex and fragment shaders, shader program
    GLuint  vertex;
    GLuint  fragment;
    GLuint  program;

    // initialize shader and program names to zero (OpenGL reserved value)
    MyShader() : vertex(0), fragment(0), program(0)
    {}
};

struct MyGeometry
{
    // OpenGL names for array buffer objects, vertex array object
    GLuint  vertexBuffer;
    GLuint	normalBuffer;
    GLuint  textureCoordBuffer;
    GLuint  vertexArray;
    GLsizei elementCount;

    // initialize object names to zero (OpenGL reserved value)
    MyGeometry() : vertexBuffer(0), normalBuffer(0), textureCoordBuffer(0), vertexArray(0), elementCount(0)
    {}
};

struct Planet {
	float radius;
	
	MyTexture *texture;
	
	glm::mat4 globalTransform;
	
	glm::mat4 scaleMatrix;
	glm::mat4 translationMatrix;
	glm::mat4 axialTiltMatrix;
	glm::mat4 localRotationMatrix;
	glm::mat4 orbitalRotationMatrix;
	
	float localAccRotDeg;
	float orbitalAccRotDeg;
	float localRotPerSec;
	float orbitalRotPerSec;
	
	Planet(float radius, float distance, float localPeriod, float orbitalPeriod, float axialTilt, MyTexture *texture)
	{
		this->radius = radius;
		
		if (localPeriod > 0)
			this->localRotPerSec = (float)(2 * 3.1415926535) /(localPeriod*3600.0f);
		else
			this->localRotPerSec = 0;
		
		if (orbitalPeriod)
			this->orbitalRotPerSec = (float)(2 * 3.1415926535) /(orbitalPeriod*3600.0f);
		else
			this->orbitalRotPerSec = 0;
			
		this->localAccRotDeg = 0.0f;
		this->orbitalAccRotDeg = 0.0f;
		this->texture = texture;

		this->scaleMatrix = glm::scale(glm::mat4(), glm::vec3(radius, radius, radius));
		this->translationMatrix = glm::translate(glm::mat4(), glm::vec3(distance,0,0));
		this->axialTiltMatrix = glm::rotate(glm::mat4(), (float)(axialTilt * 3.1415926535) / 180.0f, glm::vec3(0,0,1));
	}
	
	void Update(float deltaTime)
	{
		this->localAccRotDeg += this->localRotPerSec * deltaTime;
		this->orbitalAccRotDeg += this->orbitalRotPerSec * deltaTime;
		
		this->localRotationMatrix = glm::rotate(glm::mat4(), this->localAccRotDeg, glm::vec3(0,1,0));
		this->orbitalRotationMatrix = glm::rotate(glm::mat4(), this->orbitalAccRotDeg, glm::vec3(0,1,0));
		
		this->globalTransform = this->orbitalRotationMatrix * this->translationMatrix;
	}
};
