#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentNormal;
in vec2 UV;

// Ouput data
out vec3 color;

uniform vec3 uLight;
uniform bool DrawSkyBox;

void main(){
	vec3 normal = normalize(fragmentNormal);
	
	if(DrawSkyBox){
		color = vec3(1.0,1.0,1.0);
	}else{
		color = vec3(0.0,0.7,0.7);
	}
	
}