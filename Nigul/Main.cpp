#include "source/GUI.h"
#include "source/scene.h"
#include "source/skybox.h"
#include "source/quad.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

const unsigned int width = 1200;
const unsigned int height = 1200;

int main()
{
	std::cout << "App started" << std::endl;
	// Initialize GLFW
	glfwSetErrorCallback([](int error, const char* description) {
		std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
		});

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object of 800 by 800 pixels, naming it "YoutubeOpenGL"
	GLFWwindow* window = glfwCreateWindow(width, height, "Nigul", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to careate GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);
	gladLoadGL();

	auto start = std::chrono::high_resolution_clock::now();
	Scene loadedScene;
	loadedScene.loadScene();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;

	std::cout << "Scene loaded in " << elapsed.count() << " ms\n";

	postProcessing postpo(width, height);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	GUI menu(&loadedScene, &postpo);
	menu.init(window);

	double lastTime = glfwGetTime();
	int nbFrames = 0;

	// Main while loop
	while (!glfwWindowShouldClose(window)){
		// Measure fps
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0)
		{
			glfwSetWindowTitle(window, ("Nigul | " + std::to_string(nbFrames) + " fps").c_str());
			nbFrames = 0;
			lastTime += 1.0;
		}

		loadedScene.passSceneProperties();

		postpo.fbo->Bind();
	
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		loadedScene.drawScene();

		postpo.fbo->Unbind();

		postpo.render();

		loadedScene.camera->Inputs(window);
		loadedScene.camera->updateMatrix();

		menu.createFrame();
		menu.logic();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	menu.destroy();
	loadedScene.defaultShader->Delete();
	loadedScene.skyboxShader->Delete();
	postpo.shader->Delete();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}