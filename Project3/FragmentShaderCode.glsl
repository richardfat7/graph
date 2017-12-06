#version 440

in vec2 UV;
in vec3 normalWorld;
in vec3 vertexPositionWorld;
in float visibility;

uniform vec4 ambientLight1;
uniform vec3 lightPositionWorld1;
uniform vec3 lightPositionWorld2;
uniform vec3 lightPositionWorldr;
uniform vec3 lightPositionWorldy;
uniform vec3 lightPositionWorldg;
uniform vec3 eyePositionWorld;
uniform sampler2D myTextureSampler;
uniform sampler2D myTextureSampler2;
uniform float difdelta1;
uniform float difdeltar;
uniform float difdeltay;
uniform float difdeltag;
uniform float spedelta;

uniform bool normalMapping_flag;
uniform bool multiMapping_flag;
uniform bool sun;

uniform vec3 fog_Color;

out vec4 fogfinalColor;

void main()
{
	vec3 normal = normalize(normalWorld);
	if (normalMapping_flag){
		normal = texture(myTextureSampler2, UV).rgb;
		normal = normalize(normal * 2.0 - 1.0);
	}
	
	vec4 AmbLightCol1 = ambientLight1;

	vec3 lightVectorWorld1 = normalize(lightPositionWorld1 - vertexPositionWorld);
	float brightness = dot(lightVectorWorld1, normal);
	vec4 DifBrightness1 = vec4(brightness, brightness, brightness, 1.0f);
	float Diflightrgb1 = clamp(0.0f + difdelta1, 0 , 1); 
	vec4 DifLightCol1 = vec4(Diflightrgb1, Diflightrgb1 , Diflightrgb1, 1.0f);

	vec3 lightVectorWorld2 = normalize(lightPositionWorld2 - vertexPositionWorld);
	brightness = dot(lightVectorWorld2, normal);
	if(sun == true)brightness = brightness * -0.7;
	vec4 DifBrightness2 = vec4(brightness, brightness, brightness, 1.0f);
	vec4 DifLightCol2 = vec4(3.0f, 3.0f, 3.0f, 1.0f);

	vec3 lightVectorWorldr = normalize(lightPositionWorldr - vertexPositionWorld);
	brightness = dot(lightVectorWorldr, normal);
	vec4 DifBrightnessr = vec4(brightness, brightness, brightness, 1.0f);
	float Diflightrgbr = clamp(0.0f + difdeltar, 0 , 0.5); 
	vec4 DifLightColr = vec4(Diflightrgbr, 0.0f,0.0f, 1.0f);

	vec3 lightVectorWorldy = normalize(lightPositionWorldy - vertexPositionWorld);
	brightness = dot(lightVectorWorldy, normal);
	vec4 DifBrightnessy = vec4(brightness, brightness, brightness, 1.0f);
	float Diflightrgby = clamp(0.0f + difdeltay, 0 , 0.5); 
	vec4 DifLightColy = vec4(Diflightrgby, Diflightrgby ,0.0f, 1.0f);

	vec3 lightVectorWorldg = normalize(lightPositionWorldg - vertexPositionWorld);
	brightness = dot(lightVectorWorldg, normal);
	vec4 DifBrightnessg = vec4(brightness, brightness, brightness, 1.0f);
	float Diflightrgbg = clamp(0.0f + difdeltag, 0 , 0.5); 
	vec4 DifLightColg = vec4(0.0f, Diflightrgbg ,0.0f, 1.0f);

	vec3 reflectedLightVectorWorld = reflect (-lightVectorWorld2, normal);
	vec3 eyeVectorWorld = normalize (eyePositionWorld - vertexPositionWorld);
	float SpeBrightness = clamp(dot(reflectedLightVectorWorld, eyeVectorWorld), 0 ,1);
	vec4 SpeLightCol = vec4(0.1f + spedelta ,0.1f + spedelta , 0.1f + spedelta , 1.0f);

	vec4 MatAmbCol;
	vec4 MatDifCol;
	if(multiMapping_flag){
		MatAmbCol = vec4((0.8 * texture(myTextureSampler,UV) + 0.6 * texture(myTextureSampler2,UV)).rgb, 1.0f);
		MatDifCol = vec4((0.8 * texture(myTextureSampler,UV) + 0.6 * texture(myTextureSampler2,UV)).rgb, 1.0f);
	}
	else{
		MatAmbCol = vec4(texture(myTextureSampler,UV).rgb, 1.0f);
		MatDifCol = vec4(texture(myTextureSampler,UV).rgb, 1.0f);
		}

	vec4 MatSpeCol = vec4(0.3f, 0.3f, 0.3f, 1.0f);

	if(sun==true)AmbLightCol1;
	vec4 finalColor = MatAmbCol * AmbLightCol1+
				 MatDifCol * DifLightCol1 * DifBrightness1 +
				 MatDifCol * DifLightCol2 * DifBrightness2 +
				 MatDifCol * DifLightColr * DifBrightnessr +
				 MatDifCol * DifLightColy * DifBrightnessy +
				 MatDifCol * DifLightColg * DifBrightnessg +
				 MatSpeCol * SpeLightCol * pow(SpeBrightness,50);

	fogfinalColor = mix (vec4(fog_Color, 1.0f), finalColor, visibility);
}
