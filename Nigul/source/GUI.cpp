#include "gui.h"

bool GUI::input(Model* model) {
	bool inputApplied = false;

	if (!model)
		return inputApplied;

	if (ImGui::IsKeyPressed(ImGuiKey_Delete) && model->selectedNodeId != -1 && model->selectedNodeId != 0) {
		ImGui::OpenPopup("Delete Node");
	}

	if (ImGui::BeginPopupModal("Delete Node", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Do you want to delete this node?\n\n");
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			inputApplied = true;
			model->deleteNode(model->selectedNodeId);
			model->selectedNodeId = -1;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
		inputApplied = true;

		Node* cameraNode = model->getMainCameraNode();
		glm::mat4 cameraMatrix = cameraNode->matrix;

		// Get the position and rotation
		glm::vec3 position = glm::vec3(cameraMatrix[3]);
		glm::quat rotation = glm::quat_cast(cameraMatrix);
		glm::vec3 orientation = glm::vec3(-cameraMatrix[0][2], -cameraMatrix[1][2], -cameraMatrix[2][2]);
		glm::vec3 up = glm::vec3(1.0f, 0.0f, 0.0f);

		// Handles key inputs
		if (ImGui::IsKeyDown(ImGuiKey_W)) {
			position += speed * glm::vec3(rotation * glm::vec4(0, 0, -1, 0));
		}
		if (ImGui::IsKeyDown(ImGuiKey_S)) {
			position -= speed * glm::vec3(rotation * glm::vec4(0, 0, -1, 0));
		}
		if (ImGui::IsKeyDown(ImGuiKey_A)) {
			position += speed * glm::vec3(rotation * glm::vec4(-1, 0, 0, 0));
		}
		if (ImGui::IsKeyDown(ImGuiKey_D)) {
			position -= speed * glm::vec3(rotation * glm::vec4(-1, 0, 0, 0));
		}
		if (ImGui::IsKeyDown(ImGuiKey_Q)) {
			position += speed * glm::vec3(rotation * glm::vec4(0, 1, 0, 0));
		}
		if (ImGui::IsKeyDown(ImGuiKey_E)) {
			position -= speed * glm::vec3(rotation * glm::vec4(0, 1, 0, 0));
		}

		ImGuiIO& io = ImGui::GetIO();
		if (firstClick) {
			// Fetches the coordinates of the cursor
			axisX = io.MousePos.x;
			axisY = io.MousePos.y;
			firstClick = false;
		}

		// Fetches the coordinates of the cursor
		float mouseX = io.MousePos.x;
		float mouseY = io.MousePos.y;

		// Normalizes and shifts the coordinates of the cursor such that they begin in the middle of the screen
		// and then "transforms" them into degrees 
		float yaw = sensitivity * (float)(mouseY - axisY);
		float pitch = sensitivity * (float)(mouseX - axisX);

		glm::quat pitchQuad = glm::rotate(rotation, glm::radians(pitch), glm::normalize(glm::cross(orientation, up)));

		if (glm::angle(up * pitchQuad, up) < glm::radians(85.0f)) {
			rotation = pitchQuad;
		}

		glm::quat yawQuad = glm::rotate(rotation, glm::radians(-yaw), up);
		if (glm::angle(orientation * yawQuad, orientation) < glm::radians(85.0f)) {
			rotation = yawQuad;
		}

		// Apply to the camera matrix
		cameraNode->matrix = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation);
		model->updateTreeFrom(cameraNode, glm::mat4(1.0f));

	}
	else if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
		firstClick = true;
	}

	return inputApplied;
}

bool GUI::displayGizmo(Model* model)
{
	if (!model)
		return false;

	bool hasChanged = false;
	Node* selectedNode = model->getSelectedNode();
	Camera* camera = model->getMainCamera();
	float* matrix = glm::value_ptr(selectedNode->globalMatrix);

	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
	hasChanged |= ImGui::InputFloat3("Tr", matrixTranslation);
	hasChanged |= ImGui::InputFloat3("Rt", matrixRotation);
	hasChanged |= ImGui::InputFloat3("Sc", matrixScale);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

	if (mCurrentGizmoOperation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
			mCurrentGizmoMode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
			mCurrentGizmoMode = ImGuizmo::WORLD;
	}
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	hasChanged |= ImGuizmo::Manipulate(glm::value_ptr(camera->viewMatrix), glm::value_ptr(camera->projectionMatrix), mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL);

	if (hasChanged) {
		selectedNode->rewriteMatrix();
		model->updateTreeFrom(selectedNode, glm::mat4(1.0f));
	}

	return hasChanged;
}

