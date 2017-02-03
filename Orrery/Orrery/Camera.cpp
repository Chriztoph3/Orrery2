#include "Camera.h"
#include "glm/vec3.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <math.h>

Camera::Camera(float fov, float aspect, float near, float far)
{
	this->theta = 0;
	this->phi = 0;
	this->radius = 50;
	this->projectionMatrix = glm::perspective(fov, aspect, near, far);
	Update();
}

void Camera::ChangeAngles(float theta, float phi)
{
	this->theta += theta;
	this->phi += phi;
	
	// Keeps theta in the range [0, 2pi]
	if (this->theta < 0)
	{
		this->theta += 2 * 3.1415926535;
	}
	else if (this->theta >= 360)
	{
		this->theta -= 2 * 3.1415926535;
	}
	
	if (this->phi >= 3.1415926535/2)
	{
		this->phi = 3.1415926535/2 - 0.001;
	}
	else if (this->phi <= -3.1415926535/2)
	{
		this->phi = -3.1415926535/2 + 0.001;
	}
	
	Update();
}

void Camera::ChangeRadius(float radius)
{
	this->radius += radius;
	
	if (this->radius < 1)
	{
		this->radius = 1;
	}
	
	Update();
}

void Camera::Update()
{
	float x = this->radius * sin(this->theta) * cos(this->phi);
	float y = this->radius * sin(this->phi);
	float z = this->radius * cos(this->theta) * cos(this->phi);
	
	this->viewMatrix = glm::lookAt(glm::vec3(x, y, z), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

glm::mat4 Camera::GetViewMatrix()
{
	return this->viewMatrix;
}

glm::mat4 Camera::GetProjectionMatrix()
{
	return this->projectionMatrix;
}
