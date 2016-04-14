#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;

// Output data ; will be interpolated for each fragment.
uniform mat4 ModelTransform;
uniform mat4 Eye;
uniform mat4 Projection;
uniform vec3 objectID;

void main(){	
	// Output position of the vertex, in clip space : MVP * position
	mat4 MVM = inverse(Eye) * ModelTransform;
	vec4 wPosition = MVM * vec4(vertexPosition_modelspace,1);
	gl_Position = Projection * wPosition;
}

