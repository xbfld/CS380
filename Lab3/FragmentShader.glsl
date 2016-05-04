#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentColor;
in vec3 fragmentNormal;

// Ouput data
out vec3 color;

uniform vec3 uLight;

void main(){
	
	// color = vec3(1.0, 1.0, 1.0);
	//TODO: Assign fragmentColor as a final fragment color
	color = fragmentColor;
	//TODO:Assign fragmentNormal as a final fragment color
	
	//TODO: Phong reflection model	
}
