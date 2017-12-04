#version 440

layout(location = 0) in vec3 squareVertices;
layout(location = 1) in vec4 xyzs; 
layout(location = 2) in vec4 color; 

uniform mat4 modelTransformMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec2 UV;
out vec4 particlecolor;

void main(){


	vec3 camRight_worldspace = vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
	vec3 camUp_worldspace = vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);

	float particleSize = xyzs.w;
    vec3 position = xyzs.xyz + camRight_worldspace * squareVertices.x * particleSize + camUp_worldspace * squareVertices.y * particleSize;
	vec4 v = vec4(position, 1.0);
	vec4 newPosition = modelTransformMatrix * v;
	vec4 projectedPosition = projectionMatrix * viewMatrix * newPosition;
	gl_Position = projectedPosition;

	UV = squareVertices.xy + vec2(0.5, 0.5);
	particlecolor = color;
}