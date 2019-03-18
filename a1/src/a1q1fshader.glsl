#version 150

in vec4 color;
in float zdepth; 
out vec4 fcolor;

void main()
{ 
	if (color != vec4(0, 0, 0, 1) && color != vec4(1, 1, 1, 1)) {
		fcolor = vec4(color.x * zdepth + 0.5, color.y * zdepth + 0.5, color.z * zdepth + 0.5, 1);
	} else {
		fcolor = color;
	}
}
