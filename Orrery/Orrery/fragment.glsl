// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
//	Edited by Christopher Barber 10110661 February 2016
// ==========================================================================
#version 410

uniform sampler2D texture;

uniform bool isStar;

// interpolated colour received from vertex stage
in vec2 textureCoord;
in vec3 vertexNormal;
in vec3 lightVector;

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

void main(void)
{
	vec3 N = normalize(vertexNormal);
	vec3 L = normalize(lightVector);
	
	vec3 texColour = vec3(texture2D(texture, textureCoord));
	vec3 C = texColour;
	
	if (!isStar)
		C = C * max(0, dot(L, N));
	
    FragmentColour = vec4(C, 1);
}
