attribute highp vec2 vertexPosition;
attribute highp vec2 textureCoord;
uniform highp mat4 modelViewProjection;
varying highp vec2 vTextureCoord;

void main()
{
    vTextureCoord = textureCoord;
    gl_Position = modelViewProjection * vec4(vertexPosition, 0.0, 1.0);
}
