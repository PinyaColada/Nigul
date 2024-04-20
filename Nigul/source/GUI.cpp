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

void GUI::editTransform(const Camera& camera, glm::f32* ptr_matrix)
{
	if (ptr_matrix == nullptr)
		return;

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
	ImGui::InputFloat3("Tr", matrixTranslation);
	ImGui::InputFloat3("Rt", matrixRotation);
	ImGui::InputFloat3("Sc", matrixScale);
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
	ImGuizmo::Manipulate(glm::value_ptr(camera.viewMatrix), glm::value_ptr(camera.projectionMatrix), mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL);
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
					editTransform(*scene->camera, glm::value_ptr(selectedNode->globalMatrix));
					if (ImGuizmo::IsUsing) {
						selectedNode->rewriteMatrix();
					}

					ImGui::SeparatorText("General");
					ImGui::InputText("Name", &selectedNode->name[0], 100);
					ImGui::Text("Id: %d", selectedNode->id);

					if (selectedNode->light != nullptr) {
						ImGui::SeparatorText("Light");
						ImGui::ColorEdit3("Color", &selectedNode->light->color[0]);
						ImGui::SliderFloat("Intensity", &selectedNode->light->intensity, 0.0f, 100.0f);
						const char* lightTypes[] = { "Point", "Spot", "Directional" };
						ImGui::Combo("Type", (int*)&selectedNode->light->type, lightTypes, IM_ARRAYSIZE(lightTypes));
						if (selectedNode->light->type == LIGHT_TYPE::SPOTLIGHT) {
							ImGui::SliderFloat("Cutoff", &selectedNode->light->innerConeAngle, 0.0f, 1.0f);
							ImGui::SliderFloat("Outer Cutoff", &selectedNode->light->outerConeAngle, 0.0f, 1.0f);
							ImGui::SliderFloat("Range", &selectedNode->light->range, 0.0f, 30.0f);
						}
						else if (selectedNode->light->type == LIGHT_TYPE::POINTLIGHT) {
							ImGui::SliderFloat("Range", &selectedNode->light->range, 0.0f, 30.0f);
						}
						if (selectedNode->light->type == LIGHT_TYPE::DIRECTIONAL || selectedNode->light->type == LIGHT_TYPE::SPOTLIGHT) {
							if (ImGui::InputFloat3("Look at", &selectedNode->light->lookAt[0])) {
								selectedNode->light->direction = glm::normalize(selectedNode->light->lookAt - selectedNode->light->position);
							}
						}
						ImGui::Checkbox("Cast shadows", &selectedNode->light->castShadows);
						ImGui::Checkbox("Display shadowmap", &showShadowMap);
						if (showShadowMap) {
							GLuint textureID = selectedNode->light->shadowMap->GetTexture();
							ImGui::Begin("Shadow Map", &showShadowMap);
							ImTextureID texID = reinterpret_cast<void*>(static_cast<intptr_t>(textureID));
							ImGui::Image(texID, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
							ImGui::End();
						}
						if (selectedNode->light->type == LIGHT_TYPE::DIRECTIONAL) {
							ImGui::InputFloat("Orthogonal size", &selectedNode->light->orthogonalSize);
							ImGui::InputFloat("Shadow bias", &selectedNode->light->shadowBias, 0.000001, 0.00001, "%.6f");
							ImGui::InputFloat("Near plane", &selectedNode->light->nearPlane);
							ImGui::InputFloat("Far plane", &selectedNode->light->farPlane);
						}
					}
					if (selectedNode->camera) {
						ImGui::SeparatorText("Camera");
						ImGui::SliderFloat("FOV", &selectedNode->camera->fov, 0.0f, 180.0f);
						ImGui::SliderFloat("Near", &selectedNode->camera->nearPlane, 0.0f, 100.0f);
						ImGui::SliderFloat("Far", &selectedNode->camera->farPlane, 0.0f, 100.0f);
						ImGui::SliderFloat("Aspect ratio", &selectedNode->camera->aspectRatio, 0.0f, 2.0f);
					}
					if (selectedNode->mesh) {
						ImGui::SeparatorText("Mesh");
					}
				}
			}

			if (ImGui::CollapsingHeader("Scene properties")) {
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
			if (ImGui::Button("Add light node")) {
				scene->model->AddLightNode();
			}
			if (ImGui::Button("Import gltf")) {

			}
			if (ImGui::Button("Export scene as gltf")) {

			}
			// Here we select the skybox
			if (ImGui::BeginCombo("Skybox", scene->mainSkybox.c_str())) {
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
			if (ImGui::Button("Save camera")) {
				scene->model->saveCamera();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Options"))
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