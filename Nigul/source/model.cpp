#include"Model.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_STB_IMAGE 
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tinyGLTF/tinyGLTF.h>

#include <chrono>

tinygltf::Model model;

Light::Light() {
	color = glm::vec3(1.0f, 1.0f, 1.0f);
	intensity = 1.0;
	type = LIGHT_TYPE::POINTLIGHT;
	range = 3.0;
	innerConeAngle = 0.9;
	outerConeAngle = 0.95;
}

Node::Node() {
	name = "Root";
	id = 0;
}

Node::Node(int id, glm::mat4 matrix, glm::mat4 globalMatrix, Node* parent, std::string name):
	matrix(matrix), globalMatrix(globalMatrix), parent(parent), name(name), id(id)
{}

void Node::addChild(std::unique_ptr<Node> child){
	if (child) {
		child->parent = this; // Set this node as the parent
	}
	children.push_back(std::move(child));
}

void Node::rewriteMatrix(){
	if (parent)
		this->matrix = glm::inverse(parent->globalMatrix) * this->globalMatrix;
	else
		this->matrix = this->globalMatrix;
}

bool Node::isLeaf() const
{
	if (children.empty())
		return true;
	return false;
}

Model::Model()
{
	root = std::make_unique<Node>();
	numNodes = 1;
}

void Model::load()
{
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, this->file.c_str());

	if (!warn.empty()) {
		std::cout << "Warn: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cerr << "Err: " << err << std::endl;
	}

	if (!ret) {
		std::cerr << "Failed to load glTF: " << this->file.c_str() << std::endl;
		return;
	}

	loadTextures();
	loadMaterials();
	loadMeshes();
	loadLights();
	loadCameras();

	// Traverse all nodes
	auto rootNodes = findRootNodes();
	for (unsigned int rootNodeIndex : rootNodes) {
		traverseNode(rootNodeIndex, root->matrix, root.get());
	}
}

void Model::Draw(Shader* shader, Camera* camera)
{
	// Update the draw calls
	drawCalls.clear();
	updateTree(*root, *shader, glm::mat4(1.0f));

	if (!skipTransparent) {
		// Then we sort if there are different transparency modes
		std::sort(drawCalls.begin(), drawCalls.end(), [camera](const DrawCall& a, const DrawCall& b) {
			// If the transparency are different then we render first the opaque objects
			if (a.mesh->material->alphaMode != b.mesh->material->alphaMode)
				return a.mesh->material->alphaMode > b.mesh->material->alphaMode;

			// If the transparency is the same, we need to know the distance to the camera
			glm::vec3 aPos = a.globalMatrix[3];
			glm::vec3 bPos = b.globalMatrix[3];
			float aDist = glm::length(aPos - camera->position);
			float bDist = glm::length(bPos - camera->position);

			// If the object is opaque, we render first the closest object
			if (a.mesh->material->alphaMode == OPAQUE) {
				return aDist > bDist;
			}
			// If the object is transparent, we render first the farthest object
			return aDist < bDist;
		});
	}

	// finally, we draw
	for (auto& drawCall : drawCalls) {
		drawCall.call(camera);
	}
}

void Model::updateTree(Node& node, Shader& shader, glm::mat4 parentMatrix)
{
	// Compute the world matrix for the current node
	node.globalMatrix = parentMatrix * node.matrix;

	// Draw the mesh of the current node
	if (node.mesh != nullptr) {
		if (skipTransparent == false || (skipTransparent == true && node.mesh->material->alphaMode == OPAQUE_MODE)) {
			drawCalls.push_back(DrawCall{ node.mesh, &shader, node.globalMatrix });
		}
	}

	if (node.light != nullptr) {
		node.light->position = glm::vec3(node.globalMatrix[3]);
	}

	// Recursively draw all children
	for (auto& child : node.children) {
		updateTree(*child, shader, node.globalMatrix);
	}
}

