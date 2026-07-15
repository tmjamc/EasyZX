#version 410

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TEX0;

void main()
{
	gl_Position = vec4(aPos, 0.0, 1.0);
	TEX0 = vec2(aTexCoord.x, aTexCoord.y);
}