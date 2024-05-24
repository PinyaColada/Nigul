#include "renderer.h"

Renderer::Renderer(int width, int height) {
	shaderMap["default"] = std::make_unique<Shader>("default.vert", "default.frag");
	shaderMap["skybox"] = std::make_unique<Shader>("skybox.vert", "skybox.frag");
	shaderMap["shadow"] = std::make_unique<Shader>("shadow.vert", "shadow.frag");

	postpo = std::make_unique<FXQuad>(width, height);
}

void Renderer::render(Model* model, Skybox* skybox) {
	if (!model)
		return;

	Camera* camera = model->getMainCamera();
	Shader* defaultShader = shaderMap["default"].get();
	Shader* skyboxShader = shaderMap["skybox"].get();
	FXQuad* FX = this->postpo.get();

	renderShadowMap(model);
	setAllUniforms(model, skybox, defaultShader);

	FX->fbo->bind();

	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderModel(model, defaultShader, camera);
	renderSkybox(skybox, skyboxShader, camera);

	FX->fbo->unbind();

	render(FX);
}

void Renderer::renderModel(Model* model, Shader* shader, Camera* camera) {
	Node* root = model->root.get();

	for (auto& renderCall : getRenderCalls(root, shader, camera)) {
		render(renderCall);
	}
}

void Renderer::render(renderCall call) {
	call.shader->activate();
	call.mesh->VAO.Bind();

	if (call.mesh->material) {
		call.mesh->material->bind(call.shader);
	}

	if (call.camera) {
		call.shader->setVec3("camPos", call.camera->getPosition());
		call.shader->setMat4("camMatrix", call.camera->cameraMatrix);
	}

	call.shader->setMat4("model", call.matrix);

	if (call.mesh->material) {
		if (call.mesh->material->doubleSided) {
			glDisable(GL_CULL_FACE);
		}
		if (call.mesh->material->alphaMode == BLEND_MODE) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	}

	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS);

	glDrawElements(GL_TRIANGLES, call.mesh->indices.size(), GL_UNSIGNED_INT, 0);

	if (call.mesh->material) {
		if (call.mesh->material->doubleSided) {
			glEnable(GL_CULL_FACE);
		}
		if (call.mesh->material->alphaMode == BLEND_MODE) {
			glDisable(GL_BLEND);
		}
	}
}

void Renderer::render(FXQuad* fxQuad) {
	fxQuad->passUniforms();
	fxQuad->toViewport();
}

std::vector<renderCall> Renderer::getRenderCalls(Node* node, Shader* shader, Camera* camera) {
	std::vector<renderCall> renderCalls;

	if (node->mesh) {
		renderCalls.push_back(renderCall{ node->mesh, shader, camera, node->globalMatrix });
	}

	for (auto& child : node->children) {
		auto childCalls = getRenderCalls(child.get(), shader, camera);
		renderCalls.insert(renderCalls.end(), childCalls.begin(), childCalls.end());
	}

	return renderCalls;
}

void Renderer::renderSkybox(Skybox* skybox, Shader* shader, Camera* camera) {
	if (skybox) {
		shader->activate();

		glDepthFunc(GL_LEQUAL);
		glBindVertexArray(skybox->VAO);

		glm::mat4 view = glm::mat4(glm::mat3(camera->viewMatrix));

		shader->setMat4("view", camera->viewMatrix);
		shader->setMat4("view", view);
		shader->setMat4("projection", camera->projectionMatrix);

		shader->setSkybox("skybox", *skybox);

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
	}
}

void Renderer::setLightColorsUniform(Shader* shader, Model* model) {
	if (!model->lightFlags[Colors])
		return;
	glm::vec3 lightColors[MAX_LIGHTS];
	for (int i = 0; i < model->lodLight.size(); i++) {
		auto light = model->lodLight[i].get();
		lightColors[i] = light->color;
	}
	shader->activate();
	shader->setVecs3("lightColors", lightColors, MAX_LIGHTS);
	model->lightFlags[Colors] = false;
}

void Renderer::setLightPositionsUniform(Shader* shader, Model* model) {
	if (!model->lightFlags[Positions])
		return;
	glm::vec3 lightPositions[MAX_LIGHTS];
	for (int i = 0; i < model->lodLight.size(); i++) {
		auto light = model->lodLight[i].get();
		lightPositions[i] = light->position;
	}
	shader->activate();
	shader->setVecs3("lightPositions", lightPositions, MAX_LIGHTS);
	model->lightFlags[Positions] = false;
}

void Renderer::setLightTypesUniform(Shader* shader, Model* model) {
	if (!model->lightFlags[Types])
		return;
	int lightTypes[MAX_LIGHTS];
	for (int i = 0; i < model->lodLight.size(); i++) {
		auto light = model->lodLight[i].get();
		lightTypes[i] = static_cast<int>(light->getType());
		std::cout << "Light types set: " << lightTypes[i] << std::endl;
	}
	shader->activate();
	shader->setInts("lightTypes", lightTypes, MAX_LIGHTS);
	shader->setInt("numLights", model->lodLight.size());
	model->lightFlags[Types] = false;
}

