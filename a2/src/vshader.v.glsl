#version 150

uniform vec4 draw_color;

in vec4 v_position;

out vec4 color;

void main()
{
    gl_Position = vec4(v_position.x, v_position.y, 0, 1);
    color = draw_color;
}
