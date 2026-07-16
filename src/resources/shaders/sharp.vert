#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

#if __VERSION__ >= 130
#define COMPAT_VARYING out
#define COMPAT_ATTRIBUTE in
#define COMPAT_TEXTURE texture
#else
#define COMPAT_VARYING varying 
#define COMPAT_ATTRIBUTE attribute 
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

//COMPAT_ATTRIBUTE vec4 VertexCoord;
//COMPAT_ATTRIBUTE vec4 COLOR;
//COMPAT_ATTRIBUTE vec4 TexCoord;
//COMPAT_VARYING vec4 COL0;
COMPAT_VARYING vec4 TEX0;

uniform mat4 MVPMatrix;
uniform int FrameDirection;
uniform int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;

// vertex compatibility #defines
#define vTexCoord TEX0.xy
#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define outsize vec4(OutputSize, 1.0 / OutputSize)

COMPAT_VARYING vec2 precalc_texel;
COMPAT_VARYING vec2 precalc_scale;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
//    gl_Position = MVPMatrix * VertexCoord;
//    COL0 = COLOR;
//    TEX0.xy = TexCoord.xy;
    TEX0.xy = aTexCoord;

    precalc_texel = vTexCoord * SourceSize.xy;
    precalc_scale = max(floor(outsize.xy / InputSize.xy), vec2(1.0, 1.0));
}