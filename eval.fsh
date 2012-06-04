uniform sampler2D texUnit0;
uniform float width;
uniform float height;
uniform float k1;
uniform float k2;
uniform float k3;

const vec2 step_w = vec2(1.0 / float(width), 0.0);
const vec2 step_h = vec2(0.0, 1.0 / float(height));

varying highp vec2 texCoordOutput;

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