void GUI::init(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

void GUI::createFrame(SceneManager* scene, Renderer* renderer) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	logic(scene, renderer);
}

bool GUI::isMouseAvaliable() {
	ImGuiIO& io = ImGui::GetIO();
	return !io.WantCaptureMouse;
}

void GUI::displayNode(Model* model, Node* node) {
	if (!model)
		return;

	ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	ImGuiTreeNodeFlags node_flags = base_flags;
	const bool is_selected = (model->selectedNodeId == node->id);
	if (is_selected)
		node_flags |= ImGuiTreeNodeFlags_Selected;
	if (node->isLeaf())
		node_flags |= ImGuiTreeNodeFlags_Leaf;

	bool node_open = ImGui::TreeNodeEx(node->name.c_str(), node_flags);
	if (ImGui::IsItemClicked()) {
		model->selectedNodeId = node->id;  // Update selected node on click
	}

	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload("CHANGE_NODE", &node->id, sizeof(int));
		ImGui::Text("Move node %d", node->id);
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CHANGE_NODE"))
		{
			int payload_n = *(const int*)payload->Data;
			if (payload_n != 0)
				model->reparentNode(payload_n, node->id);
		}
		model->selectedNodeId = -1;
		ImGui::EndDragDropTarget();
	}

	if (node_open) {
		for (auto& child : node->children) {
			if (!child)
				continue;
			displayNode(model , child.get());
		}
		ImGui::TreePop();
	}
}

void GUI::displayGeneral(Model* model) {
	if (!model)
		return;

	Node* node = model->getSelectedNode();
	if (!node)
		return;

	ImGui::SeparatorText("General");
	ImGui::InputText("Name", &node->name[0], 100);
	ImGui::Text("Id: %d", node->id);
}

