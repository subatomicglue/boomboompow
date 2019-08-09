#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D tex;
#ifdef GL_ES
varying mediump vec2 v_texCoord;
varying mediump vec4 v_Color;
#else
varying vec2 v_texCoord;
varying vec4 v_Color;
#endif

void main()
{
   vec4 h = texture2D(tex, v_texCoord );
   gl_FragColor = v_Color * h.rgba;
   /*gl_FragColor = v_Color * h.bgr * vec4(1.0, 1.0, 1.0, 0.2);*/
}


