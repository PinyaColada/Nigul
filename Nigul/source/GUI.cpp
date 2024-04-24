#include "gui.h"

GUI::GUI(Scene* scene, postProcessing* quad) {
	mode = GUI_MODE::MODEL;
	selectedNodeId = -1;
	this->scene = scene;
	this->quad = quad;
}

void GUI::input() {
	if (ImGui::IsKeyPressed(ImGuiKey_Delete) && selectedNodeId != -1 && selectedNodeId != 0) {
		ImGui::OpenPopup("Delete Node");
	}

	if (ImGui::BeginPopupModal("Delete Node", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Do you want to delete this node?\n\n");
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			scene->model->deleteNode(selectedNodeId);
			selectedNodeId = -1;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

bool GUI::editTransform(const Camera& camera, glm::f32* ptr_matrix)
{
	bool hasChanged = false;

	if (ptr_matrix == nullptr)
		return hasChanged;

	float* matrix = ptr_matrix;

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
	hasChanged |= ImGuizmo::Manipulate(glm::value_ptr(camera.viewMatrix), glm::value_ptr(camera.projectionMatrix), mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL);

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

void GUI::createFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

bool GUI::isMouseAvaliable() {
	ImGuiIO& io = ImGui::GetIO();
	return !io.WantCaptureMouse;
}

void GUI::displayNode(Node& node) {
	ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	ImGuiTreeNodeFlags node_flags = base_flags;
	const bool is_selected = (selectedNodeId == node.id);
	if (is_selected)
		node_flags |= ImGuiTreeNodeFlags_Selected;
	if (node.isLeaf())
		node_flags |= ImGuiTreeNodeFlags_Leaf;
	bool node_open = ImGui::TreeNodeEx(node.name.c_str(), node_flags);
	if (ImGui::IsItemClicked()) {
		selectedNodeId = node.id;  // Update selected node on click
	}

	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload("CHANGE_NODE", &node.id, sizeof(int));
		ImGui::Text("Move node %d", node.id);
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CHANGE_NODE"))
		{
			int payload_n = *(const int*)payload->Data;
			if (payload_n != 0)
				scene->model->reparentNode(payload_n, node.id);
		}
		selectedNodeId = -1;
		ImGui::EndDragDropTarget();
	}

	if (node_open) {
		for (auto& child : node.children) {
			if (child == nullptr)
				continue;
			displayNode(*child);
		}
		ImGui::TreePop();
	}
}

void GUI::logic()
{
	input();
	ImGui::Begin("Engine");

	bool hasChanged = false;

	if (ImGui::BeginTabBar("MyTabBar"))
	{
		if (ImGui::BeginTabItem("Scene"))
		{
			if (ImGui::CollapsingHeader("Tree")) {
				Node* root = scene->model->root.get();
				displayNode(*root);
			}

			if (ImGui::CollapsingHeader("Node properties")) {
				if (selectedNodeId != -1) {
					ImGui::SeparatorText("Transform");

					Node* selectedNode = scene->model->getNodeByID(selectedNodeId);
					hasChanged |= editTransform(*scene->camera, glm::value_ptr(selectedNode->globalMatrix));

					ImGui::SeparatorText("General");
					ImGui::InputText("Name", &selectedNode->name[0], 100);
					ImGui::Text("Id: %d", selectedNode->id);

					if (selectedNode->light) {
						ImGui::SeparatorText("Light");
						Light* light = selectedNode->light;
						hasChanged |= ImGui::ColorEdit3("Color", &light->color[0]);
						hasChanged |= ImGui::SliderFloat("Range", &light->range, 0.0f, 30.0f);
						hasChanged |= ImGui::SliderFloat("Intensity", &light->intensity, 0.0f, 100.0f);
						hasChanged |= ImGui::Checkbox("Cast shadow", &light->castShadows);
						ImGui::Checkbox("Display shadowmap", &showShadowMap);

						DirectionalLight* dirLight = nullptr;
						PointLight* pointLight = nullptr;
						SpotLight* spotLight = nullptr;

						switch (selectedNode->light->getType()) {
							case LIGHT_TYPE::DIRECTIONAL:
								dirLight = dynamic_cast<DirectionalLight*>(selectedNode->light);
								hasChanged |= ImGui::InputFloat2("Orthogonal size", &dirLight->camera.size[0]);
								hasChanged |= ImGui::SliderFloat("Distance", &dirLight->distance, 0.0f, 100.0f);
								hasChanged |= ImGui::SliderFloat("Shadow bias", &dirLight->shadowBias, 0.000001, 0.00001, "%.6f");
								hasChanged |= ImGui::SliderFloat("Near plane", &dirLight->camera.nearPlane, 0.0f, 100.0f);
								hasChanged |= ImGui::SliderFloat("Far plane", &dirLight->camera.farPlane, 0.0f, 100.0f);
								break;
							case LIGHT_TYPE::POINTLIGHT:
								pointLight = dynamic_cast<PointLight*>(selectedNode->light);
								hasChanged |= ImGui::SliderFloat("Attenuation", &pointLight->attenuation, 0.0f, 15.0f);
								break;
							case LIGHT_TYPE::SPOTLIGHT:
								spotLight = dynamic_cast<SpotLight*>(selectedNode->light);
								hasChanged |= ImGui::SliderFloat("Cutoff", &spotLight->innerConeAngle, 0.0f, 1.0f);
								hasChanged |= ImGui::SliderFloat("Outer Cutoff", &spotLight->outerConeAngle, 0.0f, 1.0f);
								break;
						}
						if (showShadowMap) {
							GLuint textureID = selectedNode->light->shadowMap->GetTexture();
							ImGui::Begin("Shadow Map", &showShadowMap);
							ImTextureID texID = reinterpret_cast<void*>(static_cast<intptr_t>(textureID));
							ImGui::Image(texID, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
							ImGui::End();
						}
					}

					PerspectiveCamera* perspCamera = nullptr;
					if (selectedNode->camera) {
						ImGui::SeparatorText("Camera");
						perspCamera = dynamic_cast<PerspectiveCamera*>(selectedNode->camera);
						hasChanged |= ImGui::SliderFloat("FOV", &perspCamera->fov, 0.0f, 180.0f);
						hasChanged |= ImGui::SliderFloat("Near", &perspCamera->nearPlane, 0.0f, 100.0f);
						hasChanged |= ImGui::SliderFloat("Far", &perspCamera->farPlane, 0.0f, 100.0f);
						hasChanged |= ImGui::SliderFloat("Aspect ratio", &perspCamera->aspectRatio, 0.0f, 2.0f);
					}
					if (selectedNode->mesh) {
						ImGui::SeparatorText("Mesh");
					}

					if (true) {
						selectedNode->rewriteMatrix();
					}
				}
			}

			if (ImGui::CollapsingHeader("Scene properties")) {
				ImGui::SeparatorText("Render");
				ImGui::SliderFloat("Ambient light", &scene->ambientLight, 0.0f, 1.0f);
				ImGui::ColorEdit3("Ambient color", &scene->ambientColor[0]);
				ImGui::SliderFloat("Shadow darkness", &scene->shadowDarkness, 0.0f, 1.0f);
				ImGui::SliderFloat("Reflection factor", &scene->reflectionFactor, 0.0f, 1.0f);
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Actions"))
		{
			if (ImGui::Button("Add transform node")) {
				scene->model->AddBasicNode();
			}
			if (ImGui::Button("Add pointlight node")) {
				scene->model->AddPointLightNode();
			}
			if (ImGui::Button("Add spotlight node")) {
				scene->model->AddSpotLightNode();
			}
			if (ImGui::Button("Add directional light node")) {
				scene->model->AddDirectionalLightNode();
			}
			// Here we select the skybox
			if (ImGui::BeginCombo("Select skybox to render", scene->mainSkybox.c_str())) {
				for (auto& skybox : scene->getAllSkyboxes()) {
					bool is_selected = (scene->mainSkybox == skybox);
					if (ImGui::Selectable(skybox.c_str(), is_selected)) {
						scene->loadSkybox(skybox);
					}
					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Post process"))
		{
			// Checkbox if the aberration is applied
			ImGui::Checkbox("Aberration", &quad->isAberrationApplied);
			if (quad->isAberrationApplied) {
				ImGui::SliderFloat("Aberration factor", &quad->amountAberration, 0.0f, 1.0f);
			}
			ImGui::Checkbox("Tone mapping", &quad->isToneMappingApplied);
			
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::destroy()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}