//Created by:	Colin Dyson
//Student #:	7683407
//Evaluate Control Points as Bezier Patch

#include "common.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

typedef glm::vec4 vec4;
typedef glm::vec3 vec3;
typedef glm::mat4 mat4;

const char *WINDOW_TITLE = "Question 2";
const double FRAME_RATE_MS = 1000.0 / 60.0;

float bezier_array[] = {
	-1.0, 3.0, -3.0, 1.0,
	3.0, -6.0, 3.0, 0.0,
	-3.0, 3.0, 0.0, 0.0,
	1.0, 0.0, 0.0, 0.0
};

mat4 bezier_matrix = glm::make_mat4(bezier_array);
mat4 bezier_t_matrix = glm::transpose(bezier_matrix);

// RGBA colors
vec4 colors[7] = {
	//vec4(0.0, 0.0, 0.0, 1.0),  // black
	vec4(1.0, 0.0, 0.0, 1.0),  // red
	vec4(1.0, 1.0, 0.0, 1.0),  // yellow
	vec4(0.0, 1.0, 0.0, 1.0),  // green
	vec4(0.0, 0.0, 1.0, 1.0),  // blue
	vec4(1.0, 0.0, 1.0, 1.0),  // magenta
	vec4(1.0, 1.0, 1.0, 1.0),  // white
	vec4(0.0, 1.0, 1.0, 1.0)   // cyan
};

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };
int rotating = 0;

enum DrawModes{ Curve = 0, Both = 1, Wireframe = 2, NumModes = 3 };
int drawMode = Curve;

//Uniforms
GLuint  model_view_loc, projection_loc;
GLuint v_position_loc, draw_color_loc;

GLuint vaos[2];

char *patch_file = "teapot";
int num_patches;
std::vector<GLuint> cp_indices; //Every 16 indices describes 1 patch
std::vector<vec4> control_points;
std::vector<GLuint> patch_indices; //Used to display wireframed control points
std::vector<vec4> curve_vertices;
std::vector<vec4> curve_vertices_t;

float resolution = 10;
float t_step = 1.0f / resolution;

void
setColor(vec4 color) {
	glUniform4fv(draw_color_loc, 1, glm::value_ptr(color));
}

void
bPatch(int ii, int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n, int o, int p) {
	cp_indices.push_back(a); cp_indices.push_back(b); cp_indices.push_back(c); cp_indices.push_back(d);
	cp_indices.push_back(e); cp_indices.push_back(f); cp_indices.push_back(g); cp_indices.push_back(h);
	cp_indices.push_back(i); cp_indices.push_back(j); cp_indices.push_back(k); cp_indices.push_back(l);
	cp_indices.push_back(m); cp_indices.push_back(n); cp_indices.push_back(o); cp_indices.push_back(p);

	//We draw 8 line strips per patch for the control point wireframe
	patch_indices.push_back(a); patch_indices.push_back(b); patch_indices.push_back(c); patch_indices.push_back(d);
	patch_indices.push_back(e); patch_indices.push_back(f); patch_indices.push_back(g); patch_indices.push_back(h);
	patch_indices.push_back(i); patch_indices.push_back(j); patch_indices.push_back(k); patch_indices.push_back(l);
	patch_indices.push_back(m); patch_indices.push_back(n); patch_indices.push_back(o); patch_indices.push_back(p);

	patch_indices.push_back(a); patch_indices.push_back(e); patch_indices.push_back(i); patch_indices.push_back(m);
	patch_indices.push_back(b); patch_indices.push_back(f); patch_indices.push_back(j); patch_indices.push_back(n);
	patch_indices.push_back(c); patch_indices.push_back(g); patch_indices.push_back(k); patch_indices.push_back(o);
	patch_indices.push_back(d); patch_indices.push_back(h); patch_indices.push_back(l); patch_indices.push_back(p);
}

void
bPoint(int ii, float x, float y, float z) {
	control_points.push_back(vec4(x, y, z, 1));
}

//Modified from provided input_patches.c
void loadPatches() {
	int num_verticies;
	int ii;
	float x, y, z;
	int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;

	FILE *fp;

	if (!(fp = fopen(patch_file, "r"))) {
		fprintf(stderr, "Load_patch: Can't open %s\n", patch_file);
		exit(1);
	}

	(void)fscanf(fp, "%i\n", &num_patches);
	for (ii = 0; ii < num_patches; ii++) {
		(void)fscanf(fp, "%i, %i, %i, %i,", &a, &b, &c, &d);
		(void)fscanf(fp, "%i, %i, %i, %i,", &e, &f, &g, &h);
		(void)fscanf(fp, "%i, %i, %i, %i,", &i, &j, &k, &l);
		(void)fscanf(fp, "%i, %i, %i, %i\n", &m, &n, &o, &p);
		bPatch(ii, a - 1, b - 1, c - 1, d - 1, e - 1, f - 1, g - 1, h - 1, i - 1, j - 1, k - 1, l - 1, m - 1, n - 1, o - 1, p - 1);
	}
	(void)fscanf(fp, "%i\n", &num_verticies);
	for (ii = 1; ii <= num_verticies; ii++) {
		(void)fscanf(fp, "%f, %f, %f\n", &x, &y, &z);
		bPoint(ii, x, y, z);
	}
}

