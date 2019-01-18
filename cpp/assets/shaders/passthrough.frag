precision mediump float;

uniform sampler2D u_texture_1;
uniform vec2 u_resolution;

void main()
{
    vec4 pixel = texture2D(u_texture_1, gl_FragCoord.xy / u_resolution);
    gl_FragColor = pixel;
}