Node* Model::searchNodeByID(Node* node, int targetId)
{
	if (node == nullptr) {
		return nullptr;
	}

	if (node->id == targetId) {
		return node;
	}

	// Check each child
	for (auto& child : node->children) {
		Node* result = searchNodeByID(child.get(), targetId);
		if (result != nullptr) {
			return result; // Found the node
		}
	}

	return nullptr; // Node not found in this subtree
}

Node* Model::getNodeByID(int id) {
	Node* targetNode = searchNodeByID(root.get(), id);
	if (targetNode != nullptr)
		return targetNode;

	std::cerr << "Node with id " << id << " not found." << std::endl;
	return nullptr;
}

std::vector<unsigned int> Model::findRootNodes()
{
	std::unordered_set<unsigned int> potentialRoots;

	// Step 1: Add all nodes to potential roots
	for (unsigned int i = 0; i < model.nodes.size(); ++i) {
		potentialRoots.insert(i);
	}

	// Step 2: Remove nodes that are referenced as children
	for (const auto& node : model.nodes) {
		for (int childIndex : node.children) {
			potentialRoots.erase(childIndex);
		}
	}

	// Step 3: Remaining nodes are the root nodes
	return std::vector<unsigned int>(potentialRoots.begin(), potentialRoots.end());
}

void Model::traverseNode(unsigned int nextNode, glm::mat4 parentMatrix, Node* parentNode)
{
	const tinygltf::Node& node = model.nodes[nextNode];
	// Directly access the name or default to "Node"
	std::string nameNode = node.name.empty() ? "Node" : node.name;

	// Initialize the node's transformation matrix
	glm::mat4 nodeMatrix = glm::mat4(1.0f);

	// Use TinyGLTF's direct access to transformation properties
	if (!node.matrix.empty()) {
		nodeMatrix = glm::make_mat4(node.matrix.data());
	}
	else {
		if (!node.translation.empty()) {
			glm::vec3 translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
			nodeMatrix = glm::translate(nodeMatrix, translation);
		}
		if (!node.rotation.empty()) {
			glm::quat rotation = glm::make_quat(node.rotation.data());
			nodeMatrix *= glm::mat4_cast(rotation);
		}
		if (!node.scale.empty()) {
			glm::vec3 scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
			nodeMatrix = glm::scale(nodeMatrix, scale);
		}
	}

	glm::mat4 combinedMatrix = parentMatrix * nodeMatrix;

	// Check if the node contains a mesh and if it does, create a Node
	auto newNode = std::make_unique<Node>(numNodes, nodeMatrix, combinedMatrix, parentNode, nameNode);

	if (node.mesh != -1) {
		Mesh* newMesh = lodMesh[node.mesh].get();
		newNode->mesh = newMesh;
	}

	if (node.light != -1) {
		Light* newLight = lodLight[node.light].get(); 
		newLight->position = glm::vec3(newNode->globalMatrix[3]);
		newLight->direction = glm::normalize(-glm::vec3(newNode->globalMatrix[0][2], newNode->globalMatrix[1][2], newNode->globalMatrix[2][2]));
		newNode->light = newLight;
	}

	if (node.camera != -1 || node.name == "Camera") {
		int indexOfCamera = node.camera;
		if (lodCamera.size() == 0) {
			lodCamera.push_back(std::make_unique<Camera>());
			indexOfCamera = lodCamera.size() - 1;
		}
		Camera* newCamera = lodCamera[indexOfCamera].get();

		if (node.camera == 0 || node.name == "Camera")
			mainCameraId = numNodes;

		newCamera->position = glm::vec3(newNode->globalMatrix[3]);
		newCamera->orientation = glm::rotate(glm::quat_cast(newNode->globalMatrix), glm::vec3(0.0f, -1.0f, 0.0f));
		newNode->camera = newCamera;
	}

	// If this is the root node, assign it to the Model's root and set the parent of the node
	Node* newParentNode = newNode.get();
	parentNode->addChild(std::move(newNode));
	numNodes++;

	// Check if the node has children, and if it does, apply this function to them
	for (size_t i = 0; i < node.children.size(); ++i) {
		traverseNode(node.children[i], combinedMatrix, newParentNode);
	}
}

