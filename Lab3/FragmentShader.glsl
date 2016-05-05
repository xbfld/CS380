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
	// color = fragmentColor;
	//TODO:Assign fragmentNormal as a final fragment color
	// vec3 normal = normalize(fragmentNormal);
    // color = normal;
    // TODO: Phong reflection model
    vec3 tolight = normalize(uLight - fragmentPosition);
    vec3 toV = -normalize(vec3(fragmentPosition));
    vec3 h = normalize(toV + tolight);
    vec3 normal = normalize(fragmentNormal);
    float specular = pow(max(0.0, dot(h, normal)), 64.0);
    float diffuse = max(0.0, dot(normal, tolight));
    vec3 intensity = fragmentColor *diffuse + vec3(0.6, 0.6, 0.6)*specular;
    color = pow(intensity, vec3(1.0 / 2.2)); // Apply gamma correction 
    
    // Phong chage
    // vec3 tolight = normalize(uLight - fragmentPosition);
    // vec3 toV = -normalize(vec3(fragmentPosition));
    // vec3 h = normalize(toV + tolight);
    // vec3 normal = normalize(fragmentNormal);
    // float specular = pow(max(0.0, dot(h, normal)), 64.0);
    // float diffuse = max(0.0, dot(normal, tolight));
    // vec3 intensity = fragmentColor *diffuse + vec3(0.6, 0.6, 0.6)*specular;
    // color = floor(pow(intensity, vec3(1.0 / 2.2))*3)/3.0f;

    // Toon Shading
    // vec3 normal = normalize(fragmentNormal);
    // color = floor(dot(normal,normalize(vec3(1.0,2.0,2.0)))*3)/3.0*vec3(1.0,0.6,0.7); // Apply gamma correction 

    // color = floor(color*3)/3.0;

    // Gouraud shading
    // color = fragmentColor;

}
