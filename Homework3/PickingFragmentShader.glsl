#version 330 core

// Ouput data
out vec3 color;

uniform vec3 objectID;

void main(){
	color = objectID;
}