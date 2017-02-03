// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
uniform sampler2DRect texture;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 textureCoordData;
layout(location = 2) in vec3 VertexNormal;

// output to be interpolated between vertices and passed to the fragment stage
out vec2 textureCoord;
out vec3 vertexNormal;
out vec3 lightVector;

void main()
{
	vec4 L = viewMatrix * vec4(0 , 0 , 0 , 1.0);
	vec4 N = viewMatrix * modelMatrix * vec4(VertexNormal, 0.0);
	vec4 P = viewMatrix * modelMatrix * vec4(VertexPosition, 1.0);
    gl_Position =  projectionMatrix * P;

    // assign output colour to be interpolated
    textureCoord = textureCoordData;
    vertexNormal = vec3(N);
    lightVector = vec3(L - P);
}
