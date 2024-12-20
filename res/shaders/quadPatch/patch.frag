#version 460 core

uniform bool selected;
uniform vec4 color;

out vec4 FragColor;

in float outU;
in float outV;

void main()
{
    vec4 c = {color.r+0.0f, color.g, color.b+0.1f, color.a};
    FragColor = selected ? mix(vec4(0.5f, 0.4f, 0, c.a) , c, 0.2): c;
}