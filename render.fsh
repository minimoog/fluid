uniform sampler2D texUnit0;         //height field
uniform sampler2D texUnit1;         //texture
uniform highp float width;
uniform highp float height;

highp vec2 step_w = vec2(1.0 / float(width), 0.0);
highp vec2 step_h = vec2(0.0, 1.0 / float(height));

varying highp vec2 texCoordOutput;

void main(void)
{

    highp float left  = texture2D(texUnit0, texCoordOutput - step_w).y;
    highp float right = texture2D(texUnit0, texCoordOutput + step_w).y;
    highp float up    = texture2D(texUnit0, texCoordOutput - step_h).y;
    highp float down  = texture2D(texUnit0, texCoordOutput + step_h).y;

    highp vec3 norm = normalize(vec3(left - right, up - down, 2.0));

    gl_FragColor = texture2D(texUnit1, texCoordOutput + norm.xy);
}
