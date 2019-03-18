#version 150

in vec4 vPosition;
uniform vec4 CurrColor;
uniform mat4 ModelView, Projection;
out vec4 color;
out float zdepth;

void main()
{
    gl_Position = Projection * ModelView * vPosition;
    color = CurrColor;
    zdepth = vPosition.z;
}