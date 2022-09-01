#version 430 core
in vec2 fsTextureCoords;

uniform sampler2D sourceTexture;
uniform float widthRatio;
uniform float heightRatio;

out vec4 outColor;

void main()
{
    vec2 coords = vec2(fsTextureCoords.s * widthRatio, 1 - (1 - fsTextureCoords.t) * heightRatio);
    outColor = texture(sourceTexture, coords);
}
