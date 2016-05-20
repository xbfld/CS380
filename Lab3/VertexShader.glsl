#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
//TODO: grab color value from the application
layout(location = 2) in vec3 vertexColor;
//TODO: grab normal value from the application
layout(location = 1) in vec3 vertexNormal_modelspace;


// Output data ; will be interpolated for each fragment.
out vec3 fragmentPosition;
out vec3 fragmentColor;
out vec3 fragmentNormal;

uniform mat4 ModelTransform;
uniform mat4 OffsetFrame;
uniform mat4 Eye;
uniform mat4 Projection;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	mat4 MVM = inverse(Eye) * ModelTransform * OffsetFrame ;
		
	vec4 wPosition = MVM * vec4(vertexPosition_modelspace, 1);
	fragmentPosition = wPosition.xyz;
	gl_Position = Projection * wPosition;

	//TODO: pass the interpolated color value to fragment shader 
	fragmentColor = vertexColor;

	//TODO: Calculate/Pass normal of the the vertex
	//transpose of inversed model view matrix
	mat4 invm = inverse(MVM);
	invm[0][3] = 0; invm[1][3] = 0; invm[2][3] = 0;
	mat4 NVM = transpose(invm);
	vec4 tnormal = vec4(vertexNormal_modelspace, 0.0);
	fragmentNormal = vec3(NVM * tnormal);

	
    // Gouraud shading
    // vec3 tolight = normalize(uLight - fragmentPosition);
    // vec3 toV = -normalize(vec3(fragmentPosition));
    // vec3 h = normalize(toV + tolight);
    // vec3 normal = normalize(fragmentNormal);
    // float specular = pow(max(0.0, dot(h, normal)), 64.0);
    // float diffuse = max(0.0, dot(normal, tolight));
    // vec3 intensity = fragmentColor *diffuse + vec3(0.6, 0.6, 0.6)*specular;
    // fragmentColor = pow(intensity, vec3(1.0 / 2.2)); // Apply gamma correction 
}

