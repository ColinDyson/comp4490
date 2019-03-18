//Created by:	Colin Dyson
//Student #:	7683407
//Interactive Spline Drawing Tool

#include "common.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

#define ToRadian(x) ((x) * M_PI / 180.0f)
#define ToDegree(x) ((x) * 180.0f / M_PI)

typedef glm::vec4 vec4;
typedef glm::vec3 vec3;

const char *WINDOW_TITLE = "Question 1";
const double FRAME_RATE_MS = 1000.0 / 60.0;

float bezier_array[] = {
	-1.0, 3.0, -3.0, 1.0,
	3.0, -6.0, 3.0, 0.0,
	-3.0, 3.0, 0.0, 0.0,
	1.0, 0.0, 0.0, 0.0
};

float catmull_array0[] = {
	2, -2, 1, 1,
	-3, 3, -2, -1,
	0, 0, 1, 0,
	1, 0, 0, 0
};

float catmull_array1[] = {
	0, 1, 0, 0,
	0, 0, 1, 0,
	-0.5, 0, 0.5, 0,
	0, -0.5, 0, 0.5
};

float bspline_array[] = {
	-1, 3, -3, 1,
	3, -6, 3, 0,
	-3, 0, 3, 0,
	1, 4, 1, 0
};

glm::mat4 bezier_matrix = glm::make_mat4(bezier_array);
glm::mat4 catmull_matrix0 = glm::make_mat4(catmull_array0);
glm::mat4 catmull_matrix1 = glm::make_mat4(catmull_array1);
glm::mat4 bspline_matrix = glm::make_mat4(bspline_array);

// RGBA colors
vec4 colors[8] = {
	vec4(0.0, 0.0, 0.0, 1.0),  // black
	vec4(1.0, 0.0, 0.0, 1.0),  // red
	vec4(1.0, 1.0, 0.0, 1.0),  // yellow
	vec4(0.0, 1.0, 0.0, 1.0),  // green
	vec4(0.0, 0.0, 1.0, 1.0),  // blue
	vec4(1.0, 0.0, 1.0, 1.0),  // magenta
	vec4(1.0, 1.0, 1.0, 1.0),  // white
	vec4(0.0, 1.0, 1.0, 1.0)   // cyan
};

std::vector<vec4> control_points = {
	vec4(-0.8, -0.5, 0, 1),
	vec4(-0.4, 0.5, 0, 1),
	vec4(0.4, 0.5, 0, 1),
	vec4(0.8, -0.5, 0, 1)
};

std::vector<vec4> cp_markers, curve_points;

enum CurveMode {
	BEZIER,
	CATMULLROM,
	BSPLINE
};

CurveMode curr_curve;

//Uniforms
//GLuint  model_view, projection;
GLuint v_position, draw_color_location;

int selected_cp = -1;
float t_step = 0.01;

void
setColor(vec4 color) {
	glUniform4fv(draw_color_location, 1, glm::value_ptr(color));
}

void
setBuffer() {
	//Fill the control point markers buffer
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, (cp_markers.size() + curve_points.size()) * sizeof(vec4), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, cp_markers.size() * sizeof(vec4), &cp_markers[0].x);
	glBufferSubData(GL_ARRAY_BUFFER, cp_markers.size() * sizeof(vec4), curve_points.size() * sizeof(vec4), &curve_points[0].x);
	glEnableVertexAttribArray(v_position);
	glVertexAttribPointer(v_position, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
}

void
genCPMarkers() {
	float marker_offset = 0.02;
	cp_markers.clear();

	//For each control point, we draw an X marking it, so we create 4 points to draw lines between
	for (int i = 0; i < control_points.size(); i++) {
		vec4 curr_point = control_points[i];
		cp_markers.push_back(vec4(curr_point.x - marker_offset, curr_point.y - marker_offset, curr_point.z, 1));
		cp_markers.push_back(vec4(curr_point.x + marker_offset, curr_point.y + marker_offset, curr_point.z, 1));
		cp_markers.push_back(vec4(curr_point.x - marker_offset, curr_point.y + marker_offset, curr_point.z, 1));
		cp_markers.push_back(vec4(curr_point.x + marker_offset, curr_point.y - marker_offset, curr_point.z, 1));
	}
}

void 
evaluateBezier() {
	for (int i = 0; i + 3 < control_points.size(); i += 3) {
		float cps[] = {
			control_points[i].x, control_points[i].y, 0, 1,
			control_points[i + 1].x, control_points[i + 1].y, 0, 1,
			control_points[i + 2].x, control_points[i + 2].y, 0, 1,
			control_points[i + 3].x, control_points[i + 3].y, 0, 1
		};
		glm::mat4 cps_matrix = glm::make_mat4(cps);
		cps_matrix = cps_matrix * bezier_matrix;
		for (float t = 0; t < 1; t += t_step) { 
			vec4 temp = { pow(t, 3), pow(t, 2), t, 1 };
			vec4 new_point = vec4(cps_matrix * temp);
			curve_points.push_back(new_point);
		}
	}
}

