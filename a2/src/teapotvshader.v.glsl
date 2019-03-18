#version 150

in vec4 vPosition;

uniform mat4 ModelView, Projection;
uniform vec4 DrawColor;

out vec4 Color;

void main() {
    Color = DrawColor;
    gl_Position = Projection * ModelView * vPosition;
}