std::vector<GLuint> Model::getIndices(int accessorIndex)
{
	std::vector<GLuint> indices;

	const tinygltf::Accessor& accessor = model.accessors[accessorIndex];
	const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
	const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

	const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
	size_t byteStride = accessor.ByteStride(bufferView);

	switch (accessor.componentType) {
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
		for (size_t i = 0; i < accessor.count; ++i) {
			unsigned int value;
			std::memcpy(&value, dataPtr + i * byteStride, sizeof(unsigned int));
			indices.push_back(static_cast<GLuint>(value));
		}
		break;
		break;
	}
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
		for (size_t i = 0; i < accessor.count; ++i) {
			unsigned short value;
			std::memcpy(&value, dataPtr + i * byteStride, sizeof(unsigned short));
			indices.push_back(static_cast<GLuint>(value));
		}
		break;
	}
	case TINYGLTF_COMPONENT_TYPE_SHORT: {
		for (size_t i = 0; i < accessor.count; ++i) {
			short value;
			std::memcpy(&value, dataPtr + i * byteStride, sizeof(short));
			indices.push_back(static_cast<GLuint>(value));
		}
		break;
	}
	default:
		throw std::runtime_error("Unsupported component type in accessor");
	}

	return indices;
}

void Model::loadCameras() {
	// Preallocate space for lodCam
	lodCamera.reserve(model.cameras.size());

	for (size_t i = 0; i < model.cameras.size(); ++i) {
		auto camera = std::make_unique<Camera>();

		if (model.cameras[i].type == "perspective") {
			camera->type = CameraType::PERSPECTIVE;
			camera->aspectRatio = model.cameras[i].perspective.aspectRatio;
			camera->fov = model.cameras[i].perspective.yfov;
			camera->nearPlane = model.cameras[i].perspective.znear;
			camera->farPlane = model.cameras[i].perspective.zfar;
		}
		else if (model.cameras[i].type == "orthographic") {
			camera->type = CameraType::ORTHOGRAPHIC;
		}
		else {
			std::cerr << "Unknown camera type: " << model.cameras[i].type << std::endl;
			throw std::runtime_error("Unsupported camera type encountered");
		}

		lodCamera.push_back(std::move(camera)); 
	}
}

void Model::loadLights()
{
	// Preallocate space for lights
	lodLight.reserve(model.lights.size());

	for (size_t i = 0; i < model.lights.size(); ++i) {
		auto light = std::make_unique<Light>(); // Create a unique_ptr managing a new Light object
		light->color = glm::vec3(model.lights[i].color[0], model.lights[i].color[1], model.lights[i].color[2]);
		light->intensity = model.lights[i].intensity;

		if (model.lights[i].type == "point") {
			light->type = LIGHT_TYPE::POINTLIGHT;
			light->range = model.lights[i].range;
		}
		else if (model.lights[i].type == "spot") {
			light->type = LIGHT_TYPE::SPOTLIGHT;
			light->innerConeAngle = model.lights[i].spot.innerConeAngle;
			light->outerConeAngle = model.lights[i].spot.outerConeAngle;
		}
		else if (model.lights[i].type == "directional") {
			light->type = LIGHT_TYPE::DIRECTIONAL;
		}
		else {
			std::cerr << "Unknown light type: " << model.lights[i].type << std::endl;
			throw std::runtime_error("Unsupported light type encountered");
		}

		lodLight.push_back(std::move(light)); // Add the unique_ptr to the vector
	}
}