void Renderer::setLightIntensitiesUniform(Shader* shader, Model* model) {
	if (!model->lightFlags[Intensities])
		return;
	float lightIntensities[MAX_LIGHTS];
	for (int i = 0; i < model->lodLight.size(); i++) {
		auto light = model->lodLight[i].get();
		lightIntensities[i] = light->intensity;
	}
	shader->activate();
	shader->setFloats("lightIntensities", lightIntensities, MAX_LIGHTS);
	model->lightFlags[Intensities] = false;
}

void Renderer::setLightRangesUniform(Shader* shader, Model* model) {
	if (!model->lightFlags[Ranges])
		return;
	float lightRanges[MAX_LIGHTS];
	for (int i = 0; i < model->lodLight.size(); i++) {
		auto light = model->lodLight[i].get();
		lightRanges[i] = light->range;
	}
	shader->activate();
	shader->setFloats("lightRanges", lightRanges, MAX_LIGHTS);
	model->lightFlags[Ranges] = false;
}

void Renderer::setLightShadowBiasesUniform(Shader* shader, Model* model) {
	if (!model->lightFlags[ShadowBiases])
		return;
	float lightShadowBiases[MAX_LIGHTS];
	for (int i = 0; i < model->lodLight.size(); i++) {
		auto light = model->lodLight[i].get();
		lightShadowBiases[i] = light->shadowBias;
	}
	shader->activate();
	shader->setFloats("lightShadowBiases", lightShadowBiases, MAX_LIGHTS);
	model->lightFlags[ShadowBiases] = false;
}

void Renderer::setLightAttenuationsUniform(Shader* shader, Model* model) {
	if (!model->lightFlags[Attenuations])
		return;
	float lightAttenuations[MAX_LIGHTS];
	for (int i = 0; i < model->lodLight.size(); i++) {
		auto light = model->lodLight[i].get();
		if (light->getType() == POINTLIGHT) {
			lightAttenuations[i] = dynamic_cast<PointLight*>(light)->attenuation;
		}
		else {
			lightAttenuations[i] = 0.0f;
		}
	}
	shader->activate();
	shader->setFloats("lightAttenuations", lightAttenuations, MAX_LIGHTS);
	model->lightFlags[Attenuations] = false;
}

void Renderer::setLightProjectionMatricesUniform(Shader* shader, Model* model) {
	if (!model->lightFlags[ProjectionMatrices])
		return;
	glm::mat4 lightProjectionMatrixes[MAX_LIGHTS];
	for (int i = 0; i < model->lodLight.size(); i++) {
		auto light = model->lodLight[i].get();
		if (light->getType() == DIRECTIONAL) {
			lightProjectionMatrixes[i] = dynamic_cast<DirectionalLight*>(light)->camera->cameraMatrix;
		}
		else {
			lightProjectionMatrixes[i] = glm::mat4(1.0f);
		}
	}
	shader->activate();
	shader->setMats4("lightProjectionMatrixes", lightProjectionMatrixes, MAX_LIGHTS);
	model->lightFlags[ProjectionMatrices] = false;
}

void Renderer::setLightDirectionsUniform(Shader* shader, Model* model) {
	if (!model->lightFlags[Directions])
		return;
	glm::vec3 lightDirections[MAX_LIGHTS];
	for (int i = 0; i < model->lodLight.size(); i++) {
		auto light = model->lodLight[i].get();
		if (light->getType() == SPOTLIGHT) {
			lightDirections[i] = dynamic_cast<SpotLight*>(light)->direction;
		}
		else {
			lightDirections[i] = glm::vec3(0.0f);
		}
	}
	shader->activate();
	shader->setVecs3("lightDirections", lightDirections, MAX_LIGHTS);
	model->lightFlags[Directions] = false;
}

void Renderer::setLightInnerConeAnglesUniform(Shader* shader, Model* model) {
	if (!model->lightFlags[InnerConeAngles])
		return;
	float lightInnerConeAngles[MAX_LIGHTS];
	for (int i = 0; i < model->lodLight.size(); i++) {
		auto light = model->lodLight[i].get();
		if (light->getType() == SPOTLIGHT) {
			lightInnerConeAngles[i] = dynamic_cast<SpotLight*>(light)->innerConeAngle;
		}
		else {
			lightInnerConeAngles[i] = 0.0f;
		}
	}
	shader->activate();
	shader->setFloats("lightInnerConeAngles", lightInnerConeAngles, MAX_LIGHTS);
	model->lightFlags[InnerConeAngles] = false;
}

