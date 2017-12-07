#version 440

in vec2 UV;
in vec3 normalWorld;
in vec3 vertexPositionWorld;
in float visibility;

uniform vec4 ambientLight1;
uniform vec3 lightPositionWorld1;
uniform vec3 lightPositionWorld2;
uniform vec3 eyePositionWorld;
uniform sampler2D myTextureSampler;
uniform float difdelta1;
uniform float spedelta;

uniform vec3 fog_Color;

out vec4 fogfinalColor;

void main()
{
	vec3 normal = normalize(normalWorld);
	
	vec4 AmbLightCol1 = ambientLight1;

	vec3 lightVectorWorld1 = normalize(lightPositionWorld1 - vertexPositionWorld);
	float brightness = dot(lightVectorWorld1, normal);
	vec4 DifBrightness1 = vec4(brightness, brightness, brightness, 1.0f);
	float Diflightrgb1 = clamp(0.5f + difdelta1, 0 , 1); 
	vec4 DifLightCol1 = vec4(Diflightrgb1, Diflightrgb1 , Diflightrgb1, 1.0f);

	vec3 lightVectorWorld2 = normalize(lightPositionWorld2 - vertexPositionWorld);
	brightness = clamp(dot(lightVectorWorld2, normal), 0, 1);
	vec4 DifBrightness2 = vec4(brightness, brightness, brightness, 1.0f);
	vec4 DifLightCol2 = vec4(3.0f, 3.0f, 3.0f, 1.0f);

	vec3 reflectedLightVectorWorld = reflect (-lightVectorWorld2, normal);
	vec3 eyeVectorWorld = normalize (eyePositionWorld - vertexPositionWorld);
	float SpeBrightness = clamp(dot(reflectedLightVectorWorld, eyeVectorWorld), 0 ,1);
	vec4 SpeLightCol = vec4(0.1f + spedelta ,0.1f + spedelta , 0.1f + spedelta , 1.0f);

	vec4 MatAmbCol;
	vec4 MatDifCol;
	MatAmbCol = vec4(texture(myTextureSampler,UV).rgb, 1.0f);
	MatDifCol = vec4(texture(myTextureSampler,UV).rgb, 1.0f);

	vec4 MatSpeCol = vec4(0.3f, 0.3f, 0.3f, 1.0f);

	vec4 finalColor = MatAmbCol * AmbLightCol1+
				 MatDifCol * DifLightCol1 * DifBrightness1 +
				 MatDifCol * DifLightCol2 * DifBrightness2 +
				 MatSpeCol * SpeLightCol * pow(SpeBrightness,50);
	fogfinalColor = mix (vec4(fog_Color, 1.0f), finalColor, visibility);
}
