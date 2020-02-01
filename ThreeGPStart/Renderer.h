#pragma once

#include "ExternalLibraryHeaders.h"

#include "Helper.h"
#include "Mesh.h"
#include "Camera.h"
#include "ImageLoader.h"

struct MyMesh
{
	GLuint VAO;
	unsigned int numElements;
	GLuint textureID;
};

struct Object
{
	std::string texName;
	std::vector<MyMesh> myMeshVector;
};

class Renderer
{
private:

	std::vector<Object> myObjectVector;
	// Program object - to host shaders
	GLuint m_program{ 0 };

	bool CreateProgram();

public:
	Renderer()=default;
	~Renderer();

	void ModelLoader(const std::string& modelName, const std::string& textureName);

	bool CreateTerrain(int numCellsX, int numCellsZ, const std::string& textureFilename);

	void SkyboxLoader(const std::string& Name, const std::string& textureName);

	// Create and / or load geometry, this is like 'level load'
	bool InitialiseGeometry();

	// Render the scene
	void Render(const Helpers::Camera& camera, float deltaTime);
};

