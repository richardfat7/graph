#version 440

in layout(location=0) vec3 position;
out vec3 TexCoords;
out float visibility;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 M;
uniform bool fog;

void main(){
	vec4 pos = projection * view * M * vec4(position, 1.0f);
	gl_Position = pos;

	TexCoords = position;

	if (fog==true){
		float distance = length(view * M * vec4(position, 1.0f));
		visibility = clamp(exp(-pow((distance * 0.05f), 2.0f)), 0, 0.1f);
	}
	else visibility = 1;
}