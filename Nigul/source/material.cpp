#include "material.h"

Material::Material(){}

void Material::bind(Shader* shader) {

	if (pbrMetallicRoughness.baseColorTexture) {
		pbrMetallicRoughness.baseColorTexture->texUnit(shader, "albedo");
		pbrMetallicRoughness.baseColorTexture->Bind();
		glUniform1i(glGetUniformLocation(shader->ID, "hasColorTexture"), 1);
	}
	else {
		glUniform1i(glGetUniformLocation(shader->ID, "hasColorTexture"), 0);
	}

	if (pbrMetallicRoughness.metallicRoughness) {
		pbrMetallicRoughness.metallicRoughness->texUnit(shader, "metallicRoughness");
		pbrMetallicRoughness.metallicRoughness->Bind();
		glUniform1i(glGetUniformLocation(shader->ID, "hasMetallicRoughnessTexture"), 1);
	}
	else {
		glUniform1i(glGetUniformLocation(shader->ID, "hasMetallicRoughnessTexture"), 0);
	}

	glUniform1f(glGetUniformLocation(shader->ID, "metallicFactor"), pbrMetallicRoughness.metallicFactor);
	glUniform1f(glGetUniformLocation(shader->ID, "roughnessFactor"), pbrMetallicRoughness.roughnessFactor);
	glUniform4f(glGetUniformLocation(shader->ID, "baseColorFactor"), pbrMetallicRoughness.baseColorFactor.x,
																	pbrMetallicRoughness.baseColorFactor.y,
																	pbrMetallicRoughness.baseColorFactor.z,
																	pbrMetallicRoughness.baseColorFactor.w);

	if (emissiveTexture) {
		emissiveTexture->texUnit(shader, "emissive");
		emissiveTexture->Bind();
		glUniform1i(glGetUniformLocation(shader->ID, "hasEmissiveTexture"), 1);
	} else {
		glUniform1i(glGetUniformLocation(shader->ID, "hasEmissiveTexture"), 0);
	}

	if (normalMap) {
		normalMap->texUnit(shader, "normalMap");
		normalMap->Bind();
		glUniform1i(glGetUniformLocation(shader->ID, "hasNormalTexture"), 1);
	}
	else {
		glUniform1i(glGetUniformLocation(shader->ID, "hasNormalTexture"), 0);
	}

	if (occlusionTexture) {
		occlusionTexture->texUnit(shader, "occlusion");
		occlusionTexture->Bind();
		glUniform1i(glGetUniformLocation(shader->ID, "hasOcclusionTexture"), 1);
	}
	else {
		glUniform1i(glGetUniformLocation(shader->ID, "hasOcclusionTexture"), 0);
	}
}