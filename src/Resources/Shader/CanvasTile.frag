uniform sampler2D tileTexture;
varying highp vec2 vTextureCoord;

void main()
{
    gl_FragColor = texture2D(tileTexture, vTextureCoord);
}
