uniform sampler2D texUnit0;
uniform highp float width;
uniform highp float height;
uniform highp float k1;
uniform highp float k2;
uniform highp float k3;

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
