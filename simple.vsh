#ifdef GL_ES
precision lowp float;
#endif

attribute vec3 vertexPos;

varying vec2 texCoordOutput;

void main(void)
{    
    texCoordOutput = (vec2(vertexPos.x, vertexPos.y) + vec2(1.0)) * 0.5;

    gl_Position = vec4(vertexPos, 1);
}
