#include "Square.h"

typedef void (*keyFunc)();

map<char, keyFunc> keyFunctions{
	{SDLK_r, &keyPressed_r},
	{SDLK_f, &keyPressed_f},
	{SDLK_a, &keyPressed_a},
};

random_device rd;
default_random_engine eng(rd());
uniform_real_distribution<> distr(0.0f, 6.283185f);

// Images
vector<Image*> images{};

// Screen size
bool fullscreen = false;
int screen_width = 1200;
int screen_height = 800;
float aspect = screen_width / (float)screen_height;

Point2D origin = Point2D(0, 0);

// Mouse state
Image* clicked_object;
Point2D clicked_pos;
Point2D clicked_mouse;
float clicked_angle;

// Input state
Manip selected_manip;
bool mouse_dragging;
bool animating = false;

// OpenGL
SDL_Window* window;
SDL_Event windowEvent;
GLuint tex[16];
bool done;


void initialize() {
	Image* image = new Image("images/goldy.ppm", Point2D(300, 600));
	Image* image2 = new Image("images/brick.ppm", Point2D(450, 300));
	Image* image3 = new Image("images/test.ppm", Point2D(600, 600));
	Image* image4 = new Image("images/smiles.ppm", Point2D(800, 500));
}


void update() {
	// Process input events (e.g., mouse & keyboard)
	// List of keycodes: https://wiki.libsdl.org/SDL_Keycode
	while (SDL_PollEvent(&windowEvent)) {
		if (windowEvent.type == SDL_QUIT || (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)) {
			done = true;
		}
		if (windowEvent.type == SDL_KEYUP) {
			char key = windowEvent.key.keysym.sym;
			if (keyFunctions.count(key) > 0) {
				keyFunctions[key]();
			}
		}
		if (!animating) {
			if ((windowEvent.type == SDL_MOUSEWHEEL) && (windowEvent.wheel.y != 0)) {
				mousewheel(windowEvent.wheel.y);
			}
		}
	}

	for (Image* image : images) {
		image->update();
	}

	if (!animating) {
		int mx, my;
		int mouse_state = SDL_GetMouseState(&mx, &my);
		if (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			if (mouse_dragging == false) {
				for (Image* image : images) {
					if (image->mousePressed_left(mx, my)) {
						break;
					}
				}
				mouse_dragging = true;
			}
		} else {
			mouse_dragging = false;
		}
		mouseMoved(mx, my);
	}
}


void Image::update() {
	if (animating) {
		moveDistance(2.0f);
		updateRect();
	}
}

void Image::moveDistance(float distance) {
	Point2D move(distance * cos(rect_dir), distance * sin(rect_dir));

	float right = rect_pos.x + fabs(init_p1.x * rect_scale);
	float new_right = right + move.x;
	if (new_right > screen_width) {
		float dist_to_move = (screen_width - right) / move.x;
		rect_pos.x += dist_to_move * move.x;
		rect_pos.y += dist_to_move * move.y;
		rect_dir = atan2(move.y, -move.x);
		moveDistance(distance - dist_to_move);
		return;
	}
	float left = rect_pos.x - fabs(init_p1.x * rect_scale);
	float new_left = left + move.x;
	if (new_left < 0) {
		float dist_to_move = left / move.x;
		rect_pos.x += dist_to_move * move.x;
		rect_pos.y += dist_to_move * move.y;
		rect_dir = atan2(move.y, -move.x);
		moveDistance(distance - dist_to_move);
		return;
	}
	float top = rect_pos.y - fabs(init_p1.y * rect_scale);
	float new_top = top + move.y;
	if (new_top < 0) {
		float dist_to_move = top / move.y;
		rect_pos.x += dist_to_move * move.x;
		rect_pos.y += dist_to_move * move.y;
		rect_dir = atan2(-move.y, move.x);
		moveDistance(distance - dist_to_move);
		return;
	}
	float bot = rect_pos.y + fabs(init_p1.y * rect_scale);
	float new_bot = bot + move.y;
	if (new_bot > screen_height) {
		float dist_to_move = (screen_height - bot) / move.y;
		rect_pos.x += dist_to_move * move.x;
		rect_pos.y += dist_to_move * move.y;
		rect_dir = atan2(-move.y, move.x);
		moveDistance(distance - dist_to_move);
		return;
	}

	rect_pos.x += move.x;
	rect_pos.y += move.y;
}


