#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 ***********************************************************/
SceneManager::~SceneManager()
{
	DestroyGLTextures();
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	stbi_set_flip_vertically_on_load(true);

	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	if (image)
	{
		std::cout << "Successfully loaded image:" << filename
			<< ", width:" << width
			<< ", height:" << height
			<< ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;
	return false;
}

/***********************************************************
 *  BindGLTextures()
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glDeleteTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return textureID;
}

/***********************************************************
 *  FindTextureSlot()
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return textureSlot;
}

/***********************************************************
 *  FindMaterial()
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return false;
	}

	int index = 0;
	bool bFound = false;
	while ((index < (int)m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return bFound;
}

/***********************************************************
 *  SetTransformations()
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	scale = glm::scale(scaleXYZ);
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 ***********************************************************/
void SceneManager::SetShaderTexture(std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		int textureSlot = FindTextureSlot(textureTag);

		if (textureSlot < 0)
		{
			m_pShaderManager->setIntValue(g_UseTextureName, false);
			m_pShaderManager->setVec4Value(g_ColorValueName, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
			return;
		}

		m_pShaderManager->setIntValue(g_UseTextureName, true);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureSlot);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 ***********************************************************/
void SceneManager::SetShaderMaterial(std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/***********************************************************
 *  DefineObjectMaterials()
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	OBJECT_MATERIAL floorMaterial;
	floorMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	floorMaterial.ambientStrength = 0.2f;
	floorMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	floorMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	floorMaterial.shininess = 0.5f;
	floorMaterial.tag = "floor";
	m_objectMaterials.push_back(floorMaterial);

	OBJECT_MATERIAL woodMaterial;
	woodMaterial.ambientColor = glm::vec3(0.4f, 0.3f, 0.1f);
	woodMaterial.ambientStrength = 0.2f;
	woodMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
	woodMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	woodMaterial.shininess = 0.3f;
	woodMaterial.tag = "wood";
	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL coverMaterial;
	coverMaterial.ambientColor = glm::vec3(0.2f, 0.3f, 0.4f);
	coverMaterial.ambientStrength = 0.3f;
	coverMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	coverMaterial.specularColor = glm::vec3(0.4f, 0.5f, 0.6f);
	coverMaterial.shininess = 25.0f;
	coverMaterial.tag = "cover";
	m_objectMaterials.push_back(coverMaterial);

	OBJECT_MATERIAL paperMaterial;
	paperMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	paperMaterial.ambientStrength = 0.3f;
	paperMaterial.diffuseColor = glm::vec3(0.8f, 0.8f, 0.8f);
	paperMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	paperMaterial.shininess = 6.0f;
	paperMaterial.tag = "paper";
	m_objectMaterials.push_back(paperMaterial);

	OBJECT_MATERIAL metalMaterial;
	metalMaterial.ambientColor = glm::vec3(0.3f, 0.3f, 0.35f);
	metalMaterial.ambientStrength = 0.3f;
	metalMaterial.diffuseColor = glm::vec3(0.6f, 0.6f, 0.65f);
	metalMaterial.specularColor = glm::vec3(0.9f, 0.9f, 0.95f);
	metalMaterial.shininess = 64.0f;
	metalMaterial.tag = "metal";
	m_objectMaterials.push_back(metalMaterial);

	OBJECT_MATERIAL globeMaterial;
	globeMaterial.ambientColor = glm::vec3(0.1f, 0.2f, 0.35f);
	globeMaterial.ambientStrength = 0.25f;
	globeMaterial.diffuseColor = glm::vec3(0.2f, 0.45f, 0.7f);
	globeMaterial.specularColor = glm::vec3(0.8f, 0.85f, 0.9f);
	globeMaterial.shininess = 96.0f;
	globeMaterial.tag = "globe";
	m_objectMaterials.push_back(globeMaterial);
}

/***********************************************************
 *  SetupSceneLights()
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	m_pShaderManager->setVec3Value("lightSources[0].position", 4.0f, 10.0f, 5.0f);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.05f, 0.03f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.9f, 0.65f, 0.25f);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 0.8f, 0.7f, 0.4f);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.6f);

	m_pShaderManager->setVec3Value("lightSources[1].position", -5.0f, 8.0f, 3.0f);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.01f, 0.01f, 0.03f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.25f, 0.35f, 0.7f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.2f, 0.25f, 0.5f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 24.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.3f);

	m_pShaderManager->setVec3Value("lightSources[2].position", 0.0f, 14.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.02f, 0.02f, 0.02f);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.4f, 0.4f, 0.4f);
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", 0.3f, 0.3f, 0.3f);
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 18.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.2f);

	m_pShaderManager->setVec3Value("lightSources[3].position", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[3].ambientColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[3].specularColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 1.0f);
	m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.0f);

	m_pShaderManager->setBoolValue(g_UseLightingName, true);
}

/**************************************************************
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

/***********************************************************
 *  PrepareScene()
 ***********************************************************/
void SceneManager::PrepareScene()
{
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadConeMesh();

	DefineObjectMaterials();
	SetupSceneLights();

	CreateGLTexture("Debug/Textures/drywall.jpg", "floorTex");
	CreateGLTexture("Debug/Textures/rusticwood.jpg", "woodTex");
	CreateGLTexture("Debug/Textures/backdrop.jpg", "coverTex");
	CreateGLTexture("Debug/Textures/drywall.jpg", "paperTex");

	BindGLTextures();
}

/***********************************************************
 *  RenderScene()
 ***********************************************************/
void SceneManager::RenderScene()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("floorTex");
	SetTextureUVScale(8.0f, 8.0f);
	SetShaderMaterial("floor");
	m_basicMeshes->DrawPlaneMesh();

	scaleXYZ = glm::vec3(4.0f, 0.2f, 2.5f);
	positionXYZ = glm::vec3(0.0f, 1.5f, 0.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("woodTex");
	SetTextureUVScale(2.0f, 2.0f);
	SetShaderMaterial("wood");
	m_basicMeshes->DrawBoxMesh();

	scaleXYZ = glm::vec3(0.12f, 1.5f, 0.12f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	SetShaderTexture("woodTex");
	SetTextureUVScale(1.0f, 3.0f);
	SetShaderMaterial("wood");

	positionXYZ = glm::vec3(-1.7f, 0.75f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();

	positionXYZ = glm::vec3(1.7f, 0.75f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();

	positionXYZ = glm::vec3(-1.7f, 0.75f, -1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();

	positionXYZ = glm::vec3(1.7f, 0.75f, -1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();

	scaleXYZ = glm::vec3(1.2f, 0.06f, 0.9f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.4f, 1.63f, 0.1f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("coverTex");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("cover");
	m_basicMeshes->DrawBoxMesh();

	scaleXYZ = glm::vec3(1.05f, 0.03f, 0.78f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.42f, 1.68f, 0.1f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("paperTex");
	SetTextureUVScale(4.0f, 4.0f);
	SetShaderMaterial("paper");
	m_basicMeshes->DrawBoxMesh();

	scaleXYZ = glm::vec3(0.04f, 0.55f, 0.04f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 110.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-0.5f, 1.62f, 0.3f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.72f, 0.72f, 0.75f, 1.0f);
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	scaleXYZ = glm::vec3(0.04f, 0.08f, 0.04f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 110.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(
		-0.5f + 0.52f * glm::cos(glm::radians(110.0f)),
		1.62f,
		-0.52f * glm::sin(glm::radians(110.0f)) + 0.3f
	);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.18f, 0.18f, 0.18f, 1.0f);
	SetShaderMaterial("metal");
	m_basicMeshes->DrawConeMesh();

	scaleXYZ = glm::vec3(0.3f, 0.3f, 0.3f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-1.2f, 1.92f, 0.0f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.2f, 0.45f, 0.75f, 1.0f);
	SetShaderMaterial("globe");
	m_basicMeshes->DrawSphereMesh();
}