void Model::loadTextures()
{
	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);

	// Preallocate space for lodTex
	lodTex.resize(model.images.size());

	for (int i = 0; i < model.images.size(); i++)
	{
		// URI of current texture
		std::string texPath = model.images[i].uri;

		// Load texture and add it to lodTex
		lodTex[i] = std::make_unique<Texture>((fileDirectory + texPath).c_str(), (GLuint)i);
	}

	for (size_t i = 0; i < lodTex.size(); i++) {
		lodTex[i]->texToOpenGL();
	}

}

void Model::loadMaterials() {
	// Assuming `gltfModel` is a tinygltf::Model object containing the loaded GLTF data
	for (const auto& mat : model.materials) {
		auto material = std::make_unique<Material>();

		// Set name
		material->name = mat.name;

		// Set doubleSided
		material->doubleSided = mat.doubleSided;

		// Set alphaMode
		if (mat.alphaMode == "BLEND") {
			material->alphaMode = BLEND_MODE;
		}
		else {
			material->alphaMode = OPAQUE_MODE;
		}

		// Set baseColorFactor
		if (mat.pbrMetallicRoughness.baseColorFactor.size() == 4) {
			material->pbrMetallicRoughness.baseColorFactor = glm::vec4(
				mat.pbrMetallicRoughness.baseColorFactor[0],
				mat.pbrMetallicRoughness.baseColorFactor[1],
				mat.pbrMetallicRoughness.baseColorFactor[2],
				mat.pbrMetallicRoughness.baseColorFactor[3]
			);
		}

		// Set metallicFactor and roughnessFactor
		material->pbrMetallicRoughness.metallicFactor = mat.pbrMetallicRoughness.metallicFactor;
		material->pbrMetallicRoughness.roughnessFactor = mat.pbrMetallicRoughness.roughnessFactor;

		// Load and assign textures (example for baseColorTexture)
		if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0) { // If there is an albedo texture
			int textureIndex = mat.pbrMetallicRoughness.baseColorTexture.index;
			material->pbrMetallicRoughness.baseColorTexture = lodTex[textureIndex].get(); // Load the selected slot
		}
		else {
			material->pbrMetallicRoughness.baseColorTexture = lodTex.back().get(); // otherwise use the last texture (black texture)
		}

		// Load and assign metallic-roughness texture
		if (mat.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) { // If there is a metallicRoughness texture
			int textureIndex = mat.pbrMetallicRoughness.metallicRoughnessTexture.index;
			material->pbrMetallicRoughness.metallicRoughness = lodTex[textureIndex].get(); // Load the selected slot
		}
		else {
			material->pbrMetallicRoughness.metallicRoughness = lodTex.back().get(); // otherwise use the last texture (black texture)
		}

		if (mat.normalTexture.index >= 0) { // If there is a normal texture
			int textureIndex = mat.normalTexture.index;
			material->normalMap = lodTex[textureIndex].get(); // Load the selected slot
		}

		if (mat.occlusionTexture.index >= 0) { // If there is an occlusion texture
			int textureIndex = mat.occlusionTexture.index;
			material->occlusionTexture = lodTex[textureIndex].get(); // Load the selected slot
		}

		if (mat.emissiveTexture.index >= 0) { // If there is an emissive texture
			int textureIndex = mat.emissiveTexture.index;
			material->emissiveTexture = lodTex[textureIndex].get(); // Load the selected slot
		}

		// Add the shared_ptr<Material> to the lodMat vector
		lodMat.push_back(std::move(material));
	}
}