void render() {
	// Clear the screen to grey
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	reverse(images.begin(), images.end());
	for (Image* image : images) {
		image->render();
	}
	reverse(images.begin(), images.end());

	// Double buffering
	SDL_GL_SwapWindow(window);
}


void Image::render() {
	glUseProgram(shaderProgram);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void mouseMoved(float m_x, float m_y) {
	if (animating) { return; }
	if (mouse_dragging && clicked_object) {
		clicked_object->mouseDragged(m_x, m_y);
	} else {
		for (Image* image : images) {
			Manip hovered_manip = image->posToManip(Point2D{m_x, m_y});
			if (hovered_manip != Manip::None) {
				// shortcut code, fleshed out would be a dictionary
				setCursor((SystemCursor) hovered_manip);
				return;
			}
		}
		setCursor(SystemCursor::Default);
	}
}


void mousewheel(float amount) {
	int mx, my;
	SDL_GetMouseState(&mx, &my);
	for (Image* image : images) {
		if (image->inBounds(Point2D{(float)mx, (float)my})) {
			image->rect_brightness = image->rect_brightness + amount * 0.05f;
			image->rect_brightness = min(image->rect_brightness, 1.0f);
			image->rect_brightness = max(image->rect_brightness, 0.0f);
			return;
		}
	}
}


void Image::mouseDragged(float m_x, float m_y) {
	if (animating) { return; }
	Point2D cur_mouse = Point2D(m_x, m_y);
	Dir2D disp1, disp2;
	switch (selected_manip) {
		case Manip::Drag: {
			disp1 = cur_mouse - clicked_mouse;
			rect_pos = clicked_pos + disp1;
			break;
		} case Manip::Rotate: {
			float diff = angle(join(rect_pos, clicked_mouse), join(rect_pos, cur_mouse));
			rect_angle = clicked_angle + diff;
			break;
		} case Manip::Scale: {
			disp1 = cur_mouse - rect_pos;
			disp2 = transform(disp1, Rotator2D(-rect_angle, Point2D(0,0)));
			rect_scale = fmax(fabs(disp2.x/init_p2.x), fabs(disp2.y/init_p2.y));
			break;
	 	} default: {
			break;
		}
	}
	updateRect();
}


Manip Image::posToManip(Point2D pos) {
	if (inBounds(pos)) {
		if (pointCornerDist(pos, vector<Point2D>{p1, p2, p3, p4}) < rotate_dist) {
			return Manip::Scale;
		} else if (pointEdgeDist(pos, vector<Point2D>{p1, p2, p3, p4}) < scale_dist) {
			return Manip::Rotate;
		} else {
			return Manip::Drag;
		}
	} else {
		return Manip::None;
	}
}

bool Image::inBounds(Point2D pos) {
	return pointInConvexPolygon(pos, vector<Line2D>{l1, l2, l3, l4});
}


bool Image::mousePressed_left(float x, float y) {
	Point2D pos = Point2D{x, y};
	selected_manip = posToManip(pos);
	if (selected_manip != Manip::None) {
		clicked_object = this;
		clicked_mouse = pos;
		clicked_angle = rect_angle;
		clicked_pos = rect_pos;

		// a bit of a hack, SystemCursor is not guaranteed == Manip, but it is for now.
		setCursor((SystemCursor)selected_manip);

		return true;
	}
	return false;
}


void Image::reset() {
	rect_pos = init_pos;
	rect_scale = init_scale;
	rect_angle = init_angle;
	rect_brightness = init_brightness;
	updateRect();
}


void keyPressed_r() {
	for (Image* image : images) {
		image->reset();
	}
}


void keyPressed_f() {
	fullscreen = !fullscreen;
	SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
}

void keyPressed_a() {
	if (!animating) {
		for (Image* image : images) {
			image->rect_pos = image->init_pos;
			image->rect_angle = image->init_angle;
			image->rect_dir = distr(eng);
		}
		mouse_dragging = false;
		selected_manip = Manip::None;
		setCursor(SystemCursor::Default);
		animating = true;
	} else {
		for (Image* image : images) {
			image->rect_pos = image->init_pos;
			image->rect_angle = image->init_angle;
			image->updateRect();
		}
		animating = false;
	}

}


void setCursor(SystemCursor type) {
	SDL_Cursor* cursor;
	switch(type) {
		case SystemCursor::Drag:
			cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
			break;
		case SystemCursor::Rotate:
			cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
			break;
		case SystemCursor::Scale:
			cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
			break;
		default:
			cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
			break;
	}
	SDL_SetCursor(cursor);
}


// Assuming the angle (rect_angle), position (rect_pos), and scale (rect_scale) of the rectangle 
// have all been set above, the following code should rotate, shift and scale the shape correctly.
// It's still good to read through and make sure you understand how this works!
void Image::updateRect() {
	Motor2D translate, rotate;

	Dir2D disp = rect_pos - origin;
	translate = Translator2D(disp);
	rotate = Rotator2D(rect_angle, rect_pos);

	Motor2D movement = rotate * translate;

	// Scale points
	p1 = init_p1.scale(rect_scale, rect_scale);
	p2 = init_p2.scale(rect_scale, rect_scale);
	p3 = init_p3.scale(rect_scale, rect_scale);
	p4 = init_p4.scale(rect_scale, rect_scale);

	// Use Motor to translate and rotate points
	p1 = transform(p1, movement);
	p2 = transform(p2, movement);
	p3 = transform(p3, movement);
	p4 = transform(p4, movement);

	// Update lines based on new points
	l1 = vee(p1, p2).normalized();
	l2 = vee(p2, p3).normalized();
	l3 = vee(p3, p4).normalized();
	l4 = vee(p4, p1).normalized();


	updateVertices();
}


void Image::updateVertices() {
	vertices[0] = 2 * p3.x / (float)screen_width - 1;	// Top right x
	vertices[1] = 1 - 2 * p3.y / (float)screen_height;	// Top right y

	vertices[7] = 2 * p2.x / (float)screen_width - 1;	// Bottom right x
	vertices[8] = 1 - 2 * p2.y / (float)screen_height;	// Bottom right y

	vertices[14] = 2 * p4.x / (float)screen_width - 1;	// Top left x
	vertices[15] = 1 - 2 * p4.y / (float)screen_height; // Top left y

	vertices[21] = 2 * p1.x / (float)screen_width - 1;	// Bottom left x
	vertices[22] = 1 - 2 * p1.y / (float)screen_height; // Bottom left y
}


unsigned char* loadImage(int& img_w, int& img_h, string texture_name) {
	// Open the texture image file
	ifstream ppmFile;
	ppmFile.open(texture_name.c_str());
	if (!ppmFile) {
		printf("ERROR: Texture file '%s' not found.\n", texture_name.c_str());
		exit(1);
	}

	// Check that this is an ASCII PPM (first line is P3)
	string PPM_style;
	ppmFile >> PPM_style;
	if (PPM_style != "P3") {
		printf("ERROR: PPM Type number is %s. Not an ASCII (P3) PPM file!\n", PPM_style.c_str());
		exit(1);
	}

	// Read in the texture width and height
	ppmFile >> img_w >> img_h;
	unsigned char* img_data = new unsigned char[4 * img_w * img_h];

	// Check that the 3rd line is 255 (ie., this is an 8 bit/pixel PPM)
	int maximum;
	ppmFile >> maximum;
	if (maximum != 255) {
		printf("ERROR: Maximum size is (%d) not 255.\n", maximum);
		exit(1);
	}

	int red, green, blue;
	int pixel_num = 0;
	while (ppmFile >> red >> green >> blue) {
		// flips pixels vertically
		int pp_x = pixel_num % img_w;
		int pp_y = (pixel_num / img_w);
		int pixel_pos = (img_h - pp_y - 1) * img_w + pp_x;
		
		img_data[pixel_pos * 4 + 0] = red;
		img_data[pixel_pos * 4 + 1] = green;
		img_data[pixel_pos * 4 + 2] = blue;
		img_data[pixel_pos * 4 + 3] = 255;
		pixel_num += 1;
	}

	return img_data;
}


Image::Image(string file_name, Point2D pos) {
	id = images.size();
	init_pos = pos;
	rect_pos = init_pos;

	int img_w, img_h;
	img_data = loadImage(img_w, img_h, file_name);

	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, tex[id]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_w, img_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Create a VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create 1 buffer called vbo
	glGenBuffers(1, &vbo); 
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// upload vertices to vbo
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	// Load the vertex Shader
	ifstream in1("shaders/default.vert");
	string contents1((istreambuf_iterator<char>(in1)), istreambuf_iterator<char>());
	const char* vertexSource = contents1.c_str();

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	// Let's double check the shader compiled
	GLint status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char buffer[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
		printf("Vertex Shader Compile Failed. Info:\n\n%s\n", buffer);
	}

	ifstream in2("shaders/default.frag");
	string contents2((istreambuf_iterator<char>(in2)), istreambuf_iterator<char>());
	const char* fragmentSource = contents2.c_str();

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// Double check the shader compiled
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char buffer[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
		printf("Fragment Shader Compile Failed. Info:\n\n%s\n", buffer);
	}

	// Join the vertex and fragment shaders together into one program
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");  // set output

	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	// Tell OpenGL how to set fragment shader input
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");

	// Attribute, vals/attrib., type, normalized?, stride, offset
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);

	// Binds the VBO current GL_ARRAY_BUFFER
	glEnableVertexAttribArray(posAttrib);

	GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(colAttrib);

	GLint texAttrib = glGetAttribLocation(shaderProgram, "inTexcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));

	GLint text_loc = glGetUniformLocation(shaderProgram, "text");
	glUniform1i(text_loc, id);

	GLint brightness_loc = glGetUniformLocation(shaderProgram, "brightness");
	glUniform1f(brightness_loc, rect_brightness);


	updateRect();

	images.push_back(this);
}


Image::~Image() {
	delete[] img_data;

	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}


/////////////////////////////
/// ... below is OpenGL specific code,
///     we will cover it in detail around Week 9,
///     but you should try to poke around a bit right now.
///     I've annotated some parts with "TODO: TEST ..." check those out.
////////////////////////////
int main(int argc, char* argv[]) {
	// Initialize Graphics (for OpenGL)
	SDL_Init(SDL_INIT_VIDEO);

	// Ask SDL to get a fairly recent version of OpenGL (3.2 or greater)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	// Create a window (offsetx, offsety, width, height, flags)
	window = SDL_CreateWindow("Project 1... A Square", 100, 100, screen_width, screen_height, SDL_WINDOW_OPENGL);

	// The above window cannot be resized which makes some code slightly easier.
	// Below show how to make a full screen window or allow resizing
	// SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 0, 0,
	// screen_width, screen_height, SDL_WINDOW_FULLSCREEN|SDL_WINDOW_OPENGL);
	// SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100,
	// screen_width, screen_height, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
	// SDL_Window* window = SDL_CreateWindow("My OpenGL
	// Program",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,0,0,SDL_WINDOW_FULLSCREEN_DESKTOP|SDL_WINDOW_OPENGL);
	// //Boarderless window "fake" full screen

	// Create a context to draw in
	SDL_GLContext context = SDL_GL_CreateContext(window);

	// OpenGL functions using glad library
	if (gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		printf("OpenGL loaded\n");
		printf("Vendor:   %s\n", glGetString(GL_VENDOR));
		printf("Renderer: %s\n", glGetString(GL_RENDERER));
		printf("Version:  %s\n", glGetString(GL_VERSION));
	} else {
		printf("ERROR: Failed to initialize OpenGL context.\n");
		return -1;
	}

	//// Allocate Textures (Created in Load Image) ///////
	glGenTextures(16, tex);
	//// End Allocate Texture ///////

	initialize();

	// Event Loop
	done = false;
	while (!done) {
		update();
		render();
	}

	for (Image* image : images) {
		delete image;
	}

	// Clean Up
	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}
