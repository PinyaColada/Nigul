#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <json/json.h>
#include <functional>
#include <unordered_set>
#include <bitset>

#include "Mesh.h"
#include "Material.h"
#include "camera.h"
#include "FBO.h"
#include "light.h"

#define MAX_LIGHTS 4

enum LightChangeFlags {
	Colors,
	Positions,
	Types,
	Intensities,
	Ranges,
	ShadowBiases,
	Attenuations,
	ProjectionMatrices,
	Directions,
	InnerConeAngles,
	OuterConeAngles,
	CastShadows,
	ShadowMapSamples,
	NumLightChangeFlags
};

class Node {
public:
	Node() = default;
	~Node() = default;

	Light* light = nullptr;
	Mesh* mesh = nullptr;
	Camera* camera = nullptr;
	Node* parent = nullptr;

	std::string name = "root";

	glm::mat4 matrix = glm::mat4(1.0f);
	glm::mat4 globalMatrix = glm::mat4(1.0f);

	std::vector<std::unique_ptr<Node>> children;

	int id = 0;

	Node(int id, glm::mat4 matrix, glm::mat4 globalMatrix, Node* parent = nullptr, std::string name = std::string("Node"));

	void addChild(std::unique_ptr<Node> child);
	void rewriteMatrix();
	bool isLeaf() const;
};

class Model
{
public:
	// Loads in a model from a file and stores the information in 'data', 'JSON', and 'file'
	Model();
	~Model() = default;
	void load();
	void save();

	// Variables for easy access
	std::string file;

	// Prevents textures from being loaded twice
	std::vector<std::unique_ptr<Texture>> lodTex;
	std::vector<std::unique_ptr<Material>> lodMat;
	std::vector<std::unique_ptr<Mesh>> lodMesh;
	std::vector<std::unique_ptr<Light>> lodLight;
	std::vector<std::unique_ptr<Camera>> lodCamera;

	std::unique_ptr<Node> root;

	// Loads a single mesh by its index
	void loadTextures();
	void loadMaterials();
	void loadMeshes();
	void loadLights();
	void loadCameras();

	int mainCameraId = -1;
	int nodeWithCamera = -1;
	int numNodes = 0;
	int selectedNodeId = 0;

	bool skipTransparent = false;
	bool loaded = false;

	float ambientLight = 0.05f;
	glm::vec3 ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 backgroundColor = glm::vec3(1.0f, 1.0f, 1.0f);
	float shadowDarkness = 1.0f;
	float reflectionFactor = 0.5f;

	std::vector<unsigned int> findRootNodes();
	void traverseNode(unsigned int nextNode, glm::mat4 parentMatrix = glm::mat4(1.0f), Node* parentNode = nullptr);
	void updateTreeFrom(Node* node, glm::mat4 parentMatrix);

	Node* searchNodeByID(Node* node, int targetId);
	Node* getNodeByID(int id);
	std::vector<Node*> getTree();

	void deleteNode(int id);
	void reparentNode(int id, int newParentId);

	void AddBasicNode();
	void AddLightNode(LIGHT_TYPE lightType);
	void AddCameraNode();

	std::vector<int> filterNodesOfModel(std::function<bool(int nodeID)> func);

	// Getters
	inline Camera* getMainCamera() { return lodCamera[mainCameraId].get(); }
	inline Node* getSelectedNode() { return getNodeByID(selectedNodeId); }
	inline Node* getMainCameraNode() { return getNodeByID(nodeWithCamera); }
	inline Node* getRootNode() { return root.get(); }

	// Assembles all the floats into vertices
	std::vector<Vertex> assembleVertices(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& texUVs);
	std::vector<GLuint> getIndices(int accessorIndex);
	std::vector<glm::vec2> getVec2(int accessorIndex);
	std::vector<glm::vec3> getVec3(int accessorIndex);

	// Flags for changes
	std::bitset<NumLightChangeFlags> lightFlags;
	bool hasAmbientLightChanged = true;
	bool hasAmbientColorChanged = true;
	bool hasShadowDarknessChanged = true;
	bool hasReflectionFactorChanged = true;
	bool hasSkyboxChanged = true;
};
