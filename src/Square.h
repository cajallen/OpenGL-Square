#ifndef SQUARE_H_
#define SQUARE_H_

#include "glad/glad.h"  //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_opengl.h>
#else
	#include <SDL.h>
	#include <SDL_opengl.h>
#endif
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include "pga.h"
#include "geom_lib_2d.h"
using namespace std;

enum class SystemCursor {
	Default,
	Drag,
	Rotate,
	Scale
};

enum class Manip {
	None,
	Drag,
	Rotate,
	Scale
};

void initialize();
void update();
void render();
void mouseMoved(float mx, float my);
void mousewheel(float amount);
void keyPressed_r();
void keyPressed_f();
void keyPressed_a();
void setCursor(SystemCursor type);
unsigned char* loadImage(int& img_w, int& img_h, string texture_name);

class Image {
 public:
	int id;
	// settings
	float rotate_dist = 20.0f;
	float scale_dist = 8.0f;
	Point2D init_pos = Point2D(400, 400);
	float init_scale = 0.5f;
	float init_angle = 0.0f;
	float init_brightness = 0.5f;
	Point2D init_p1 = Point2D(-100.0f, 100.0f);	// bottom left
	Point2D init_p2 = Point2D(100.0f, 100.0f);	// bottom right
	Point2D init_p3 = Point2D(100.0f, -100.0f);	// top right
	Point2D init_p4 = Point2D(-100.0f, -100.0f);		// top left

	// runtime
	Point2D rect_pos = init_pos;
	float rect_scale = init_scale;
	float rect_angle = init_angle;
	float rect_brightness = init_brightness;
	float rect_dir;
	unsigned char* img_data;

	Point2D p1, p2, p3, p4;
	Line2D l1, l2, l3, l4;

	// openGL
	GLuint vao;
	GLuint vbo;
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint shaderProgram;

	// These are not the inital values, they are only non-empty to show the signs.
	// Use init_p# to initialize values.
	float vertices[32] = {
	//    X     Y     R     G     B     U     V
		 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top right
		 1.0f,-1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
		-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // top left
		-1.0f,-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f   // bottom left
	};

	Image(string file_name, Point2D pos);
	~Image();

	bool mousePressed_left(float x, float y);
	void mouseDragged(float x, float y);
	void updateRect();
	void updateVertices();
	void reset();
	void moveDistance(float distance);

	Manip posToManip(Point2D pos);
	bool inBounds(Point2D pos);

	void update();
	void render();
};


#endif