void Model::loadMeshes()
{
	// Go through all the meshes in the gltfModel
	for (size_t i = 0; i < model.meshes.size(); i++) {
		const auto& mesh = model.meshes[i];
		const auto& primitive = mesh.primitives[0];

		// Get all accessor indices
		unsigned int posAccInd = primitive.attributes.find("POSITION")->second;
		unsigned int normalAccInd = primitive.attributes.find("NORMAL")->second;

		std::vector<glm::vec3> positions = getVec3(posAccInd);
		std::vector<glm::vec3> normals = getVec3(normalAccInd);
		std::vector<glm::vec2> texUVs;

		std::vector<Texture> textures;

		// Check if TEXCOORD_0 exists
		auto texCoordIt = primitive.attributes.find("TEXCOORD_0");
		if (texCoordIt != primitive.attributes.end()) {
			unsigned int texAccInd = texCoordIt->second;
			texUVs = getVec2(texAccInd);
		}
		else {
			// Fill with zeros if TEXCOORD_0 does not exist
			texUVs.assign(positions.size(), glm::vec2(0.0f, 0.0f));
		}
		int indexMaterial = primitive.material;

		// Get indices
		unsigned int indAccInd = primitive.indices;
		std::vector<GLuint> indices = getIndices(indAccInd);

		// Combine the vertices, indices, and textures into a mesh
		std::vector<Vertex> vertices = assembleVertices(positions, normals, texUVs);
		auto meshPtr = std::make_unique<Mesh>(vertices, indices, lodMat[indexMaterial].get());

		// Add the mesh to the lodMesh vector
		lodMesh.push_back(std::move(meshPtr));
	}
}

std::vector<Vertex> Model::assembleVertices(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& texUVs)
{
	size_t numVertices = positions.size();
	std::vector<Vertex> vertices;
	vertices.reserve(numVertices); // Reserve space in the vector

	for (size_t i = 0; i < numVertices; i++) {
		vertices.emplace_back(Vertex{
			positions[i],
			normals[i],
			glm::vec3(1.0f, 1.0f, 1.0f),
			texUVs[i]
			});
	}

	return vertices;
}

std::vector<glm::vec2> Model::getVec2(int accessorIndex) {
	std::vector<glm::vec2> vec2Array;

	const tinygltf::Accessor& accessor = model.accessors[accessorIndex];
	const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
	const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

	const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
	size_t byteStride = accessor.ByteStride(bufferView);

	if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
		throw std::runtime_error("Accessor does not contain float type data");
	}
	if (byteStride != sizeof(glm::vec2)) {
		throw std::runtime_error("Byte stride does not match glm::vec2 size");
	}

	for (size_t i = 0; i < accessor.count; ++i) {
		glm::vec2 value;
		std::memcpy(&value, dataPtr + i * byteStride, sizeof(glm::vec2));
		vec2Array.push_back(value);
	}

	return vec2Array;
}

std::vector<glm::vec3> Model::getVec3(int accessorIndex) {
	std::vector<glm::vec3> vec3Array;

	const tinygltf::Accessor& accessor = model.accessors[accessorIndex];
	const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
	const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

	const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
	size_t byteStride = accessor.ByteStride(bufferView);

	if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
		throw std::runtime_error("Accessor does not contain float type data");
	}
	if (byteStride != sizeof(glm::vec3)) {
		throw std::runtime_error("Byte stride does not match glm::vec3 size");
	}

	for (size_t i = 0; i < accessor.count; ++i) {
		glm::vec3 value;
		std::memcpy(&value, dataPtr + i * byteStride, sizeof(glm::vec3));
		vec3Array.push_back(value);
	}

	return vec3Array;
}

void Model::reparentNode(int id, int newParentId) {
	if (id == 0)
		std::cerr << "Cannot change the parent of the root node." << std::endl;

	Node* targetNode = getNodeByID(id);
	if (targetNode == nullptr) {
		std::cerr << "Node with id " << id << " not found." << std::endl;
		return;
	}

	Node* newParentNode = getNodeByID(newParentId);
	if (newParentNode == nullptr) {
		std::cerr << "New parent node with id " << newParentId << " not found." << std::endl;
		return;
	}

	// Remove the node from its current parent's children list or from the root nodes
	std::unique_ptr<Node> nodeToMove;
	auto& siblings = targetNode->parent->children;
	auto it = std::find_if(siblings.begin(), siblings.end(),
		[targetNode](const std::unique_ptr<Node>& node) { return node.get() == targetNode; });
	if (it != siblings.end()) {
		nodeToMove = std::move(*it);
		siblings.erase(it);
	}

	// Change the parent of the node
	nodeToMove->parent = newParentNode;

	// Add the node to the new parent's children list
	newParentNode->children.push_back(std::move(nodeToMove));

	// Update the node's matrix
	targetNode->rewriteMatrix();
}

