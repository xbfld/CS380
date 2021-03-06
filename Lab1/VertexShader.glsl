#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
// layout(location = 1) in vec3 vertexColor;
//layout(location = 2) in vec3 offset;
//    (...) : Layout Qualifier

// Output data ; will be interpolated for each fragment.
out vec4 fragmentColor;
// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform vec4 vcolor = vec4(1.0f, 1.0f, 1.0f, 1.0f);

void main(){	
	// Output position of the vertex, in clip space : MVP * position
	gl_Position = MVP * vec4(vertexPosition_modelspace, 1);
	fragmentColor = vcolor;
	//fragmentColor = (vcolor+vertexColor)/2.0f;
}

