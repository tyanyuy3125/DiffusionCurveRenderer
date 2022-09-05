#version 430 core

layout(points) in;
layout(triangle_strip, max_vertices = 8) out;

uniform mat4 projection;
uniform float pointsDelta;
uniform float diffusionWidth;
uniform float diffusionGap;
uniform float zoom;
uniform vec2 controlPoints[32];
uniform int controlPointsCount;

uniform vec4 leftColors[32];
uniform float leftColorPositions[32];
uniform int leftColorsCount;

uniform vec4 rightColors[32];
uniform float rightColorPositions[32];
uniform int rightColorsCount;

uniform float blurPointPositions[32];
uniform float blurPointStrengths[32];
uniform int blurPointsCount;

in float gsPoint[];

out vec4 fsColor;
out vec4 fsBlur;

float customPow(float x, float y)
{
    if (x == 0 && y == 0)
        return 1;

    return pow(x, y);
}

float factorial(int n)
{
    float result = 1;

    for (int i = 2; i <= n; ++i)
        result *= i;

    return result;
}

float choose(int n, int k)
{
    return factorial(n) / (factorial(k) * factorial(n - k));
}

vec2 valueAt(float t)
{
    vec2 value = vec2(0, 0);
    int degree = controlPointsCount - 1;

    for (int i = 0; i <= degree; ++i)
    {
        float c0 = choose(degree, i);
        float c1 = customPow(t, i);
        float c2 = customPow(1 - t, degree - i);

        value += c0 * c1 * c2 * controlPoints[i];
    }

    return value;
}

vec2 tangentAt(float t)
{
    vec2 tangent = vec2(0, 0);
    int degree = controlPointsCount - 1;

    for (int i = 0; i <= degree - 1; i++)
    {
        float c0 = choose(degree - 1, i);
        float c1 = customPow(t, i);
        float c2 = customPow(1 - t, degree - 1 - i);
        tangent += degree * c0 * c1 * c2 * (controlPoints[i + 1] - controlPoints[i]);
    }

    return normalize(tangent);
}

vec2 normalAt(float t)
{
    vec2 tangent = tangentAt(t);
    return normalize(vec2(-tangent.y, tangent.x));
}

vec4 leftColorAt(float t)
{
    for (int i = 1; i < leftColorsCount; i++)
    {
        float t0 = leftColorPositions[i - 1];
        float t1 = leftColorPositions[i];

        if (t0 <= t && t <= t1)
            return mix(leftColors[i - 1], leftColors[i], (t - t0) / (t1 - t0));
    }

    return vec4(0);
}

vec4 rightColorAt(float t)
{
    for (int i = 1; i < rightColorsCount; i++)
    {
        float t0 = rightColorPositions[i - 1];
        float t1 = rightColorPositions[i];

        if (t0 <= t && t <= t1)
            return mix(rightColors[i - 1], rightColors[i], (t - t0) / (t1 - t0));
    }

    return vec4(0);
}

vec3 blurAt(float t)
{
    for (int i = 1; i < blurPointsCount; i++)
    {
        float t0 = blurPointPositions[i - 1];
        float t1 = blurPointPositions[i];

        if (t0 <= t && t <= t1)
        {
            float mixedStrength = mix(blurPointStrengths[i - 1], blurPointStrengths[i], (t - t0) / (t1 - t0));
            return vec3(mixedStrength, mixedStrength, mixedStrength);
        }
    }

    return vec3(0);
}

void main()
{
    float t0 = gsPoint[0];
    float t1 = t0 + pointsDelta;

    vec2 v0 = valueAt(t0);
    vec2 v1 = valueAt(t1);

    vec2 n0 = normalAt(t0);
    vec2 n1 = normalAt(t1);

    vec4 l0 = leftColorAt(t0);
    vec4 l1 = leftColorAt(t1);

    vec4 r0 = rightColorAt(t0);
    vec4 r1 = rightColorAt(t1);

    // For blur
    float deltaRed0 = abs(l0.r - r0.r);
    float deltaGreen0 = abs(l0.g - r0.g);
    float deltaBlue0 = abs(l0.b - r0.b);

    float deltaRed1 = abs(l1.r - r1.r);
    float deltaGreen1 = abs(l1.g - r1.g);
    float deltaBlue1 = abs(l1.b - r1.b);

    float width = diffusionWidth * min(zoom, 1.0f);
    float gap = diffusionGap * min(zoom, 1.0f);

    vec4 b0 = vec4(blurAt(t0) * max(deltaRed0, max(deltaGreen0, deltaBlue0)), 1);
    vec4 b1 = vec4(blurAt(t1) * max(deltaRed1, max(deltaGreen1, deltaBlue1)), 1);

    // Left side
    {
        gl_Position = projection * vec4(v0 - 0.5f * gap * n0, 0, 1);
        fsColor = l0;
        fsBlur = b0;
        EmitVertex();

        gl_Position = projection * vec4(v0 - 0.5f * gap * n0 - 1.0f * width * n0, 0, 1);
        fsColor = l0;
        fsBlur = b0;
        EmitVertex();

        gl_Position = projection * vec4(v1 - 0.5f * gap * n1, 0, 1);
        fsColor = l1;
        fsBlur = b1;
        EmitVertex();

        gl_Position = projection * vec4(v1 - 0.5f * gap * n1 - 1.0f * width * n1, 0, 1);
        fsColor = l1;
        fsBlur = b1;
        EmitVertex();

        EndPrimitive();
    }

    // Right side
    {
        gl_Position = projection * vec4(v0 + 0.5f * gap * n0, 0, 1);
        fsColor = r0;
        fsBlur = b0;
        EmitVertex();

        gl_Position = projection * vec4(v0 + 0.5f * gap * n0 + 1.0f * width * n0, 0, 1);
        fsColor = r0;
        fsBlur = b0;
        EmitVertex();

        gl_Position = projection * vec4(v1 + 0.5f * gap * n1, 0, 1);
        fsColor = r1;
        fsBlur = b1;
        EmitVertex();

        gl_Position = projection * vec4(v1 + 0.5f * gap * n1 + 1.0f * width * n1, 0, 1);
        fsColor = r1;
        fsBlur = b1;
        EmitVertex();

        EndPrimitive();
    }
}
