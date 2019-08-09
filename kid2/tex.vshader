#ifdef GL_ES
precision mediump float;
#endif

attribute vec3 InVertex;
attribute vec2 InTexCoord0;
attribute vec4 InColor;
attribute vec3 InNormal;
uniform mat4 local2proj;

#ifdef GL_ES
varying mediump vec2 v_texCoord;
varying mediump vec4 v_Color;
#else
varying vec2 v_texCoord;
varying vec4 v_Color;
#endif

void main()
{
      gl_Position = local2proj * vec4(InVertex,1.0);
      v_texCoord  = InTexCoord0.st;
      v_Color = InColor;
}

