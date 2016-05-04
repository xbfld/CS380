#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
//TODO: grab color value from the application
layout(location = 2) in vec3 vertexColor;
//TODO: grab normal value from the application



// Output data ; will be interpolated for each fragment.
out vec3 fragmentPosition;
out vec3 fragmentColor;
out vec3 fragmentNormal;

uniform mat4 ModelTransform;
uniform mat4 Eye;
uniform mat4 Projection;
uniform vec3 uLight;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	mat4 MVM = inverse(Eye) * ModelTransform;
		
	vec4 wPosition = MVM * vec4(vertexPosition_modelspace, 1);
	fragmentPosition = wPosition.xyz;
	gl_Position = Projection * wPosition;
	
	//TODO: pass the interpolated color value to fragment shader 
	fragmentColor = vertexColor;

	//TODO: Calculate/Pass normal of the the vertex
	//transpose of inversed model view matrix

}