void GUI::displayLight(Model* model) {
	if (!model)
		return;

	Node* node = model->getSelectedNode();
	if (!node || !node->light)
		return;

	ImGui::SeparatorText("Light properties");
	Light* light = node->light;
	model->lightFlags[Colors] = ImGui::ColorEdit3("Color", &light->color[0]);
	model->lightFlags[Ranges] = ImGui::SliderFloat("Range", &light->range, 0.0f, 30.0f); // Continue...
	model->lightFlags[Intensities] = ImGui::SliderFloat("Intensity", &light->intensity, 0.0f, 100.0f);
	ImGui::SeparatorText("Shadow properties");
	model->lightFlags[CastShadows] = ImGui::Checkbox("Cast shadow", &light->castShadows);
	model->lightFlags[ShadowBiases] = ImGui::SliderFloat("Shadow bias", &light->shadowBias, 0.000001, 0.00001, "%.6f");
	ImGui::Checkbox("Display shadowmap", &showShadowMap);

	switch (node->light->getType()) { // Delete this methods, and put the code here
		case LIGHT_TYPE::DIRECTIONAL:
			break;
		case LIGHT_TYPE::POINTLIGHT: {
			PointLight* pointLight = static_cast<PointLight*>(node->light);
			model->lightFlags[Attenuations] = ImGui::SliderFloat("Attenuation", &pointLight->attenuation, 0.0f, 15.0f);
			break;
		}
		case LIGHT_TYPE::SPOTLIGHT: {
			SpotLight* spotLight = static_cast<SpotLight*>(node->light);
			model->lightFlags[InnerConeAngles] = ImGui::SliderFloat("Cutoff", &spotLight->innerConeAngle, 0.0f, 1.0f);
			model->lightFlags[OuterConeAngles] = ImGui::SliderFloat("Outer Cutoff", &spotLight->outerConeAngle, 0.0f, 1.0f);
			break;
		}
	}

	if (showShadowMap) {
		GLuint textureID = light->shadowMap->tex->ID;
		ImGui::Begin("Shadow Map", &showShadowMap);
		ImTextureID texID = reinterpret_cast<void*>(static_cast<intptr_t>(textureID));
		ImGui::Image(texID, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
	}
}

void GUI::displayCamera(Model* model) {
	if (!model)
		return;

	Node* selectedNode = model->getSelectedNode();
	if (!selectedNode || !selectedNode->camera)
		return;

	ImGui::SeparatorText("Camera");
	PerspectiveCamera* perspCamera = dynamic_cast<PerspectiveCamera*>(selectedNode->camera);
	ImGui::SliderFloat("FOV", &perspCamera->fov, 0.0f, 180.0f);
	ImGui::SliderFloat("Near", &perspCamera->nearPlane, 0.0f, 1.0f);
	ImGui::SliderFloat("Far", &perspCamera->farPlane, 0.0f, 300.0f);
	ImGui::SliderFloat("Aspect ratio", &perspCamera->aspectRatio, 0.0f, 2.0f);
}

void GUI::displayActions(SceneManager* scene) {
	// Here we select the skybox
	ImGui::SeparatorText("Load amd save");
	if (ImGui::BeginCombo("Render skybox", scene->mainSkybox.c_str())) {
		for (auto& skybox : scene->getAllSkyboxes()) {
			bool is_selected = (scene->mainSkybox == skybox);
			if (ImGui::Selectable(skybox.c_str(), is_selected)) {
				scene->loadSkybox(skybox);
				if (scene->getMainModel())
					scene->getMainModel()->hasSkyboxChanged = true;
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo("Render model", scene->mainModel.c_str())) {
		for (auto& model : scene->getAllModels()) {
			bool is_selected = (scene->mainModel == model);
			if (ImGui::Selectable(model.c_str(), is_selected)) {
				scene->loadModel(model);
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	Model* model = scene->getMainModel();
	if (model) {
		ImGui::SeparatorText("Add nodes");
		if (ImGui::Button("Add transform node")) {
			model->AddBasicNode();
		}
		if (ImGui::Button("Add pointlight node")) {
			model->AddLightNode(LIGHT_TYPE::POINTLIGHT);
		}
		if (ImGui::Button("Add spotlight node")) {
			model->AddLightNode(LIGHT_TYPE::SPOTLIGHT);
		}
		if (ImGui::Button("Add directional light node")) {
			model->AddLightNode(LIGHT_TYPE::DIRECTIONAL);
		}
		if (ImGui::Button("Update model")) {
			model->save();
		}
	}
}

void GUI::displayRender(Model* model) {
	if (!model) return;

	ImGui::SeparatorText("Render");
	model->hasAmbientLightChanged = ImGui::SliderFloat("Ambient light", &model->ambientLight, 0.0f, 1.0f);
	model->hasAmbientColorChanged = ImGui::ColorEdit3("Ambient color", &model->ambientColor[0]);
	model->hasShadowDarknessChanged = ImGui::SliderFloat("Shadow darkness", &model->shadowDarkness, 0.0f, 1.0f);
	model->hasReflectionFactorChanged = ImGui::SliderFloat("Reflection factor", &model->reflectionFactor, 0.0f, 1.0f);
}

void GUI::logic(SceneManager* scene, Renderer* renderer)
{
	Model* model = scene->getMainModel();

	input(model);
	ImGui::Begin("Engine");
	if (ImGui::BeginTabBar("EngineTabBar"))
	{
		if (ImGui::BeginTabItem("Scene"))
		{
			if (ImGui::CollapsingHeader("Tree")) {
				displayNode(model, model->root.get());
			}

			if (ImGui::CollapsingHeader("Node properties")) {
				ImGui::SeparatorText("Transform");

				displayGizmo(model);
				displayGeneral(model);
				displayLight(model);
				displayCamera(model);
			}

			if (ImGui::CollapsingHeader("Render properties")) {
				displayRender(model);
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Actions"))
		{
			displayActions(scene);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("FX"))
		{
			// Checkbox if the aberration is applied
			ImGui::SeparatorText("Tonemapper");
			ImGui::Checkbox("Aberration", &renderer->postpo->isToneMappingApplied);
			if (renderer->postpo->isToneMappingApplied) {
				ImGui::SliderFloat("Aberration factor", &renderer->postpo->exposure, 0.0f, 4.0f);
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

GUI::~GUI()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}