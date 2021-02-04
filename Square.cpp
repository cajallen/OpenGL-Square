#include "Square.h"

typedef void (*keyFunc)();

string textureName = "goldy.ppm";

map<char, keyFunc> keyFunctions{
	{SDLK_r, &r_keyPressed},
	{SDLK_f, &f_keyPressed}
};

// Screen size
bool fullscreen = false;
int screen_width = 800;
int screen_height = 800;

Point2D init_pos = Point2D(0, 0);
float init_scale = 0.3f;
float init_angle = 0.0f;

Point2D rect_pos = init_pos;
float rect_scale = init_scale;
float rect_angle = init_angle;

// TODO: Can we do these per pixel? I think it's just screen_width, but maybe they should be a function? not sure
float rotate_dist = 0.1f;
float scale_dist = 0.04f;

float vertices[] = {
	// The function updateVertices() changes these values to match p1,p2,p3,p4
	//  X     Y     R     G     B     U    V
	1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top right
	1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
	-1.0f, 1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // top left
	-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom left
};

Point2D origin = Point2D(0, 0);

// Watch winding...
Point2D init_p1 = Point2D(vertices[21], vertices[22]);
Point2D init_p2 = Point2D(vertices[7], vertices[8]);
Point2D init_p3 = Point2D(vertices[0], vertices[1]);
Point2D init_p4 = Point2D(vertices[14], vertices[15]);

Point2D p1, p2, p3, p4;

// Not necessary, helpful.
Line2D l1, l2, l3, l4;

// Mouse state
bool mouse_dragging = false;
Point2D clicked_pos;
Point2D clicked_mouse;
float clicked_angle, clicked_size;

// Input state
bool do_translate = false;
bool do_rotate = false;
bool do_scale = false;

// OpenGL
SDL_Window* window;

// Shader sources
const GLchar* vertexSource =
	"#version 150 core\n"
	"in vec2 position;"
	"in vec3 inColor;"
	"in vec2 inTexcoord;"
	"out vec3 Color;"
	"out vec2 texcoord;"
	"void main() {"
	"   Color = inColor;"
	"   gl_Position = vec4(position, 0.0, 1.0);"
	"   texcoord = inTexcoord;"
	"}";

const GLchar* fragmentSource =
	"#version 150 core\n"
	"uniform sampler2D tex0;"
	"in vec2 texcoord;"
	"out vec3 outColor;"
	"void main() {"
	"   outColor = texture(tex0, texcoord).rgb;"
	"}";



void initialize() {
	updateRect();
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
	}

	int mx, my;
	if (SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		if (mouse_dragging == false) {
			mouseClicked(2 * mx / (float)screen_width - 1, 1 - 2 * my / (float)screen_height);
			mouse_dragging = true;
		}
	} else {
		mouse_dragging = false;
	}
	mouseMoved(2 * mx / (float)screen_width - 1, 1 - 2 * my / (float)screen_height);
}


void render() {

}


void mouseMoved(float m_x, float m_y) {
	Point2D cur_mouse = Point2D(m_x, m_y);
	if (mouse_dragging) {
		if (do_translate) {
			Dir2D disp = cur_mouse - clicked_mouse;
			rect_pos = clicked_pos + disp;
		}

		if (do_scale) {
			rect_scale = clicked_size;
			// correct position
			Dir2D pos_disp = cur_mouse - rect_pos;
			// correct rotation
			Dir2D disp;
			disp.x = pos_disp.x*cos(rect_angle)-pos_disp.y*sin(rect_angle);
			disp.y = pos_disp.x*sin(rect_angle)+pos_disp.y*cos(rect_angle);

			rect_scale = fmax(fabs(disp.x), fabs(disp.y));
		}

		if (do_rotate) {
			float diff = angle(join(rect_pos, clicked_mouse), join(rect_pos, cur_mouse));
			rect_angle = clicked_angle + diff;
		}

		if (do_translate || do_scale || do_rotate) {
			updateRect();
		}
	} else {
		if (pointInConvexPolygon(cur_mouse, vector<Line2D>{l1, l2, l3, l4})) {
			if (pointCornerDist(cur_mouse, vector<Point2D>{p1, p2, p3, p4}) < rotate_dist) {
				setCursor(SystemCursor::Scale);
			} else if (pointEdgeDist(cur_mouse, vector<Point2D>{p1, p2, p3, p4}) < scale_dist) {
				setCursor(SystemCursor::Rotate);
			} else {
				setCursor(SystemCursor::Drag);
			}
		} else {
			setCursor(SystemCursor::Default);
		}
	}
}