void constructPatches() {
	//for each set of 16 indices, we evaluate a patch
	mat4 cpx, cpy, cpz, cpx_t, cpy_t, cpz_t, xpatch_matrix, ypatch_matrix, zpatch_matrix, xpatch_matrix_t, ypatch_matrix_t, zpatch_matrix_t;
	vec4 uvector, vvector, temp;
	float xnew, ynew, znew, xnew_t, ynew_t, znew_t;
	for (int i = 0; i < cp_indices.size(); i += 16) {
		std::vector<vec4> patch_vertices;
		float cpx_array[] = {
			control_points[cp_indices[i]].x, control_points[cp_indices[i + 1]].x, control_points[cp_indices[i + 2]].x, control_points[cp_indices[i + 3]].x,
			control_points[cp_indices[i + 4]].x, control_points[cp_indices[i + 5]].x,  control_points[cp_indices[i + 6]].x,  control_points[cp_indices[i + 7]].x,
			control_points[cp_indices[i + 8]].x, control_points[cp_indices[i + 9]].x,  control_points[cp_indices[i + 10]].x,  control_points[cp_indices[i +11]].x,
			control_points[cp_indices[i + 12]].x, control_points[cp_indices[i + 13]].x,  control_points[cp_indices[i + 14]].x, control_points[cp_indices[i + 15]].x
		};

		float cpy_array[] = {
			control_points[cp_indices[i]].y, control_points[cp_indices[i + 1]].y, control_points[cp_indices[i + 2]].y, control_points[cp_indices[i + 3]].y,
			control_points[cp_indices[i + 4]].y, control_points[cp_indices[i + 5]].y,  control_points[cp_indices[i + 6]].y,  control_points[cp_indices[i + 7]].y,
			control_points[cp_indices[i + 8]].y, control_points[cp_indices[i + 9]].y,  control_points[cp_indices[i + 10]].y,  control_points[cp_indices[i + 11]].y,
			control_points[cp_indices[i + 12]].y, control_points[cp_indices[i + 13]].y,  control_points[cp_indices[i + 14]].y, control_points[cp_indices[i + 15]].y
		};

		float cpz_array[] = {
			control_points[cp_indices[i]].z, control_points[cp_indices[i + 1]].z, control_points[cp_indices[i + 2]].z, control_points[cp_indices[i + 3]].z,
			control_points[cp_indices[i + 4]].z, control_points[cp_indices[i + 5]].z,  control_points[cp_indices[i + 6]].z,  control_points[cp_indices[i + 7]].z,
			control_points[cp_indices[i + 8]].z, control_points[cp_indices[i + 9]].z,  control_points[cp_indices[i + 10]].z,  control_points[cp_indices[i + 11]].z,
			control_points[cp_indices[i + 12]].z, control_points[cp_indices[i + 13]].z,  control_points[cp_indices[i + 14]].z, control_points[cp_indices[i + 15]].z
		};
		//Create control point matrices for x, y, and z
		//A second calc is made for the transpose of the points matrices, to create a set of vertices going in the other direction for the full wireframeq
		cpx = glm::make_mat4(cpx_array); cpx_t = glm::transpose(cpx);
		cpy = glm::make_mat4(cpy_array); cpy_t = glm::transpose(cpy);
		cpz = glm::make_mat4(cpz_array); cpz_t = glm::transpose(cpz);

		xpatch_matrix = bezier_t_matrix * cpx; xpatch_matrix_t = bezier_t_matrix * cpx_t;
		xpatch_matrix = xpatch_matrix * bezier_matrix; xpatch_matrix_t = xpatch_matrix_t * bezier_matrix;
		ypatch_matrix = bezier_t_matrix * cpy; ypatch_matrix_t = bezier_t_matrix * cpy_t;
		ypatch_matrix = ypatch_matrix * bezier_matrix; ypatch_matrix_t = ypatch_matrix_t * bezier_matrix;
		zpatch_matrix = bezier_t_matrix * cpz; zpatch_matrix_t = bezier_t_matrix * cpz_t;
		zpatch_matrix = zpatch_matrix * bezier_matrix; zpatch_matrix_t = zpatch_matrix_t * bezier_matrix;

		for (int u_int = 0; u_int < resolution; u_int ++) {
			float u = (float)u_int / resolution;
			uvector = vec4(pow(u, 3), pow(u, 2), u, 1);
			for (int v_int = 0; v_int < resolution; v_int ++) {
				float v = (float)(v_int) / resolution;
				vvector = vec4(pow(v, 3), pow(v, 2), v, 1);

				temp = vvector * xpatch_matrix;
				xnew = glm::dot(temp, uvector);
				temp = vvector * xpatch_matrix_t;
				xnew_t = glm::dot(temp, uvector);

				temp = vvector * ypatch_matrix;
				ynew = glm::dot(temp, uvector);
				temp = vvector * ypatch_matrix_t;
				ynew_t = glm::dot(temp, uvector);

				temp = vvector * zpatch_matrix;
				znew = glm::dot(temp, uvector);
				temp = vvector * zpatch_matrix_t;
				znew_t = glm::dot(temp, uvector);

				curve_vertices.push_back(vec4(xnew, ynew, znew, 1));
				curve_vertices_t.push_back(vec4(xnew_t, ynew_t, znew_t, 1));
			}
		}
	}
}

