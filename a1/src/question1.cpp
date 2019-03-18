//Created by:	Colin Dyson
//Student #:	7683407
//Display a robot running in a circle on a checkerboard plane

#include "common.h"
#include "icosphere.cpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char *WINDOW_TITLE = "Question 1";
const double FRAME_RATE_MS = 1000.0 / 60.0;

const float PI = 3.1415;

typedef glm::vec4 color4;
typedef glm::vec4 point4;
typedef glm::vec3 vec3;

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 cube_vertices[8] = {
	point4(-0.5, -0.5,  0.5, 1.0),
	point4(-0.5,  0.5,  0.5, 1.0),
	point4(0.5,  0.5,  0.5, 1.0),
	point4(0.5, -0.5,  0.5, 1.0),
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(-0.5,  0.5, -0.5, 1.0),
	point4(0.5,  0.5, -0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0)
};

GLuint cube_indices[] = {
	// each group of 3 is a triangle face
	1, 0, 3, 1, 3, 2,
	2, 3, 7, 2, 7, 6,
	3, 0, 4, 3, 4, 7,
	6, 5, 1, 6, 1, 2,
	4, 5, 6, 4, 6, 7,
	5, 4, 0, 5, 0, 1
};

// RGBA colors
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

vec3 model_pos[] = {
	vec3(0.0,	6.0,	0.0),		//Robot
	vec3(0.0,	0.0,	0.0),		//Robot Torso
	vec3(0.0,	2.5,	0.0),		//Robot Head
	vec3(-1.5,	0.7,	0.0),		//Robot Right Arm
	vec3(1.5,	0.7,	0.0),		//Robot Left Arm
	vec3(-0.7,	-2.0,	0.0),		//Robot Right Leg
	vec3(0.7,	-2.0,	0.0)		//Robot Left Leg
};

vec3 model_scales[] = {
	vec3(1, 1, 1),
	vec3(2, 3, 2),
	vec3(0.8, 0.8, 0.8),
	vec3(0.5, 2, 0.5),
	vec3(0.5, 2, 0.5),
	vec3(0.6, 2.4, 0.6),
	vec3(0.6, 2.4, 0.6)
};

GLuint tube_indices[] = {	//Every group of 3 is a triangle face
	0, 1, 3, 0, 3, 2,
	2, 3, 5, 2, 5, 4,
	4, 5, 7, 4, 7, 6,
	6, 7, 9, 6, 9, 8,
	8, 9, 11, 8, 11, 10,
	10, 11, 1, 10, 1, 0
};

GLuint  ModelView, Projection, vPosition, CurrColor;

const int SUBDIVISIONS = 3;

GLuint ico_indices_count, ico_vertices_count;

float t = 0;
float limb_sway = 0.0f;	//Limbs sway between angles of 5PI/4 and 3PI/4
float limb_direction = 1.0f;

//Generates a tube with 1/t_step sides and a height of 1
void 
genTube(float t_step, std::vector<glm::vec4> &tube_vertices)
{
	for (float t = 0; t < t_step; t ++) {
		tube_vertices.push_back(glm::vec4(sin(t / 6 * 2 * PI), 0.5, cos(t / 6 * 2 * PI), 1.0));
		tube_vertices.push_back(glm::vec4(sin(t / 6 * 2 * PI), -0.5, cos(t / 6 * 2 * PI), 1.0));
	}
}

void
drawTile()
{
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	for (int i = 0; i < sizeof(cube_indices) / sizeof(GLuint); i += 3) {
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));
	}
}

void
drawCube()
{
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	for (int i = 0; i < sizeof(cube_indices) / sizeof(GLuint); i += 3) {
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));
	}
}

void
drawSphere()
{	//Set vertices pointer to beginning of ico_vertices in our buffer
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(cube_vertices))); 
	glDrawElements(GL_TRIANGLES, ico_indices_count, GL_UNSIGNED_INT, BUFFER_OFFSET(sizeof(cube_indices)));
}

void
drawTube()
{
	//Draw the sides (composed of triangles)
	//Set vertices offset to the end of ico_vertices in our buffer
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(cube_vertices) + (ico_vertices_count * sizeof(glm::vec4))));
	for (int i = 0; i < sizeof(tube_indices) / sizeof(GLuint); i += 3) {
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)(sizeof(cube_indices) + (ico_indices_count * sizeof(GLuint)) + (i * sizeof(GLuint))));
	}
}

// OpenGL initialization
void
init() 
{
	static std::vector<glm::vec4> ico_vertices;
	static std::vector<GLuint> ico_indices;
	icosphere(SUBDIVISIONS, ico_vertices, ico_indices);
	ico_indices_count = ico_indices.size();
	ico_vertices_count = ico_vertices.size();

	//Generate the vertices for a hexagonal tub
	static std::vector<glm::vec4> prism_vertices;
	genTube(6, prism_vertices); 

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	
	// Create and initialize a buffer object
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices) + (ico_vertices.size() * sizeof(glm::vec4)) + (prism_vertices.size() * sizeof(glm::vec4))
				, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cube_vertices), cube_vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices), ico_vertices.size() * sizeof(glm::vec4), &ico_vertices[0].x);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices) + ico_vertices.size() * sizeof(glm::vec4), prism_vertices.size() * sizeof(glm::vec4), &prism_vertices[0].x);
	
	// Another for the index buffer
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_vertices) + (ico_vertices.size() * sizeof(glm::vec4)) + sizeof(tube_indices)
				, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(cube_indices), cube_indices);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), ico_indices.size() * sizeof(GLuint), &ico_indices[0]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices) + ico_indices.size() * sizeof(GLuint), sizeof(tube_indices), tube_indices);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("a1q1vshader.glsl", "a1q1fshader.glsl");
	glUseProgram(program);

	// set up vertex arrays
	vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");
	CurrColor = glGetUniformLocation(program, "CurrColor");

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

