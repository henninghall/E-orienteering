#version 150

out vec4 out_Color;
in vec3 color;
in vec3 normal;
in vec2 texCoord;

 uniform sampler2D texUnit;

void main(void)
{
  //out_Color = texture(texUnit, texCoord)  * vec4(color,1.0);
  out_Color = vec4(color,1.0);

 }
