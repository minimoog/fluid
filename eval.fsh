#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D texUnit0;
uniform float width;
uniform float height;

const float d = 0.1;
const float t = 1.3;
const float c = 0.05;
const float mu = 0.05;

const float f1 =  c * c * t * t / (d * d);
const float f2 = 1.0 / (mu * t + 2.0);
const float k1 = (4.0 - 8.0 * f1) * f2;
const float k2 = (mu * t - 2.0) * f2;
const float k3 = 2.0 * f1 * f2;

vec2 step_w = vec2(1.0 / float(width), 0.0);
vec2 step_h = vec2(0.0, 1.0 / float(height));

varying vec2 texCoordOutput;

void main(void)
{
    vec4 center = texture2D(texUnit0, texCoordOutput);
    vec4 right  = texture2D(texUnit0, texCoordOutput + step_w);
    vec4 left   = texture2D(texUnit0, texCoordOutput - step_w);
    vec4 up     = texture2D(texUnit0, texCoordOutput - step_h);
    vec4 down   = texture2D(texUnit0, texCoordOutput + step_h);

    float next = k1 * center.y + k2 * center.x + k3 * (right.y + left.y + down.y + up.y);
    float prev = center.y;

    gl_FragColor = vec4(prev, next, center.z, 1.0);
}
