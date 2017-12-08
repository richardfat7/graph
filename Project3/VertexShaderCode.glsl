#version 440  // GLSL version your computer supports

in layout(location=0) vec3 position;
in layout(location=1) vec2 vertexUV;
in layout(location=2) vec3 normal;


uniform mat4 modelTransformMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform bool fog_flag;

out vec2 UV;
out vec3 normalWorld;
out vec3 vertexPositionWorld;
out float visibility;

void main()
{
	vec4 v = vec4(position, 1.0);
	vec4 new_position = modelTransformMatrix * v;
	vec4 projected_position = projectionMatrix * viewMatrix *  new_position;
	gl_Position = projected_position;

	vec4 normal_temp = modelTransformMatrix * vec4(normal,0);
	normalWorld = normal_temp.xyz;
	vertexPositionWorld = new_position.xyz;
	UV = vertexUV;

	visibility = 1;
	if(fog_flag == true){
		float fogDen = 0.02f;
		float fogGrad = 2.0f;
		float distance = length(viewMatrix * new_position);
		visibility = exp ( -pow ((distance * fogDen), fogGrad ));
		visibility = clamp (visibility , 0 ,1);
	}

}