// OpenGL initialization
void
init()
{
	loadPatches();
	std::cout << "Loaded " << control_points.size() << " Control Points in " << num_patches << " patches" << std::endl;
	constructPatches();

	GLuint program = InitShader("teapotvshader.v.glsl", "teapotfshader.f.glsl");
	glUseProgram(program);

	v_position_loc = glGetAttribLocation(program, "vPosition");
	draw_color_loc = glGetUniformLocation(program, "DrawColor");
	model_view_loc = glGetUniformLocation(program, "ModelView");
	projection_loc = glGetUniformLocation(program, "Projection");

	GLuint buffer;
	glGenVertexArrays(2, &vaos[0]);
	glBindVertexArray(vaos[0]);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, control_points.size() * sizeof(vec4), &control_points[0].x, GL_STATIC_DRAW);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, patch_indices.size() * sizeof(GLuint), &patch_indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(v_position_loc);
	glVertexAttribPointer(v_position_loc, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindVertexArray(0);

	glBindVertexArray(vaos[1]);
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, (curve_vertices_t.size() * sizeof(vec4)) + (curve_vertices.size() * sizeof(vec4)), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, curve_vertices.size() * sizeof(vec4), &curve_vertices[0].x);
	glBufferSubData(GL_ARRAY_BUFFER, curve_vertices.size() * sizeof(vec4), curve_vertices_t.size() * sizeof(vec4), &curve_vertices_t[0].x);

	glEnableVertexAttribArray(v_position_loc);
	glVertexAttribPointer(v_position_loc, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

//----------------------------------------------------------------------------

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const glm::vec3 viewer_pos(0.0, 0.0, 10.0);
	mat4 model_view, trans, rot;
	trans = glm::translate(trans, -viewer_pos);
	rot = glm::rotate(rot, glm::radians(Theta[Xaxis]), glm::vec3(1, 0, 0));
	rot = glm::rotate(rot, glm::radians(Theta[Yaxis]), glm::vec3(0, 1, 0));
	rot = glm::rotate(rot, glm::radians(Theta[Zaxis]), glm::vec3(0, 0, 1));
	model_view = trans * rot;
	glUniformMatrix4fv(model_view_loc, 1, GL_FALSE, glm::value_ptr(model_view));

	if (drawMode == Both || drawMode == Wireframe) {
		glBindVertexArray(vaos[0]);
		setColor(colors[5]);
		for (int i = 0; i < patch_indices.size(); i += 4) {
			glDrawElements(GL_LINE_STRIP, 4, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));
		}
		glBindVertexArray(0);
	}

	if (drawMode == Both || drawMode == Curve) {
		glBindVertexArray(vaos[1]);
		setColor(colors[2]);
		for (int i = 0; i < curve_vertices.size(); i += resolution) {
			glDrawArrays(GL_LINE_STRIP, i, resolution);
		}

		for (int i = 0; i < curve_vertices_t.size(); i += resolution) {
			glDrawArrays(GL_LINE_STRIP, curve_vertices.size() + i, resolution);
		}
	}

	glBindVertexArray(0);

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case ' ':
		drawMode = (drawMode + 1) % NumModes;
		break;
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
		case GLUT_LEFT_BUTTON:
			if (Axis == Xaxis) {
				if (rotating) {
					rotating = 0;
				}
				else {
					rotating = 1;
				}
			}
			Axis = Xaxis;
			break;
		case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;
			if (Axis == Yaxis) {
				if (rotating) {
					rotating = 0;
				}
				else {
					rotating = 1;
				}
			}
			Axis = Yaxis;
			break;
		case GLUT_RIGHT_BUTTON:
			if (Axis == Zaxis) {
				if (rotating) {
					rotating = 0;
				}
				else {
					rotating = 1;
				}
			}
			Axis = Zaxis;
			break;
		}
	}
}

//----------------------------------------------------------------------------

void
update(void)
{
	if (rotating) {
		Theta[Axis] += 0.5;

		if (Theta[Axis] > 360.0) {
			Theta[Axis] -= 360.0;
		}
	}
}

//----------------------------------------------------------------------------

void
reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	GLfloat aspect = GLfloat(width) / height;
	glm::mat4  proj = glm::perspective(glm::radians(45.0f), aspect, 0.5f, 80.0f);

	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(proj));
}
