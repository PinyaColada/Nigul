#pragma once
#include<json/json.h>
#include <unordered_set>
#include <optional>
#include <functional>

#include "Mesh.h"
#include "Material.h"
#include "camera.h"
#include "FBO.h"

enum LIGHT_TYPE {
	POINTLIGHT = 0,
	SPOTLIGHT = 1,
	DIRECTIONAL = 2
};

struct Light {
	glm::vec3 color;
	float intensity = 1.0;
	LIGHT_TYPE type;
	float range = 3.0;
	float innerConeAngle = 0.9;
	float outerConeAngle = 0.95;

	bool castShadows = true;
	std::unique_ptr<FBO> shadowMap = std::make_unique<FBO>(8192, 8192, true);
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
	float orthogonalSize = 5.0f;
	float shadowBias = 0.00001f;
	glm::mat4 lightProjectionMatrix = glm::mat4(1.0f);

	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 direction = glm::vec3(0.0f);
	glm::vec3 lookAt = glm::vec3(0.0f);
	Light();
};

class Node {
public:
	Light* light = nullptr;
	Mesh* mesh = nullptr;
	Camera* camera = nullptr;
	Node* parent = nullptr;

	std::string name;
	glm::mat4 matrix = glm::mat4(1.0f);
	glm::mat4 globalMatrix = glm::mat4(1.0f);
	std::vector<std::unique_ptr<Node>> children;
	int id;

	Node();
	Node(int id, glm::mat4 matrix, glm::mat4 globalMatrix, Node* parent = nullptr, std::string name = std::string("Node"));

	void addChild(std::unique_ptr<Node> child);
	void rewriteMatrix();
	bool isLeaf() const;
};

struct DrawCall {
	Mesh* mesh;
	Shader* shader;
	glm::mat4 globalMatrix;

	void call(Camera* camera) {
		mesh->Draw(shader, camera, globalMatrix);
	}
};

class Model
{
public:
	// Loads in a model from a file and stores the information in 'data', 'JSON', and 'file'
	Model();
	void load();

	void Draw(Shader* shader, Camera* camera);
	void passLightUniforms(Shader* shader);
	void renderShadowMaps(Shader* shader);

	// Variables for easy access
	std::string file;

	// Prevents textures from being loaded twice
	std::vector<std::unique_ptr<Texture>> lodTex;
	std::vector<std::unique_ptr<Material>> lodMat;
	std::vector<std::unique_ptr<Mesh>> lodMesh;
	std::vector<std::unique_ptr<Light>> lodLight;
	std::vector<std::unique_ptr<Camera>> lodCamera;

	std::vector<DrawCall> drawCalls;

	std::unique_ptr<Node> root;

	// Loads a single mesh by its index
	void loadTextures();
	void loadMaterials();
	void loadMeshes();
	void loadLights();
	void loadCameras();

	int mainCameraId = -1;
	int numNodes = 0;
	int numTextures = 0;
	bool skipTransparent = false;

	std::vector<unsigned int> findRootNodes();

	// Traverses a node recursively, so it essentially traverses all connected nodes
	void traverseNode(unsigned int nextNode, glm::mat4 parentMatrix = glm::mat4(1.0f), Node* parentNode = nullptr);
	void updateTree(Node& node, Shader& shader, glm::mat4 parentMatrix);

	Node* searchNodeByID(Node* node, int targetId);
	Node* getNodeByID(int id);

	// Destructor
	void deleteNode(int id);
	void reparentNode(int id, int newParentId);

	void AddBasicNode();
	void AddLightNode();
	void AddCameraNode();
	std::vector<int> filterNodesOfModel(std::function<bool(int nodeID)> func);

	void saveCamera();

	// Assembles all the floats into vertices
	std::vector<Vertex> assembleVertices(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& texUVs);

	std::vector<GLuint> getIndices(int accessorIndex);
	std::vector<glm::vec2> getVec2(int accessorIndex);
	std::vector<glm::vec3> getVec3(int accessorIndex);
};