void Model::deleteNode(int id) {
	if (id == 0)
		std::cerr << "Cannot delete the root node." << std::endl;

	Node* targetNode = getNodeByID(id);
	if (targetNode == nullptr) {
		std::cerr << "Node with id " << id << " not found." << std::endl;
		return;
	}

	// Remove the node from its parent's children list or from the root nodes
	auto& siblings = targetNode->parent->children;
	siblings.erase(std::remove_if(siblings.begin(), siblings.end(),
		[targetNode](const std::unique_ptr<Node>& node) { return node.get() == targetNode; }), siblings.end());

}

void Model::passLightUniforms(Shader* shader)
{
	shader->Activate();
	const int maxLights = 3; 
	glm::vec3 lightColors[maxLights];
	int lightTypes[maxLights];
	glm::vec3 lightPositions[maxLights];
	glm::vec3 lightDirections[maxLights];
	float lightIntensities[maxLights];
	float lightRange[maxLights];
	float lightInnerConeAngles[maxLights];
	float lightOuterConeAngles[maxLights];
	float lightShadowBiases[maxLights];
	glm::mat4 lightProjectionMatrixes[maxLights];

	int indexTexture = lodTex.size();

	int lightShadowMaps[maxLights] = { indexTexture, indexTexture + 1 , indexTexture + 2};

	for (size_t i = 0; i < lodLight.size() && i < maxLights; i++) {
		auto light = lodLight[i].get();
		lightColors[i] = light->color;
		lightTypes[i] = static_cast<int>(light->type);
		lightIntensities[i] = light->intensity;
		lightPositions[i] = light->position;
		lightDirections[i] = light->direction;
		lightRange[i] = light->range;
		lightInnerConeAngles[i] = light->innerConeAngle;
		lightOuterConeAngles[i] = light->outerConeAngle;
		lightProjectionMatrixes[i] = light->lightProjectionMatrix;
		lightShadowBiases[i] = light->shadowBias;
		glActiveTexture(GL_TEXTURE0 + lightShadowMaps[i]);
		glBindTexture(GL_TEXTURE_2D, light->shadowMap->texture);
	}

	glUniform1i(glGetUniformLocation(shader->ID, "numLights"), static_cast<int>(lodLight.size()));
	glUniform3fv(glGetUniformLocation(shader->ID, "lightColors"), maxLights, glm::value_ptr(lightColors[0]));
	glUniform1iv(glGetUniformLocation(shader->ID, "lightTypes"), maxLights, lightTypes);
	glUniform1iv(glGetUniformLocation(shader->ID, "lightShadowMapSamples"), maxLights, lightShadowMaps);
	glUniformMatrix4fv(glGetUniformLocation(shader->ID, "lightProjectionMatrixes"), maxLights, GL_FALSE, glm::value_ptr(lightProjectionMatrixes[0]));
	glUniform3fv(glGetUniformLocation(shader->ID, "lightPositions"), maxLights, glm::value_ptr(lightPositions[0]));
	glUniform3fv(glGetUniformLocation(shader->ID, "lightDirections"), maxLights, glm::value_ptr(lightDirections[0]));
	glUniform1fv(glGetUniformLocation(shader->ID, "lightIntensities"), maxLights, lightIntensities);
	glUniform1fv(glGetUniformLocation(shader->ID, "lightRanges"), maxLights, lightRange);
	glUniform1fv(glGetUniformLocation(shader->ID, "lightShadowBiases"), maxLights, lightShadowBiases);
	glUniform1fv(glGetUniformLocation(shader->ID, "lightInnerConeAngles"), maxLights, lightInnerConeAngles);
	glUniform1fv(glGetUniformLocation(shader->ID, "lightOuterConeAngles"), maxLights, lightOuterConeAngles);
}

