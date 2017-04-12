#version 150

in  vec3 in_Position;
in  vec3 in_Normal;
in  vec3 in_Color;
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
	gl_Position = projection * lookAtMatrix * mdlMatrix  * vec4(in_Position, 1.0);
	normal = in_Normal;
	texCoord = inTexCoord;


	const vec3 light = vec3(0.58, 0.58, 0.58);
	float shade;

	shade = dot(normalize(in_Normal), light);
	color = vec3(shade);

}