//----------------------------------------------------------------------------

void
display(void) 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//  Generate the model-view matrix
	const glm::vec3 viewer_pos(0.0, 5.0, 40.0);
	const float viewer_rot = PI / 6;
	const float path_radius = 15.0f;
	glm::mat4 view_trans, robot_trans, tile_trans, model_view;
	color4 model_color;

	view_trans = glm::translate(view_trans, -viewer_pos);
	view_trans = glm::rotate(view_trans, viewer_rot, vec3(1, 0, 0));

	// Draw floor
	for (int i = -5; i < 6; i++) {
		for (int j = -5; j < 6; j++) {
			if ((i + j) % 2 == 0) {
				model_color = colors[0];
			}
			else {
				model_color = colors[6];
			}
			glUniform4fv(CurrColor, 1, glm::value_ptr(model_color)); //Set color for 'checkerboard' effect
			model_view = view_trans;
			model_view = glm::translate(model_view, vec3(4*i, 0, 4*j));
			model_view = glm::scale(model_view, vec3(4, 1, 4));
			glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
			drawTile();
		}
	}

	//Move Robot
	robot_trans = view_trans;
	robot_trans = glm::rotate(robot_trans, t * 2 * PI, vec3(0.0f, 1.0f, 0.0f)); //Rotation around 0,0
	robot_trans = glm::translate(robot_trans, vec3(path_radius, 0.0f, 0.0f));   //Push path of robot to a set radius
	robot_trans = glm::translate(robot_trans, model_pos[0]);
	robot_trans = glm::scale(robot_trans, model_scales[0]);

	//Draw Torso
	model_color = colors[3];
	glUniform4fv(CurrColor, 1, glm::value_ptr(model_color));
	model_view = robot_trans;
	model_view = glm::translate(model_view, model_pos[1]);
	model_view = glm::scale(model_view, model_scales[1]);
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	drawCube();

	//Draw Head
	model_color = colors[1];
	glUniform4fv(CurrColor, 1, glm::value_ptr(model_color));
	model_view = robot_trans;
	model_view = glm::translate(model_view, model_pos[2]);
	model_view = glm::scale(model_view, model_scales[2]);
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	drawSphere();
	
	//Draw Arms
	model_color = colors[4];
	glUniform4fv(CurrColor, 1, glm::value_ptr(model_color));
	model_view = robot_trans;
	model_view = glm::translate(model_view, model_pos[3]);
	model_view = glm::rotate(model_view, limb_sway, vec3(1.0f, 0.0f, 0.0f));
	model_view = glm::translate(model_view, vec3(0.0f, -0.5f, 0.0f));
	model_view = glm::scale(model_view, model_scales[3]);
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	drawTube();

	model_view = robot_trans;
	model_view = glm::translate(model_view, model_pos[4]);
	model_view = glm::rotate(model_view, -limb_sway, vec3(1.0f, 0.0f, 0.0f));
	model_view = glm::translate(model_view, vec3(0.0f, -0.5f, 0.0f));
	model_view = glm::scale(model_view, model_scales[4]);
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	drawTube();

	//Draw Legs
	model_color = colors[7];
	glUniform4fv(CurrColor, 1, glm::value_ptr(model_color));
	model_view = robot_trans;
	model_view = glm::translate(model_view, model_pos[5]);
	model_view = glm::rotate(model_view, limb_sway, vec3(1.0f, 0.0f, 0.0f));
	model_view = glm::translate(model_view, vec3(0.0f, -0.7f, 0.0f));
	model_view = glm::scale(model_view, model_scales[5]);
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	drawTube();

	model_view = robot_trans;
	model_view = glm::translate(model_view, model_pos[6]);
	model_view = glm::rotate(model_view, -limb_sway, vec3(1.0f, 0.0f, 0.0f));
	model_view = glm::translate(model_view, vec3(0.0f, -0.7f, 0.0f));
	model_view = glm::scale(model_view, model_scales[6]);
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	drawTube();

	glutSwapBuffers();
	t += 0.001;
	if (abs(1.0 - t) < 0.001) {
		t = 0.0;
	}

	limb_sway += 0.01f * limb_direction * 2.0f * PI;
	if (abs(limb_sway) >= PI / 3.0f) {
		limb_direction *= -1;
	}
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
	glm::mat4  projection = glm::perspective(glm::radians(45.0f), aspect, 0.5f, 80.0f);

	glUniformMatrix4fv(Projection, 1, GL_FALSE, glm::value_ptr(projection));
}