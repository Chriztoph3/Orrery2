#pragma once

#include "glm/mat4x4.hpp"

class Camera {
private:
	float fov, near, far;
	float theta, phi, radius;
	
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	
	void Update();
	
public:
	Camera(float fov, float aspect, float near, float far);
	
	void ChangeAngles(float theta, float phi);
	void ChangeRadius(float radius);
	
	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix();
};
