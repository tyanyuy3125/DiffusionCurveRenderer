#version 430 core

in vec2 fsTextureCoords;

uniform sampler2D sourceTexture;
uniform sampler2D targetTexture;

out vec4 outColor;

void main()
{
    vec4 color = texture(targetTexture, fsTextureCoords);

    if (color.a > 0.1)
        outColor = color;
    else
        outColor = texture(sourceTexture, fsTextureCoords);
}
