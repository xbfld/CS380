#version 330 core

in vec3 fragmentColor;

// Ouput color
out vec3 color;

void main(){
	// Output color = color specified in the vertex shader
	color = fragmentColor;
}