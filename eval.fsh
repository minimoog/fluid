uniform sampler2D texUnit0;
uniform highp float width;
uniform highp float height;

const highp float d = 0.1;
const highp float t = 1.3;
const highp float c = 0.05;
const highp float mu = 0.05;

const highp float f1 =  c * c * t * t / (d * d);
const highp float f2 = 1.0 / (mu * t + 2.0);
const highp float k1 = (4.0 - 8.0 * f1) * f2;
const highp float k2 = (mu * t - 2.0) * f2;
const highp float k3 = 2.0 * f1 * f2;

highp vec2 step_w = vec2(1.0 / float(width), 0.0);
highp vec2 step_h = vec2(0.0, 1.0 / float(height));

varying highp vec2 texCoordOutput;

void main(void)
{
    highp vec4 center = texture2D(texUnit0, texCoordOutput);
    highp vec4 right  = texture2D(texUnit0, texCoordOutput + step_w);
    highp vec4 left   = texture2D(texUnit0, texCoordOutput - step_w);
    highp vec4 up     = texture2D(texUnit0, texCoordOutput - step_h);
    highp vec4 down   = texture2D(texUnit0, texCoordOutput + step_h);

    highp float next = k1 * center.y + k2 * center.x + k3 * (right.y + left.y + down.y + up.y);
    highp float prev = center.y;

    gl_FragColor = vec4(prev, next, center.z, 1.0);
}