void Model::AddBasicNode() {
	Node* newNode = new Node();
	newNode->name = "Node";
	newNode->id = numNodes + 1;
	newNode->parent = root.get();
	root->addChild(std::unique_ptr<Node>(newNode));
	numNodes++;
}

void Model::AddLightNode() {
	Node* newNode = new Node();
	newNode->name = "Light";
	newNode->id = numNodes + 1;
	newNode->parent = root.get();
	Light* newLight = new Light();
	newLight->type = LIGHT_TYPE::DIRECTIONAL;
	newNode->light = newLight;
	root->addChild(std::unique_ptr<Node>(newNode));	
	lodLight.push_back(std::unique_ptr<Light>(newLight));
	numNodes++;
}

void Model::AddCameraNode() {
	Node* newNode = new Node();
	newNode->name = "Camera";
	newNode->id = numNodes + 1;
	this->mainCameraId = numNodes + 1;
	newNode->parent = root.get();
	Camera* newCamera = new Camera();
	newNode->camera = newCamera;
	root->addChild(std::unique_ptr<Node>(newNode));
	lodCamera.push_back(std::unique_ptr<Camera>(newCamera));
	numNodes++;
}

std::vector<int> Model::filterNodesOfModel(std::function<bool(int node)> func) {
	std::vector<int> nodesID;
	for (int i = 0; i < model.nodes.size(); i++) {
		if (func(i)) {
			nodesID.push_back(i);
		}
	}
	return nodesID;
}

void Model::saveCamera(){
	std::cout << "saving camera" << std::endl;
	std::function<bool(int node)> func = [](int nodeID) {
		return model.nodes[nodeID].camera != -1;
	};
	// Use filterNodesOfModel to get the nodes with a camera
	auto nodesWithCamera = filterNodesOfModel(func);

	if (nodesWithCamera.size() == 0) {
		std::cerr << "No camera found in the model." << std::endl;
		return;
	}

	// Convert the nodes matrix into an std::vector<double>
	std::vector<double> newMatrix;
	glm::mat4 mainCameraMatrix(1.0f);
	mainCameraMatrix[3] = glm::vec4(getNodeByID(mainCameraId)->camera->position, 0.0f);
	mainCameraMatrix[0][2] = -getNodeByID(mainCameraId)->camera->orientation.x;
	mainCameraMatrix[1][2] = -getNodeByID(mainCameraId)->camera->orientation.y;
	mainCameraMatrix[2][2] = -getNodeByID(mainCameraId)->camera->orientation.z;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			newMatrix.push_back(static_cast<double>(mainCameraMatrix[i][j]));
		}
	}
	model.nodes[nodesWithCamera[0]].matrix = newMatrix;
	
	// Save the gltf model
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;
	bool ret = loader.WriteGltfSceneToFile(&model, file, true, true, true, false);
	if (!ret) {
		std::cerr << "Failed to save the glTF file." << std::endl;
	}
}

void Model::renderShadowMaps(Shader* shader)
{
	skipTransparent = true;

	for (auto& light : lodLight) {
		if (light->castShadows) {
			glm::mat4 orthgonalProjection = glm::ortho(-light->orthogonalSize, light->orthogonalSize,
													   -light->orthogonalSize, light->orthogonalSize,
														light->nearPlane,      light->farPlane);
			glm::mat4 lightView = glm::lookAt(light->position * 20.0f, light->direction, glm::vec3(0.0f, 1.0f, 0.0f));
			light->lightProjectionMatrix = orthgonalProjection * lightView;

			shader->Activate();
			shader->setMat4("lightProjection", light->lightProjectionMatrix);

			glEnable(GL_DEPTH_TEST);

			light->shadowMap->Bind();

			glClear(GL_DEPTH_BUFFER_BIT);

			Draw(shader, nullptr);

			light->shadowMap->Unbind();
		}
	}

	skipTransparent = false;
}
