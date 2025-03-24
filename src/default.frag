#version 330 core

out vec4 fColor;

in vec3 vColor;
in vec2 tCoord;
in vec3 vPos;

uniform sampler2D texture0;

void main()
{
    if (texture(texture0, tCoord).a < 0.1)
    {
        discard;
    }
    fColor =  texture(texture0, tCoord) * vec4(vColor, 1.0) * 1.0f;
}
