#version 430 core

in vec2 fsTextureCoords;

uniform sampler2D colorSourceTexture;
uniform sampler2D colorTargetTexture;

uniform sampler2D blurSourceTexture;
uniform sampler2D blurTargetTexture;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBlur;

void main()
{
    // Color
    vec4 color = texture(colorTargetTexture, fsTextureCoords);

    if (color.a > 0.1f)
        outColor = color;
    else
        outColor = texture(colorSourceTexture, fsTextureCoords);


    // Blur
    vec4 blur = texture(blurTargetTexture, fsTextureCoords);

    if (blur.a > 0.1f)
        outBlur = blur;
    else
        outBlur = texture(blurSourceTexture, fsTextureCoords);
}
