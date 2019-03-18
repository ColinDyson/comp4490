//Created by:	Colin Dyson
//Student #:	7683407
// Display a color cube
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We us an orthographic projection
//   as the default projetion.

#include "common.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

const char *WINDOW_TITLE = "CPU Color Cube";
const double FRAME_RATE_MS = 1000.0 / 60.0;

typedef glm::vec4  color4;
typedef glm::vec4  point4;

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
	point4(-0.5, -0.5,  0.5, 1.0),
	point4(-0.5,  0.5,  0.5, 1.0),
	point4(0.5,  0.5,  0.5, 1.0),
	point4(0.5, -0.5,  0.5, 1.0),
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(-0.5,  0.5, -0.5, 1.0),
	point4(0.5,  0.5, -0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0)
};

//Each group of 4 forms a square
GLuint indices[] = {
	0, 1, 2, 3,
	3, 2, 6, 7,
	7, 4, 5, 6,
	4, 0, 1, 5,
	1, 5, 2, 6,
	4, 0, 3, 7
};

// RGBA olors
color4 colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0),  // black
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(1.0, 1.0, 1.0, 1.0),  // white
	color4(0.0, 1.0, 1.0, 1.0)   // cyan
};

color4 ground_colors[] = {
	color4(0.0, 1.0, 0.0, 1.0),
	color4(0.0, 0.0, 1.0, 1.0)
};

void subdivideCube(int sub, std::vector<point4> &cube_vertices, std::vector<point4> &cube_indices) {
	int v_index = 0;
	point4 midpoint;
	for (point4 v : vertices) {
		cube_vertices.push_back(glm::vec4(v[0], v[1], v[2], 1));
	}
	cube_indices.assign((GLuint *)indices, (GLuint *)indices + (sizeof(indices) / sizeof(GLuint)));

	for (int s = 0; s < sub; s++) {
		int isize = cube_indices.size();
	}
}

// OpenGL initialization
void
init()
{
	std::vector<point4> cube_vertices;
	std::vector<point4> cube_indices;
	subdivideCube(1, cube_vertices, cube_indices);

	// Create a vertex array object
	GLuint ground_vao;
	glGenVertexArrays(1, &ground_vao);
	glBindVertexArray(ground_vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices) + sizeof(ground_colors), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);

	

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("a1q2vshader.glsl", "a1q2fshader.glsl");
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

//----------------------------------------------------------------------------

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033: // Escape Key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	}
}

//----------------------------------------------------------------------------

void
mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		switch (button) {
		case GLUT_LEFT_BUTTON:      break;
		case GLUT_MIDDLE_BUTTON:    break;
		case GLUT_RIGHT_BUTTON:     break;
		}
	}
}

//----------------------------------------------------------------------------

void
update(void)
{
}

//----------------------------------------------------------------------------

void
reshape(int width, int height)
{
}