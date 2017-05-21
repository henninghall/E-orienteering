#version 150

out vec4 out_Color;
in vec3 color;

uniform mat4 mdlMatrix;
in vec3 normal;
in vec2 texCoord;
in vec3 Normal_cameraspace;
in vec3 LightDirection_cameraspace;
uniform bool showShadow;
uniform bool hasOpenLand;

in vec3 fragVert;

in vec3 pos;
in vec3 surfacePosition;

uniform vec3 cameraPos;
uniform sampler2D texUnit;

void main(void)
{
const vec3 light = vec3(0.58, 0.58, 0.58);
float shade;
shade = dot(normalize(normal), light);

// To prevent too dark
if(shade < 0.4) shade = 0.39 + shade * 0.3;

if(showShadow == false) shade = 1.0;
out_Color = texture(texUnit, texCoord) * vec4(shade, shade, shade, 1.0);

}
