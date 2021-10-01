#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <cmath>

#include "shaderClass.h"
#include "VBO.h"
#include "EBO.h"
#include "VAO.h"

#include "Node.h"

std::vector<float> circle(float w, float h, float x, float y, float radius, int point_count, bool state);
void join_nodes(Node* a, Node* b, float rest_length);

int main() {
	
	glfwInit();//initialize GLFW

	//specify GLFW version and profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//declare window size
	const unsigned int WINDOW_W = 800;
	const unsigned int WINDOW_H = 800;

	std::string title = "Softbody Simulation";

	GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Softbody Simulation", NULL, NULL);//create window

	//error checking
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	//set focus to window
	glfwMakeContextCurrent(window);

	//load GLAD to initialize OpenGL
	gladLoadGL();

	//set viewport to window
	glViewport(0, 0, WINDOW_W, WINDOW_H);

	//create geometry
	std::vector<float> verticies = {0};

	Shader def("default.vert", "default.frag");

	VAO vao;
	vao.Bind();

	VBO vbo(&verticies[0], sizeof(verticies));

	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

	vbo.Unbind();
	vao.Unbind();

	GLuint uniID = glGetUniformLocation(def.ID, "scale");

	int widthImg, heightImg, numColCh;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytes = stbi_load("cat.jpg", &widthImg, &heightImg, &numColCh, 0);

	GLuint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthImg, heightImg, 0, GL_RGB, GL_UNSIGNED_BYTE, bytes);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(bytes);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint tex0Uni = glGetUniformLocation(def.ID, "tex0");
	def.Activate();
	glUniform1i(tex0Uni, 0);

	Node mouse;
	bool clickflag=0;

	std::vector<Node> nodes;
	nodes.resize(0xFFFF);

	const int radius = 20;
	const int point_count = 10;

	//framerate limiter and display
	double previousTime = glfwGetTime();
	int frameCount = 0;
	std::string frameratedisplay;
	int frameratelimit = 60;

	bool frame1 = 0;

	bool addnodeflag = 0;
	bool connectorflag = 0;
	float distx;
	float disty;
	float distsq;
	const float collisionchecksqr = 4 * radius * radius;

	std::vector<float> cursor;

	Node* connector = new Node;

	int count = 0;

	bool frame1flag = 0;
	bool playstate = 0;

	//window loop
	while (!glfwWindowShouldClose(window)) {
		double currentTime = glfwGetTime();

		while (frameCount != 0 && frameCount / (currentTime - previousTime) > frameratelimit) {
			currentTime = glfwGetTime();
		}
		frameCount++;

		//input handler
		glfwPollEvents();

		glfwGetCursorPos(window, &mouse.x, &mouse.y);
		mouse.x = mouse.x * 2 - WINDOW_W;
		mouse.y = WINDOW_H - 2 * mouse.y;
		addnodeflag = 1;
		connectorflag = 0;

		for (int i = 0; i < count; i++) {
			if (connector == &(nodes[i])) {
				addnodeflag = 0;
				connectorflag = 1;
			}
		}

		verticies = std::vector<float>();
		cursor = circle(WINDOW_W, WINDOW_H, mouse.x, mouse.y, radius, point_count, addnodeflag);

		for (int i = 0; i < count; i++) {
			distx = mouse.x - nodes[i].x;
			disty = mouse.y - nodes[i].y;
			distsq = distx * distx + disty * disty;
			if (distsq < collisionchecksqr) addnodeflag = 0;

			std::vector<float> c = circle(WINDOW_W, WINDOW_H, nodes[i].x, nodes[i].y, radius, point_count, distsq >= collisionchecksqr);
			verticies.insert(verticies.end(), c.begin(), c.end());
			for (Node* n : nodes[i].connections) {
				verticies.push_back(nodes[i].x / WINDOW_W);
				verticies.push_back(nodes[i].y / WINDOW_H);
				verticies.push_back(1);
				verticies.push_back((n->x + 3) / WINDOW_W);
				verticies.push_back((n->y + 3) / WINDOW_H);
				verticies.push_back(1);
				verticies.push_back(n->x / WINDOW_W);
				verticies.push_back(n->y / WINDOW_H);
				verticies.push_back(1);
			}
			if (playstate) {
				nodes[i].update();
				if (abs(nodes[i].x) > WINDOW_W) {
					nodes[i].x = WINDOW_W * abs(nodes[i].x) / nodes[i].x;
					nodes[i].velx *= -0.9;
				}
				if (abs(nodes[i].y) > WINDOW_H) {
					nodes[i].y = WINDOW_H * abs(nodes[i].y) / nodes[i].y;
					nodes[i].vely *= -0.9;
				}
			}
		}
		for (int i = 0; i < count; i++) {
			if (playstate)nodes[i].apply_forces();
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			if (!clickflag) {
				if (addnodeflag) {
					nodes[count] = mouse;
					count++;
				}
				else {
					for (int i = 0; i < count; i++) {
						distx = mouse.x - nodes[i].x;
						disty = mouse.y - nodes[i].y;
						distsq = distx * distx + disty * disty;
						if (distsq < collisionchecksqr) {
							if(!connectorflag) connector = &(nodes[i]);
							else {
								distx = connector->x - nodes[i].x;
								disty = connector->y - nodes[i].y;
								distsq = distx * distx + disty * disty;
								join_nodes(connector, &(nodes[i]), sqrt(distsq));
								connector = nullptr;
								connectorflag = 0;
							}

						}
					}
				}
			}
			clickflag = 1;
		}
		else clickflag = 0;

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			if (!frame1flag) playstate = !playstate;
			frame1flag = 1;
		}
		else frame1flag = 0;

		if (addnodeflag) verticies.insert(verticies.end(), cursor.begin(), cursor.end());
		if (connectorflag) {
			verticies.push_back(mouse.x / WINDOW_W);
			verticies.push_back(mouse.y / WINDOW_H);
			verticies.push_back(0);
			verticies.push_back(((*connector).x+2) / WINDOW_W);
			verticies.push_back(((*connector).y+2) / WINDOW_H);
			verticies.push_back(0);
			verticies.push_back((*connector).x / WINDOW_W);
			verticies.push_back((*connector).y / WINDOW_H);
			verticies.push_back(0);

		}

		//background color
		glClearColor(0, 0, 0.1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		vbo.getData(&verticies[0],verticies.size()*sizeof(GLfloat));

		//render geometry from VAO
		def.Activate();
		glUniform1f(uniID, 1.0f);
		glBindTexture(GL_TEXTURE_2D, texture);
		vao.Bind();
		glDrawArrays(GL_TRIANGLES, 0, verticies.size()/3);

		//refresh screen
		glfwSwapBuffers(window);

		//framerate display
		std::ostringstream fpsstream;
		if (currentTime - previousTime >= 1.0)
		{
			fpsstream << frameCount;

			frameCount = 0;
			previousTime = currentTime;
			frameratedisplay = title + " | FPS: " + fpsstream.str();
			glfwSetWindowTitle(window, frameratedisplay.c_str());
		}
	}

	//destroy all objects
	glfwDestroyWindow(window);
	def.Delete();
	vao.Delete();
	vbo.Delete();

	//terminate glfw instance
	glfwTerminate();

	return 0;
}

std::vector<float> circle(float w, float h, float x, float y, float radius, int point_count, bool state) {
	std::vector<float> result;
	
	for (int i = 0; i < point_count; i++) {

		result.push_back((x + radius * cos(2 * 3.141592653589793 * i / (float)point_count)) / w);
		result.push_back((y + radius * sin(2 * 3.141592653589793 * i / (float)point_count)) / h);
		result.push_back(state);
		result.push_back((x + radius * cos(2 * 3.141592653589793 * (i + 1) / (float)point_count)) / w);
		result.push_back((y + radius * sin(2 * 3.141592653589793 * (i + 1) / (float)point_count)) / h);
		result.push_back(state);
		result.push_back(x / w);
		result.push_back(y / h);
		result.push_back(state);

	}
	return result;
}

void join_nodes(Node* a, Node* b, float rest_length) {
	if (std::find((*a).connections.begin(), (*a).connections.end(), b) == (*a).connections.end() && a != b) {
		a->connections.push_back(b);
		b->connections.push_back(a);
		a->rest_lengths.push_back(rest_length);
		std::cout << a->rest_lengths[(*a).rest_lengths.size() - 1] << std::endl;
		b->rest_lengths.push_back(rest_length);
	}
}