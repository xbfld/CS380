#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec2 vertexUV;
layout(location = 4) in vec3 tangents;

// Output data ; will be interpolated for each fragment.
out vec3 fragmentPosition;
out vec3 fragmentNormal;
out vec2 UV;

smooth out vec3 ulightdir;
smooth out vec3 utovdir;
smooth out vec3 uhalf;

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

	//transpose of inversed model view matrix
	mat4 invm = inverse(MVM);
	invm[0][3] = 0; invm[1][3] = 0;	invm[2][3] = 0;
	mat4 NVM = transpose(invm);
	vec4 tnormal = vec4(vertexNormal_modelspace, 0.0);
	fragmentNormal = vec3(NVM * tnormal);
	UV = vertexUV;

	//TODO: calculate light properties in tangent space
	vec3 n = vec3(NVM * tnormal);
	vec3 t = normalize((NVM * vec4(tangents,0))).xyz;
	vec3 b = cross(n, t);

	vec3 vert_pos = wPosition.xyz;
	vec3 lightDir = normalize(uLight - vert_pos);

	vec3 v;
	v.x = dot(lightDir, t); v.y = dot(lightDir, b); v.z = dot(lightDir, n);
	ulightdir = normalize(v);

	v.x = dot(vert_pos, t); v.y = dot(vert_pos, b); v.z = dot(vert_pos, n);
	utovdir = normalize(v);

	vec3 halfVector = normalize(vert_pos + lightDir);
	v.x = dot(halfVector, t); v.y = dot(halfVector, b);v.z = dot(halfVector, n);
	uhalf = v;
}