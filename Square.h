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
#include "pga.h"
#include "geom_lib_2d.h"
using namespace std;

enum class SystemCursor {
	Default,
	Drag,
	Rotate,
	Scale
};

void initialize();
void mouseClicked(float mx, float my);
void mouseMoved(float mx, float my);
void r_keyPressed();
void f_keyPressed();
void setCursor(SystemCursor type);
void updateRect();
void updateVertices();
unsigned char* loadImage(int& img_w, int& img_h);


#endif
