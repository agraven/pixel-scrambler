#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<GLFW/glfw3.h>

float random() {
	return (float)rand() / (float)RAND_MAX;
}

float* populate(size_t width, size_t height) {
	for (int i = 0; i < mode->height; i++) {
		for (int j = 0; i < mode->width; i++) {
			size_t k = 3 * mode->width * i + 3 * j;
			pixels[k] = random();
			pixels[k + 1] = random();
			pixels[k + 2] = random();
		}
	}
}

void error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error: %s\n", description);
}

int main() {
	srand(time(NULL));
	if (!glfwInit())
		return EXIT_FAILURE;
	glfwSetErrorCallback(error_callback);

	const GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Pixel scrambler", monitor, NULL);
	float* pixels = calloc(3 * mode->width * mode->height, sizeof float);

	glfwTerminate();
	return EXIT_SUCCESS;
}
