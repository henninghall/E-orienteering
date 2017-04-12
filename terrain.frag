#version 150

out vec4 out_Color;
in vec3 color;

uniform mat4 mdlMatrix;

in vec3 normal;
in vec2 texCoord;
in vec3 Normal_cameraspace;
in vec3 LightDirection_cameraspace;

uniform vec3 lightSourcesDirPosArr[4];
uniform vec3 lightSourcesColorArr[4];
uniform float specularExponent[4];
uniform bool isDirectional[4];


in vec3 fragVert;
in vec3 surfacePosition;

uniform vec3 cameraPos;
uniform sampler2D texUnit;

void main(void)
{


mat3 normalMatrix;
vec3 n_normal;
vec3 fragPosition;
vec3 surfaceToLight;
float brightness;
vec4 surfaceColor;
vec3 sumColor;
vec3 diffuseComponent;


normalMatrix = mat3(mdlMatrix);//transpose(inverse(mat3(mdlMatrix)));
n_normal = normalize(normalMatrix * normal);

fragPosition = vec3(mdlMatrix * vec4(fragVert, 1));


for(int i = 0; i < 4; i++){
	
	
	// diffuse
	/*surfaceToLight = normalize(lightSourcesDirPosArr[i] - surfacePosition);//fragPosition);
	brightness = dot(n_normal, surfaceToLight);// / (length(surfaceToLight) / length(n_normal));
	brightness = clamp(brightness, 0, 1);

	vec4 surfaceColor = texture(texUnit, texCoord);
	diffuseComponent =  brightness * lightSourcesColorArr[i] * surfaceColor.rgb;
	*/
	


	// directional 
	if(isDirectional[i]){
		surfaceToLight = normalize(lightSourcesDirPosArr[i]);  
	}
	else {
		surfaceToLight = normalize(lightSourcesDirPosArr[i] - surfacePosition); 
	}
	
	float shade; 
	shade = dot(n_normal, surfaceToLight); 
	shade = clamp(shade, 0, 1);
	vec4 surfaceColor = texture(texUnit, texCoord);
	diffuseComponent = vec3(shade, shade, shade) * lightSourcesColorArr[i] * surfaceColor.rgb ;
	

	// specular
	vec3 incidenceVector = surfaceToLight;//normalize(surfacePosition - lightSourcesDirPosArr[i]);
	vec3 reflectionVector = reflect(incidenceVector, n_normal); 
	vec3 surfaceToCamera = normalize(cameraPos - surfacePosition); 
	float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
	float specularCoefficient = pow(cosAngle,  specularExponent[i]);
	vec3 specularComponent = specularCoefficient * surfaceColor.rgb * lightSourcesColorArr[i];


	sumColor = sumColor + diffuseComponent + specularComponent;
}


out_Color = vec4(sumColor,1);


}
