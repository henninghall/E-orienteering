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

	mat3 normalMatrix1 = mat3(mdlMatrix);	// how does this work? 
	
	texCoord = inTexCoord;
	gl_Position = projMatrix * mdlMatrix * vec4(inPosition, 1.0);


	//texCoord = inTexCoord;
	normal = inNormal;

	//fragVert = inPosition;

	//gl_Position = projection * lookAtMatrix * mdlMatrix  * vec4(in_Position, 1.0);

	surfacePosition = (mdlMatrix * vec4(inPosition, 1.0)).xyz;

}
