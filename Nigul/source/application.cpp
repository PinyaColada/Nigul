#include "application.h"

Application::Application(int width, int height, const std::string& title): width(width), height(height), title(title)
{}

void Application::init() {
	std::cout << "App started" << std::endl;
	// Initialize GLFW
	glfwSetErrorCallback([](int error, const char* description) {
		std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
		});

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Nigul", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to careate GLFW window" << std::endl;
		glfwTerminate();
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);
	gladLoadGL();

	auto start = std::chrono::high_resolution_clock::now();

	sceneManager = std::make_unique<Scene>();
	sceneManager->loadScene();

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;

	std::cout << "Scene loaded in " << elapsed.count() << " ms\n";

	postpo = std::make_unique<postProcessing>(width, height);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	menu = std::make_unique<GUI>(sceneManager.get(), postpo.get());
	menu->init(window);

	double lastTime = glfwGetTime();
	int nbFrames = 0;
}

void Application::loop(){
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	// Main while loop
	while (!glfwWindowShouldClose(window)) {
		// Measure fps
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0)
		{
			glfwSetWindowTitle(window, (title + " | " + std::to_string(nbFrames) + " fps").c_str());
			nbFrames = 0;
			lastTime += 1.0;
		}

		sceneManager->processInput(window);

		sceneManager->passSceneProperties();

		postpo->fbo->Bind();

		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		sceneManager->drawScene();

		postpo->fbo->Unbind();

		postpo->render();

		menu->createFrame();
		menu->logic();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Application::destroy() {
	menu->destroy();
	sceneManager->defaultShader->Delete();
	sceneManager->skyboxShader->Delete();
	postpo->shader->Delete();
	glfwDestroyWindow(window);
	glfwTerminate();
}
