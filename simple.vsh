attribute highp vec3 vertexPos;
attribute highp vec2 vertexTexCoord;

//uniform highp mat4 viewMatrix;
uniform highp mat4 projectionMatrix;

varying highp vec2 texCoordOutput;

void main(void)
{
    texCoordOutput = vertexTexCoord;

    gl_Position = projectionMatrix  * vec4(vertexPos, 1);
}
