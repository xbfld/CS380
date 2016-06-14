#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec2 vertexUV;

out vec3 fragmentPosition;
out vec3 fragmentNormal;
out vec2 UV;
out float Reflectivity;

smooth out vec3 ReflectDir;
smooth out vec3 RefractDir;

uniform mat4 ModelTransform;
uniform mat4 Eye;
uniform mat4 Projection;
uniform bool DrawSkyBox;
uniform vec3 WorldCameraPosition;

void main(){
	mat4 MVM = inverse(Eye) * ModelTransform;

	vec4 wPosition = MVM * vec4(vertexPosition_modelspace, 1);
	fragmentPosition = wPosition.xyz;
	
	mat4 invm = inverse(MVM);
	invm[0][3] = 0; invm[1][3] = 0;	invm[2][3] = 0;
	mat4 NVM = transpose(invm);
	vec4 tnormal = vec4(vertexNormal_modelspace, 0.0);
	fragmentNormal = vec3(NVM * tnormal);
	UV = vertexUV;

	gl_Position = Projection * wPosition;	

	//TODO: Calculate Reflection Dir for Environmental map
	mat4 inverseProjection = inverse(Projection);
	mat3 inverseModelview = mat3(inverse(MVM));
	Reflectivity = 1;
	if (DrawSkyBox){
		ReflectDir = vertexPosition_modelspace;
	}
	else{
		float eta = 0.6666; //air~1 glass=1.5, eta = n1/n2
		float oneovereta = 1.5;
		// Reflectivity from Fresnel equations
		Reflectivity = pow(abs((eta-1.0)/(eta+1.0)),2.0);
		ReflectDir = vec3(Eye* vec4(-reflect(-fragmentPosition, normalize(fragmentNormal)),.0));
		RefractDir = vec3(Eye* vec4(refract(fragmentPosition, normalize(fragmentNormal),eta),.0));
	}
	ReflectDir = vec3(ReflectDir.x,-ReflectDir.yz);
	RefractDir = vec3(RefractDir.x,-RefractDir.yz);
}

