#version 440

in layout(location=0) vec3 position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 M;
uniform bool fog_flag;

out vec3 TexCoords;
out float visibility;

void main(){
	vec4 pos = projection * view * M * vec4(position, 1.0f);
	gl_Position = pos;

	TexCoords = position;

	visibility = 1;
	if(fog_flag == true){
		float fogDen = 0.05f;
		float fogGrad = 2.0f;
		float distance = length(view * M * vec4(TexCoords, 1.0f));
		visibility = exp ( -pow ((distance * fogDen), fogGrad ));
		visibility = clamp (visibility , 0 ,1);
	}

}