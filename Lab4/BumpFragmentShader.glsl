#version 330 core

in vec3 fragmentPosition;
//in vec3 fragmentColor;
in vec3 fragmentNormal;

in vec2 UV;
//dir light
in vec3 ulightdir;
in vec3 utovdir;
in vec3 uhalf;

// Ouput data
out vec3 color;

//Uniform variables
uniform vec3 uLight;

uniform sampler2D myTextureSampler;
uniform sampler2D myBumpSampler;

void main(){
	//directional light		
	vec3 tolight = normalize(uLight - fragmentPosition);
	vec3 toV = -normalize(vec3(fragmentPosition));
	vec3 h = normalize(toV + tolight);
	vec3 normal = normalize(fragmentNormal);	
	//TODO: change lighting parameter into normal map parameters
	
	//TODO: change normal with loaded normal texture
	
	float specular = pow(max(0.0, dot(h, normal)), 64.0);
	float diffuse = max(0.0, dot(normal, tolight));
	vec3 Kd = vec3(1.0, 1.0, 1.0);
	//TODO: Change material color to texture color
	
	vec3 intensity = Kd*diffuse + vec3(0.3, 0.3, 0.3)*specular;

	vec3 finalColor = intensity;
	color = pow(finalColor, vec3(1.0 / 2.2));// Apply gamma correction    		
}