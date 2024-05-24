#include "application.h"

Application::Application(int width, int height, const std::string& title): width(width), height(height), title(title){}

void Application::init() {
	glfwSetErrorCallback([](int error, const char* description) {
		std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
	});

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Nigul", NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to careate GLFW window" << std::endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);
	gladLoadGL();

	sceneManager = std::make_unique<SceneManager>();
	sceneManager->loadScene();

	renderer = std::make_unique<Renderer>(width, height);

	menu = std::make_unique<GUI>();
	menu->init(window);
}

void Application::loop(){
	while (!glfwWindowShouldClose(window)) {
		Model* model = sceneManager->getMainModel();
		Skybox* skybox = sceneManager->getMainSkybox();
		SceneManager* sm = sceneManager.get();
		Renderer* r = renderer.get();

		renderer->render(model, skybox);
		menu->createFrame(sm, r);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

Application::~Application() {
	menu.reset(); // Lets clean first ImGUI
	glfwDestroyWindow(window);
	glfwTerminate();
}
