#version 430 core
in vec4 fsBlur;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 blur;

void main()
{
    blur = fsBlur;
}
