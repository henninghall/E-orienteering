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

/*
out_Color = vec4(0.4,0.4,0.4,1.0);

*/
const vec3 light = vec3(0.98, 0.98, 0.98);
float shade;
shade = dot(normalize(normal), light);

if(shade < 0.9) shade = 0.9;


out_Color = texture(texUnit, texCoord) *vec4(shade, shade, shade, 1.0);

}
