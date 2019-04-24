#version 120
#ifdef GL_ES
precision mediump float;
#endif

uniform float u_Time;

vec3 permute(vec3 x) { return mod( x*x*34.+x, 289.); }

float snoise(vec2 v) {
    vec2 i = floor(v + (v.x + v.y) * .36602540378443),
        x0 = v -   i + (i.x + i.y) * .211324865405187,
        j = step(x0.yx, x0),
        x1 = x0 - j + .211324865405187,
        x3 = x0 - .577350269189626;
    i = mod(i, 289.);
    vec3 p = permute(permute(i.y + vec3(0., j.y, 1.))
                             + i.x + vec3(0., j.x, 1.)),
        m = max(.5 - vec3(dot(x0, x0), dot(x1, x1), dot(x3, x3)), 0.),
        x = 2. * fract(p * .024390243902439) - 1.,
        h = abs(x) - .5,
        a0 = x - floor(x + .5),
        g = a0 * vec3(x0.x, x1.x, x3.x)
            + h * vec3(x0.y, x1.y, x3.y);
    m = m*m*m*m* (1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h));  
    return .5 + 65. * dot(m, g);
}

void main()
{
    float alpha = snoise(gl_FragCoord.xy / 10. + u_Time)  * gl_Color.a;
    gl_FragColor = vec4(gl_Color.rgb, alpha);
}