
#version 150

in  vec3 inPosition;
in  vec3 inNormal;
out  vec3 color;
out  vec3 normal;
in vec2 inTexCoord;
out vec2 texCoord;

uniform mat4 myMatrix;
uniform mat4 rotation;
uniform mat4 projection;
uniform mat4 lookAtMatrix;
uniform mat4 mdlMatrix;




void main(void)
{
	gl_Position = projection * lookAtMatrix * mdlMatrix  * vec4(inPosition, 1.0);
	normal = inNormal;
	texCoord = inTexCoord;


	const vec3 light = vec3(0.58, 0.58, 0.58);
	float shade;

	//shade = dot(normalize(inNormal), light);
	color = vec3(shade);

}
