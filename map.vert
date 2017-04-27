#version 150

in  vec3 inPosition;
in  vec3 inNormal;
in vec2 inTexCoord;
out vec2 texCoord;
out vec3 fragVert;
out vec3 normal;
out vec3 surfacePosition;

// NY
uniform mat4 projMatrix;
uniform mat4 mdlMatrix;


void main(void)
{
	gl_PointSize = 100.0;
	gl_Position = projMatrix * mdlMatrix * vec4(inPosition, 1.0);
	normal = inNormal;
	texCoord = inTexCoord;



}
