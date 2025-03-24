#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTex;

out vec3 vColor;
out vec2 tCoord;
out vec3 vPos;

uniform mat4 matrix;
uniform mat4 cam;
uniform mat4 proj;

void main()
{
    vPos = vec3(matrix * vec4(aPos, 1.0f));

    gl_Position = proj * cam * matrix * vec4(vPos, 1.0);
    vColor = aColor;
    tCoord = aTex;
}