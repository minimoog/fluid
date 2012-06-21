#ifdef GL_ES
precision highp float;
#endif

varying vec2 texCoordOutput;

uniform vec2 droplet_position;
uniform vec2 resolution;

void main()
{
    vec3 color;

    vec2 scdm = droplet_position / resolution;
    float radius = distance(scdm, texCoordOutput);

    if (radius < 0.05) {
        color = vec3(0.0, 1.0, 0.0);
    } else {
        color = vec3(0.0, 0.0, 0.0);
    }

    gl_FragColor = vec4(color, 1.0);
}
