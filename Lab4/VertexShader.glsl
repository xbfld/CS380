#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec3 fragmentPosition;
out vec3 fragmentNormal;
out vec2 UV;


uniform mat4 ModelTransform;
uniform mat4 Eye;
uniform mat4 Projection;

void main(){
	// Output position of the vertex, in clip space : MVP * position
	mat4 MVM = inverse(Eye) * ModelTransform;
		
	vec4 wPosition = MVM * vec4(vertexPosition_modelspace, 1);
	fragmentPosition = wPosition.xyz;
	gl_Position = Projection * wPosition;
	
	//transpose of inversed model view matrix
	mat4 invm = inverse(MVM);
	invm[0][3] = 0; invm[1][3] = 0;	invm[2][3] = 0;
	mat4 NVM = transpose(invm);
	vec4 tnormal = vec4(vertexNormal_modelspace, 0.0);
	fragmentNormal = vec3(NVM * tnormal);	
	UV = vertexUV;
}

