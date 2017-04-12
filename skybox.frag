#version 150

out vec4 out_Color;
in vec3 color;
in vec3 normal;
in vec2 texCoord;

 uniform sampler2D skyboxTex;

void main(void)
{
  out_Color = texture(skyboxTex, texCoord);
 }
