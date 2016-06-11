#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentNormal;
in vec2 UV;

// Ouput data
out vec3 color;

uniform vec3 uLight;

void main(){
	//directional light
	vec3 tolight = normalize(uLight - fragmentPosition);	
	vec3 toV = -normalize(vec3(fragmentPosition));
	vec3 h = normalize(toV + tolight);
	vec3 normal = normalize(fragmentNormal);

	float specular = pow(max(0.0, dot(h, normal)), 64.0);
	float diffuse = max(0.0, dot(normal, tolight));

	vec3 intensity = vec3(1.0,1.0,0.0) * diffuse + vec3(0.3, 0.3, 0.3)*specular;

	
	vec3 finalColor = intensity;
	color = pow(finalColor, vec3(1.0 / 2.2));// Apply gamma correction    		
}