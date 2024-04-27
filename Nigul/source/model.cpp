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
	if (!parent)
		return;

	this->matrix = glm::inverse(parent->globalMatrix) * this->globalMatrix;
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
			glm::vec3 cameraPos = camera->getPosition();
			float aDist = glm::length(aPos - cameraPos);
			float bDist = glm::length(bPos - cameraPos);

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
	if (node.mesh) {
		if (skipTransparent == false || (skipTransparent == true && node.mesh->material->alphaMode == OPAQUE_MODE)) {
			drawCalls.push_back(DrawCall{ node.mesh, &shader, node.globalMatrix });
		}
	}

	// Update light
	if (node.light) {
		node.light->updatePosition(node.globalMatrix);
		node.hasChanged = false;
	}

	// Update camera
	if (node.camera) {
		node.camera->updateView(node.globalMatrix);
		node.camera->updateMatrix();
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
	std::cout << node.name << std::endl;

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
			nodeMatrix *= glm::mat4_cast(-rotation);
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
		newNode->light = newLight;
	}

	if (node.camera != -1 || node.name == "Camera") {
		std::cout << "Camera found" << std::endl;
		int indexOfCamera = node.camera;
		if (lodCamera.size() == 0) {
			lodCamera.push_back(std::make_unique<PerspectiveCamera>());
			indexOfCamera = lodCamera.size() - 1;
		}
		Camera* newCamera = lodCamera[indexOfCamera].get();

		if (node.camera == 0 || node.name == "Camera")
			mainCameraId = numNodes;

		newCamera->updateView(newNode->globalMatrix);
		newCamera->updateProjection();
		newCamera->updateMatrix();
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
		std::unique_ptr<Camera> camera;

		if (model.cameras[i].type == "perspective") {
			auto perspectiveCamera = std::make_unique<PerspectiveCamera>();
			perspectiveCamera->aspectRatio = model.cameras[i].perspective.aspectRatio;
			perspectiveCamera->fov = model.cameras[i].perspective.yfov;
			perspectiveCamera->nearPlane = model.cameras[i].perspective.znear;
			perspectiveCamera->farPlane = model.cameras[i].perspective.zfar;
			camera = std::move(perspectiveCamera);
		}
		else if (model.cameras[i].type == "orthographic") {
			auto orthographicCamera = std::make_unique<OrthographicCamera>();
			orthographicCamera->size.x = model.cameras[i].orthographic.xmag;
			orthographicCamera->size.y = model.cameras[i].orthographic.ymag;
			orthographicCamera->nearPlane = model.cameras[i].orthographic.znear;
			orthographicCamera->farPlane = model.cameras[i].orthographic.zfar;
			camera = std::move(orthographicCamera);
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
		std::unique_ptr<Light> light;

		if (model.lights[i].type == "point") {
			auto pointLight = std::make_unique<PointLight>();
			light = std::move(pointLight);
		}
		else if (model.lights[i].type == "spot") {
			auto spotLight = std::make_unique<SpotLight>();
			spotLight->innerConeAngle = model.lights[i].spot.innerConeAngle;
			spotLight->outerConeAngle = model.lights[i].spot.outerConeAngle;
			spotLight->direction = glm::vec3(0.0f);
			light = std::move(spotLight);
		}
		else if (model.lights[i].type == "directional") {
			light = std::make_unique<DirectionalLight>();
		}
		else {
			std::cerr << "Unknown light type: " << model.lights[i].type << std::endl;
			throw std::runtime_error("Unsupported light type encountered");
		}

		light->color = glm::vec3(model.lights[i].color[0], model.lights[i].color[1], model.lights[i].color[2]);
		light->intensity = model.lights[i].intensity;
		light->range = model.lights[i].range;

		lodLight.push_back(std::move(light));
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
	const int maxLights = 4; 
	int indexTexture = lodTex.size() ;
	glm::vec3 lightColors[maxLights];
	int lightTypes[maxLights];
	int lightShadowMaps[maxLights];
	glm::vec3 lightPositions[maxLights];
	glm::vec3 lightDirections[maxLights];
	bool lightCastShadows[maxLights];
	float lightIntensities[maxLights];
	float lightRanges[maxLights];
	float lightInnerConeAngles[maxLights];
	float lightOuterConeAngles[maxLights];
	float lightShadowBiases[maxLights];
	float lightAttenuations[maxLights];
	glm::mat4 lightProjectionMatrixes[maxLights];

	for (size_t i = 0; i < lodLight.size() && i < maxLights; i++) {
		auto light = lodLight[i].get();
		lightColors[i] = light->color;
		lightTypes[i] = static_cast<int>(light->getType());
		lightIntensities[i] = light->intensity;
		lightPositions[i] = light->position;
		lightRanges[i] = light->range;
		lightShadowMaps[i] = indexTexture + i;
		lightShadowBiases[i] = light->shadowBias;
		lightCastShadows[i] = light->castShadows;

		if (light->getType() == POINTLIGHT) {
			lightAttenuations[i] = dynamic_cast<PointLight*>(light)->attenuation;
		}
		else {
			lightAttenuations[i] = 0.0f;
		}

		if (light->getType() == SPOTLIGHT) {
			lightDirections[i] = dynamic_cast<SpotLight*>(light)->direction;
			lightInnerConeAngles[i] = dynamic_cast<SpotLight*>(light)->innerConeAngle;
			lightOuterConeAngles[i] = dynamic_cast<SpotLight*>(light)->outerConeAngle;
		}
		else {
			lightDirections[i] = glm::vec3(0.0f);
			lightInnerConeAngles[i] = 0.0f;
			lightOuterConeAngles[i] = 0.0f;
		}

		if (light->getType() == DIRECTIONAL) {
			lightProjectionMatrixes[i] = dynamic_cast<DirectionalLight*>(light)->camera.cameraMatrix;
			glActiveTexture(GL_TEXTURE0 + lightShadowMaps[i]);
			glBindTexture(GL_TEXTURE_2D, light->shadowMap->texture);
		}
		else {
			lightProjectionMatrixes[i] = glm::mat4(1.0f);
		}
	}

	shader->setSizeT("numLights", lodLight.size());
	shader->setVecs3("lightColors", lightColors, maxLights);
	shader->setInts("lightTypes", lightTypes, maxLights);
	shader->setBools("lightCastShadows", lightCastShadows, maxLights);
	shader->setInts("lightShadowMapSamples", lightShadowMaps, maxLights);
	shader->setMats4("lightProjectionMatrixes", lightProjectionMatrixes, maxLights);
	shader->setVecs3("lightPositions", lightPositions, maxLights);
	shader->setVecs3("lightDirections", lightDirections, maxLights);
	shader->setFloats("lightAttenuations", lightAttenuations, maxLights);
	shader->setFloats("lightIntensities", lightIntensities, maxLights);
	shader->setFloats("lightRanges", lightRanges, maxLights);
	shader->setFloats("lightShadowBiases", lightShadowBiases, maxLights);
	shader->setFloats("lightInnerConeAngles", lightInnerConeAngles, maxLights);
	shader->setFloats("lightOuterConeAngles", lightOuterConeAngles, maxLights);
}

void Model::AddBasicNode() {
	Node* newNode = new Node();
	newNode->name = "Node";
	newNode->id = numNodes + 1;
	newNode->parent = root.get();
	root->addChild(std::unique_ptr<Node>(newNode));
	numNodes++;
}

void Model::AddDirectionalLightNode() {
	auto newNode = std::make_unique<Node>();
	newNode->name = "Directional Light";
	newNode->id = numNodes + 1;
	newNode->parent = root.get();
	auto newLight = std::make_unique<DirectionalLight>();
	newNode->light = newLight.get();
	root->addChild(std::move(newNode));
	lodLight.push_back(std::move(newLight));
	numNodes++;
}

void Model::AddPointLightNode() {
	auto newNode = std::make_unique<Node>();
	newNode->name = "Point Light";
	newNode->id = numNodes + 1;
	newNode->parent = root.get();
	auto newLight = std::make_unique<PointLight>();
	newNode->light = newLight.get();
	root->addChild(std::move(newNode));
	lodLight.push_back(std::move(newLight));
	numNodes++;
}

void Model::AddSpotLightNode() {
	auto newNode = std::make_unique<Node>();
	newNode->name = "Spot Light";
	newNode->id = numNodes + 1;
	newNode->parent = root.get();
	auto newLight = std::make_unique<SpotLight>();
	newNode->light = newLight.get();
	root->addChild(std::move(newNode));
	lodLight.push_back(std::move(newLight));
	numNodes++;
}

void Model::AddCameraNode() {
	Node* newNode = new Node();
	newNode->name = "Camera";
	newNode->id = numNodes + 1;
	this->mainCameraId = numNodes + 1;
	newNode->parent = root.get();
	Camera* newCamera = new PerspectiveCamera();
	newCamera->updateView(newNode->globalMatrix);
	newCamera->updateProjection();
	newCamera->updateMatrix();
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

void Model::renderShadowMaps(Shader* shader)
{
	skipTransparent = true;

	for (auto& light : lodLight) {
		if (light->getType() != DIRECTIONAL)
			continue;

		DirectionalLight* direct = static_cast<DirectionalLight*>(light.get());

		direct->updateProjection();

		shader->Activate();
		shader->setMat4("lightProjection", direct->camera.cameraMatrix);

		glEnable(GL_DEPTH_TEST);

		direct->shadowMap->Bind();

		glClear(GL_DEPTH_BUFFER_BIT);

		if (light->castShadows)
			Draw(shader, nullptr);

		direct->shadowMap->Unbind();
	}

	skipTransparent = false;
}
