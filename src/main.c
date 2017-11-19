#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<GL/glew.h>
#include<SDL2/SDL.h>

const char* vertex_source = "#version 150 core\n"
"in vec2 position;"
"in vec2 texcoord;"
"out vec2 Texcoord;"
"void main() {"
"	Texcoord = texcoord;"
"	gl_Position = vec4(position, 0.0, 1.0);"
"}";
const char* fragment_source = "#version 150 core\n"
"in vec2 Texcoord;"
"out vec4 outColor;"
"uniform sampler2D tex;"
"void main() {"
"	outColor = texture(tex, Texcoord) * vec4(1.0, 1.0, 1.0, 1.0);"
"}";
float vertices[] = {
	-1.0f, 1.0f, 0.0f, 0.0f,
	1.0f,  1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 1.0f,
	-1.0f,-1.0f, 0.0f, 1.0f
};
GLuint elements[] = {
	0, 1, 2,
	2, 3, 0
};

float randomf() {
	return (float)rand() / (float)RAND_MAX;
}

void populate(float* buffer, size_t width, size_t height) {
	srand(time(NULL));
	for (int i = 0; i < 3 * width * height; i++) {
		buffer[i] = randomf();
	}
}

const int tex_width = 10, tex_height = 10;

int main() {
	float* pixels = (float*)calloc(3 * tex_width * tex_height, sizeof(float));
	if (errno != 0) {
		perror("allocation error");
		exit(EXIT_FAILURE);
	}
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
	}
	SDL_Window *window = SDL_CreateWindow("Pixel scrambler", 0, 0, 640, 480, SDL_WINDOW_OPENGL);
	SDL_Event event;
	SDL_GLContext context = SDL_GL_CreateContext(window);

	glewExperimental = GL_TRUE;
	glewInit();
	
	// BEGIN BOILERPLATE
	GLuint ebo, vbo, vao, vertexShader, fragmentShader, shaderProgram, texture;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertex_source, NULL);
	glCompileShader(vertexShader);
	GLint status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	printf("%s\n", status ? "succes" : "failue");

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragment_source, NULL);
	glCompileShader(fragmentShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	GLint posAttrib, texAttrib;
	posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)) );

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	populate(pixels, tex_width, tex_height);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_FLOAT, pixels);
	glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// END BOILERPLATE


	while (1) {
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) break;

		populate(pixels, tex_width, tex_height);
		glClearColor(1,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_FLOAT, pixels);
		glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		SDL_GL_SwapWindow(window);
	}

	free(pixels);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return EXIT_SUCCESS;
}