void 
evaluateCatmullrom() {
	for (int i = 1; i + 2 < control_points.size(); i ++) {
		float cps[] = {
			control_points[i - 1].x, control_points[i - 1].y, 0, 1,
			control_points[i].x, control_points[i].y, 0, 1,
			control_points[i + 1].x, control_points[i + 1].y, 0, 1,
			control_points[i + 2].x, control_points[i + 2].y, 0, 1
		};
		glm::mat4 cps_matrix = glm::make_mat4(cps);
		cps_matrix = cps_matrix * catmull_matrix1 * catmull_matrix0;
		for (float t = 0; t < 1; t += t_step) {
			vec4 temp = { pow(t, 3), pow(t, 2), t, 1 };
			vec4 new_point = vec4(cps_matrix * temp);
			curve_points.push_back(new_point);
		}
	}
}

void 
evaluateBspline() {
	for (int i = 1; i + 2 < control_points.size(); i++) {
		float cps[] = {
			control_points[i - 1].x, control_points[i - 1].y, 0, 1,
			control_points[i].x, control_points[i].y, 0, 1,
			control_points[i + 1].x, control_points[i + 1].y, 0, 1,
			control_points[i + 2].x, control_points[i + 2].y, 0, 1
		};
		glm::mat4 cps_matrix = glm::make_mat4(cps);
		cps_matrix = cps_matrix * bspline_matrix;
		cps_matrix = 0.166666f * cps_matrix;
		for (float t = 0; t < 1; t += t_step) {
			vec4 temp = { pow(t, 3), pow(t, 2), t, 1 };
			vec4 new_point = vec4(cps_matrix * temp);
			curve_points.push_back(new_point);
		}
	}
}

void 
evaluateCurve() {
	curve_points.clear();
	switch (curr_curve)
	{
	case BEZIER:
		evaluateBezier();
		break;
	case CATMULLROM:
		evaluateCatmullrom();
		break;
	case BSPLINE:
		evaluateBspline();
		break;
	default:
		break;
	}
	setBuffer();
}

void
changeCurve() {
	switch (curr_curve)
	{
	case BEZIER:
		std::cout << "Evaluating curve as a Catmull-Rom Spline." << std::endl;
		curr_curve = CATMULLROM;
		break;
	case CATMULLROM:
		std::cout << "Evaluating curve as a B Spline." << std::endl;
		curr_curve = BSPLINE;
		break;
	case BSPLINE:
		std::cout << "Evaluating curve as a Bezier Spline." << std::endl;
		curr_curve = BEZIER;
		break;
	default:
		break;
	}
	evaluateCurve();
}

void
handleClick(int x_window, int y_window) {
	float x = (float)x_window / (float)glutGet(GLUT_WINDOW_WIDTH) * 2 - 1;
	float y = (float)y_window / (float)glutGet(GLUT_WINDOW_HEIGHT) * -2 + 1;

	if (selected_cp == -1) {
		//If no control point is currently selected, check if one was clicked on
		for (int i = 0; i < control_points.size() && selected_cp == -1; i++) {
			float square_dist = pow(x - control_points[i].x, 2) + pow(y - control_points[i].y, 2);
			if (square_dist <= 0.004) {
				selected_cp = i;
			}
		}
		//If no control point was clicked on, create a new one
		if (selected_cp == -1) {
			std::cout << "Creating new Control Point at " << x << ", " << y << std::endl;
			control_points.push_back(vec4(x, y, 0, 1));
			genCPMarkers();
			evaluateCurve();
			setBuffer();
		}
	}
	//If a control point is selected, move it to the new location
	else {
		std::cout << "Moving control point " << selected_cp << " to [" << x << ", " << y << "]" << std::endl;
		control_points[selected_cp] = vec4(x, y, 0, 1);
		selected_cp = -1;
		genCPMarkers();
		evaluateCurve();
		setBuffer();
	}
}


// OpenGL initialization
void
init() 
{
	genCPMarkers();

	std::cout << "Evaluating curve as a Bezier Spline." << std::endl;
	curr_curve = BEZIER;
	evaluateCurve();
	
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint program = InitShader("vshader.v.glsl", "fshader.f.glsl");
	glUseProgram(program);

	v_position = glGetAttribLocation(program, "v_position");

	setBuffer();

	draw_color_location = glGetUniformLocation(program, "draw_color");

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

//----------------------------------------------------------------------------

void
display(void) 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < control_points.size(); i++) {
		if (selected_cp == i) {
			setColor(colors[1]);
		}
		else {
			setColor(colors[6]);
		}
		glDrawArrays(GL_LINES, i * 4, 4);
	}

	setColor(colors[3]);
	glDrawArrays(GL_LINE_STRIP, cp_markers.size(), curve_points.size());

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y) 
{
	switch (key) {
	case ' ': // Spacebar)
		changeCurve();
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
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			handleClick(x, y);
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
	glViewport(0, 0, width, height);

	GLfloat aspect = GLfloat(width) / height;
	glm::mat4  proj = glm::perspective(glm::radians(45.0f), aspect, 0.5f, 80.0f);

	//glUniformMatrix4fv(projection, 1, GL_FALSE, glm::value_ptr(proj));
}