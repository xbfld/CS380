#version 330 core

in vec4 fragmentColor;

// Ouput color
out vec4 color;

void main(){
	// Output color = color specified in the vertex shader
	color = fragmentColor;
}