void mousePressed_left(float m_x, float m_y) {
	printf("Clicked at %f, %f\n", m_x, m_y);

	clicked_mouse = Point2D(m_x, m_y);
	clicked_angle = rect_angle;
	clicked_pos = rect_pos;

	do_translate = false;
	do_rotate = false;
	do_scale = false;

	if (pointInConvexPolygon(clicked_mouse, vector<Line2D>{l1, l2, l3, l4})) {
		if (pointCornerDist(clicked_mouse, vector<Point2D>{p1, p2, p3, p4}) < rotate_dist) {
			do_scale = true;
			setCursor(SystemCursor::Scale);
		} else if (pointEdgeDist(clicked_mouse, vector<Point2D>{p1, p2, p3, p4}) < scale_dist) {
			do_rotate = true;
			setCursor(SystemCursor::Rotate);
		} else {
			do_translate = true;
			setCursor(SystemCursor::Drag);
		}
	}
}


void r_keyPressed() {
	cout << "The 'r' key was pressed" << endl;

	rect_pos = Point2D(0.0f, 0.0f);
	rect_scale = init_scale;
	rect_angle = 0.0f;

	updateRect();
}

void keyPressed_f() {
	fullscreen = !fullscreen;
	SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
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
void updateRect() {
	Motor2D translate, rotate;

	Dir2D disp = rect_pos - origin;
	translate = Translator2D(disp);
	rotate = Rotator2D(rect_angle, rect_pos);

	Motor2D movement = rotate * translate;

	// Scale points
	p1 = init_p1.scale(rect_scale);
	p2 = init_p2.scale(rect_scale);
	p3 = init_p3.scale(rect_scale);
	p4 = init_p4.scale(rect_scale);

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


void updateVertices() {
	vertices[0] = p3.x;  // Top right x
	vertices[1] = p3.y;  // Top right y

	vertices[7] = p2.x;  // Bottom right x
	vertices[8] = p2.y;  // Bottom right y

	vertices[14] = p4.x;  // Top left x
	vertices[15] = p4.y;  // Top left y

	vertices[21] = p1.x;  // Bottom left x
	vertices[22] = p1.y;  // Bottom left y
}


// TODO: Read from ASCII (P3) PPM files
// Inputs are output variables for returning the image width and heigth
unsigned char* loadImage(int& img_w, int& img_h) {
	// Open the texture image file
	ifstream ppmFile;
	ppmFile.open(textureName.c_str());
	if (!ppmFile) {
		printf("ERROR: Texture file '%s' not found.\n", textureName.c_str());
		exit(1);
	}

	// Check that this is an ASCII PPM (first line is P3)
	string PPM_style;
	ppmFile >> PPM_style;  // Read the first line of the header
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

	// TODO: This loop puts in fake data, replace with the actual pixels read from
	// the file
	//      ... see project description for a hint on how to do this
	int red, green, blue;
	int pixelNum = 0;
	while (ppmFile >> red >> green >> blue) {
		img_data[pixelNum * 4 + 0] = red;
		img_data[pixelNum * 4 + 1] = green;
		img_data[pixelNum * 4 + 2] = blue;
		img_data[pixelNum * 4 + 3] = 255;
		pixelNum += 1;
	}

	return img_data;
}

/////////////////////////////
/// ... below is OpenGL specifc code,
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

	// aspect ratio (needs to be updated if the window is resized)
	float aspect = screen_width / (float)screen_height;

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

	//// Allocate Texture 0 (Created in Load Image) ///////
	GLuint tex0;
	glGenTextures(1, &tex0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex0);

	// What to do outside 0-1 range
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int img_w, img_h;
	unsigned char* img_data = loadImage(img_w, img_h);
	printf("Loaded Image of size (%d,%d)\n", img_w, img_h);
	// memset(img_data,0,4*img_w*img_h); //Load all zeros
	// Load the texture into memory
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_w, img_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	//// End Allocate Texture ///////

	// Build a Vertex Array Object. This stores the VBO and attribute mappings in one object
	GLuint vao;

	// Create a VAO
	glGenVertexArrays(1, &vao);
	
	// Bind the above created VAO to the current context
	glBindVertexArray(vao);

	// Allocate memory on the graphics card to store geometry (vertex buffer object)
	GLuint vbo;
	glGenBuffers(1, &vbo);  // Create 1 buffer called vbo

	// Set the vbo as the active array buffer (Only one buffer can be active at a time)
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// upload vertices to vbo
	// GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW =
	// geometry changes infrequently GL_STREAM_DRAW = geom. changes frequently.
	// This effects which types of GPU memory is used
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	// Load the vertex Shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
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

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
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
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");  // set output
	glLinkProgram(shaderProgram);                          // run the linker

	glUseProgram(shaderProgram);  // Set the active shader (only one can be used at a time)

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

	initialize();

	// Event Loop (Loop forever processing each event as fast as possible)
	SDL_Event windowEvent;
	bool done = false;
	while (!done) {
		update();
		render();

		// upload vertices to vbo
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

		// Clear the screen to grey
		glClearColor(0.6f, 0.6f, 0.6f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw the two triangles (4 vertices) making up the square
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  

		// Double buffering
		SDL_GL_SwapWindow(window);
	}

	delete[] img_data;
	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	glDeleteBuffers(1, &vbo);

	glDeleteVertexArrays(1, &vao);

	// Clean Up
	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}