void Renderer::setLightOuterConeAnglesUniform(Shader* shader, Model* model) {
	if (!model->lightFlags[OuterConeAngles])
		return;
	float lightOuterConeAngles[MAX_LIGHTS];
	for (int i = 0; i < model->lodLight.size(); i++) {
		auto light = model->lodLight[i].get();
		if (light->getType() == SPOTLIGHT) {
			lightOuterConeAngles[i] = dynamic_cast<SpotLight*>(light)->outerConeAngle;
		}
		else {
			lightOuterConeAngles[i] = 0.0f;
		}
	}
	shader->activate();
	shader->setFloats("lightOuterConeAngles", lightOuterConeAngles, MAX_LIGHTS);
	model->lightFlags[OuterConeAngles] = false;
}

void Renderer::setLightCastShadowsUniform(Shader* shader, Model* model) {
	if (!model->lightFlags[CastShadows])
		return;
	int lightCastShadows[MAX_LIGHTS];
	for (int i = 0; i < model->lodLight.size(); i++) {
		auto light = model->lodLight[i].get();
		lightCastShadows[i] = light->castShadows;
	}
	shader->activate();
	shader->setInts("lightCastShadows", lightCastShadows, MAX_LIGHTS);
	model->lightFlags[CastShadows] = false;
}

void Renderer::setLightShadowMapSamplesUniform(Shader* shader, Model* model) {
	if (!model->lightFlags[ShadowMapSamples])
		return;
	int indexTexture = model->lodTex.size();
	int lightShadowMapSamples[MAX_LIGHTS] = { indexTexture, indexTexture + 1, indexTexture + 2, indexTexture + 3 };
	shader->activate();
	shader->setInts("lightShadowMapSamples", lightShadowMapSamples, MAX_LIGHTS);
	for (int i = 0; i < model->lodLight.size(); i++) {
		auto light = model->lodLight[i].get();
		light->shadowMap->tex->bind();
	}
	model->lightFlags[ShadowMapSamples] = false;
}

void Renderer::setAmbientColorUniform(Shader* shader, Model* model) {
	if (!model->hasAmbientColorChanged)
		return;
	shader->activate();
	shader->setVec3("ambientColor", model->ambientColor);
	model->hasAmbientColorChanged = false;
}

void Renderer::setAmbientLightUniform(Shader* shader, Model* model) {
	if (!model->hasAmbientLightChanged)
		return;
	shader->activate();
	shader->setFloat("ambientLight", model->ambientLight);
	model->hasAmbientLightChanged = false;
}

void Renderer::setShadowDarknessUniform(Shader* shader, Model* model) {
	if (!model->hasShadowDarknessChanged)
		return;
	shader->activate();
	shader->setFloat("shadowDarkness", model->shadowDarkness);
	model->hasShadowDarknessChanged = false;
}

void Renderer::setReflectionFactorUniform(Shader* shader, Model* model) {
	if (!model->hasReflectionFactorChanged)
		return;
	shader->activate();
	shader->setFloat("reflectionFactor", model->reflectionFactor);
	model->hasReflectionFactorChanged = false;
}

void Renderer::setSkyboxUniforms(Shader* shader, Model* model, Skybox* skybox) {
	if (!model->hasSkyboxChanged)
		return;
	bool hasSkybox = (skybox != nullptr);
	shader->activate();
	shader->setBool("hasSkybox", hasSkybox);
	if (hasSkybox) {
		shader->setSkybox("skybox", *skybox);
	}
	model->hasSkyboxChanged = false;
}

void Renderer::setAllUniforms(Model* model, Skybox* skybox, Shader* shader) {
	setLightColorsUniform(shader, model);
	setLightPositionsUniform(shader, model);
	setLightTypesUniform(shader, model);
	setLightIntensitiesUniform(shader, model);
	setLightRangesUniform(shader, model);
	setLightShadowBiasesUniform(shader, model);
	setLightAttenuationsUniform(shader, model);
	setLightProjectionMatricesUniform(shader, model);
	setLightDirectionsUniform(shader, model);
	setLightInnerConeAnglesUniform(shader, model);
	setLightOuterConeAnglesUniform(shader, model);
	setLightCastShadowsUniform(shader, model);
	setLightShadowMapSamplesUniform(shader, model);
	setAmbientLightUniform(shader, model);
	setAmbientColorUniform(shader, model);
	setShadowDarknessUniform(shader, model);
	setReflectionFactorUniform(shader, model);
	setSkyboxUniforms(shader, model, skybox);
}

void Renderer::renderShadowMap(Model* model) {
	if (!model->lightFlags[ProjectionMatrices])
		return;

	for (auto& light : model->lodLight) {
		if (light->getType() != DIRECTIONAL)
			return;

		glEnable(GL_DEPTH_TEST);

		light->shadowMap->bind();

		glClear(GL_DEPTH_BUFFER_BIT);
		if (light->castShadows)
			renderModel(model, shaderMap["shadow"].get(), light->camera);

		light->shadowMap->unbind();
	}
}