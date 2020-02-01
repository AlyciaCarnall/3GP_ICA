#include "Renderer.h"


// On exit must clean up any OpenGL resources e.g. the program, the buffers
Renderer::~Renderer()
{
	glDeleteProgram(m_program);	
	
}

// Load, compile and link the shaders and create a program object to host them
bool Renderer::CreateProgram()
{
	// Create a new program (returns a unqiue id)
	m_program = glCreateProgram();

	// Load and create vertex and fragment shaders
	GLuint vertex_shader{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/vertex_shader.glsl") };
	GLuint fragment_shader{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/fragment_shader.glsl") };
	if (vertex_shader == 0 || fragment_shader == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(m_program, vertex_shader);

	// The attibute 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself
	//glBindAttribLocation(m_program, 0, "vertex_position");

	// Attach the fragment shader (copies it)
	glAttachShader(m_program, fragment_shader);

	// Done with the originals of these as we have made copies
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(m_program))
		return false;

	return !Helpers::CheckForGLError();
}

void Renderer::ModelLoader(const std::string& modelName, const std::string& textureName)
{
	Object jeep;

	Helpers::ModelLoader jeepModel;
	if (!jeepModel.LoadFromFile(modelName))
		std::cerr << "Could not load model" << std::endl;

	Helpers::ImageLoader jeepTexture;
	if (!jeepTexture.Load(textureName))
		std::cerr << "Could not load model" << std::endl;

	for (const Helpers::Mesh& mesh : jeepModel.GetMeshVector())
	{
		MyMesh modelMesh;
		//create vbo s
		
		GLuint normalsVBO;
		GLuint elementsEBO;
		GLuint texcoordVBO;

		GLuint positionsVBO;
		//Generate 1 buffer id, put the resulting identifier in positionsVBO variable.
		glGenBuffers(1, &positionsVBO);

		//Bind the buffer to the context at the GL_ARRAY_BUFFER binding point (target).
		//This is the first time the buffer is used so this also createsthe buffer object.
		glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);
		
		//Fill the bound buffer with the vertices, we pass the size in bytes and a pointer to the data.
		//the last parameter is a hint to open GL that we will not alter the vertices again.
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);
		
		//Clear binding - not absolutely required but a good idea!
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &normalsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.normals.size(), mesh.normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &elementsEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.elements.size(), mesh.elements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenBuffers(1, &texcoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texcoordVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.uvCoords.size(), mesh.uvCoords.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		modelMesh.numElements = (GLuint)mesh.elements.size();

		/*	Create a Vertex Array Object (VAO) to wrap or 'record' all bindings etc. needed to render
			As well as the make up of any streamed data
			Once we bind the VAO subsequent binds etc. are 'recorded' in the VAO.*/

		// Create a unique id for a vertex array object
		glGenVertexArrays(1, &modelMesh.VAO);

		// Bind it to be current and since first use this also allocates memory for it
		// Note no target binding point as there is only one type of vao
		glBindVertexArray(modelMesh.VAO);

		// Bind the vertex buffer to the context (records this action in the VAO)
		glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);

		// Enable the first attribute in the program (the vertices) to stream to the vertex shader
		glEnableVertexAttribArray(0);

		// Describe the make up of the vertex stream
		glVertexAttribPointer(
			0,                  // attribute 0
			3,                  // size in bytes of each item in the stream
			GL_FLOAT,           // type of the item
			GL_FALSE,           // normalized or not (advanced)
			0,                  // stride (advanced)
			(void*)0            // array buffer offset (advanced)
		);

		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		glBindBuffer(GL_ARRAY_BUFFER, texcoordVBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2,
			2,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		glGenTextures(1, &modelMesh.textureID);
		glBindTexture(GL_TEXTURE_2D, modelMesh.textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, jeepTexture.Width(), jeepTexture.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, jeepTexture.GetData());
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsEBO);
		glBindVertexArray(0);

		jeep.myMeshVector.push_back(modelMesh);
	}

	myObjectVector.push_back(jeep);

}

bool Renderer::CreateTerrain(int numCellsX, int numCellsZ, const std::string& textureFilename)
{
	Object terrain;
	terrain.texName = textureFilename;
	MyMesh terrainMesh;
	float terrainScale{ 100};
	int numVertX = numCellsX + 1;
	int numVertZ = numCellsZ + 1;

	Helpers::ImageLoader terrainTexture;
	if (!terrainTexture.Load(textureFilename))
		std::cerr << "Could not load model" << std::endl;

	GLbyte* terrainData = terrainTexture.GetData();

	//Generate verticies
	std::vector < glm::vec3 > terrainVertices;

	glm::vec3 start((numCellsX * terrainScale) / 2, 0 , (numCellsZ * terrainScale) / 2);
	   	  
	//Texture Coordinates
	std::vector <glm::vec2> uvCoords;

	Helpers::ImageLoader heightMap;
	if (!heightMap.Load("Data\\Terrain\\curvy.gif"))
		std::cerr << "Could not load height map" << std::endl;

	unsigned char* texels = (unsigned char*)heightMap.GetData();

	for (int z{ 0 }; z < numCellsZ + 1; ++z)
	{
	for (int x{ 0 }; x < numCellsX + 1; ++x)
	{
		
			glm::vec3 pos{ start };

			pos.x = (x * terrainScale - start.x);
			pos.z = start.z - (z * terrainScale );

			float u = (float)x / (numVertX - 1);
			float v = (float)z / (numVertZ - 1);

			int heightMapX = (int)(u * (heightMap.Width() - 1));
			int heightMapY = (int)(v * (heightMap.Height() - 1));

			int offset = (heightMapX + heightMapY * heightMap.Width()) * 4;
			pos.y = texels[offset];

			terrainVertices.push_back(pos);
			uvCoords.push_back(glm::vec2(u, v));
		}
	}

	//Indicies generation
	std::vector <GLint> terrainElements;

	bool DiamondPattern = true;

	for (int z{ 0 }; z < numCellsX; ++z)
	{
		for (int x{ 0 }; x < numCellsZ; ++x)
		{
			int startVertIndex = z * numVertX + x;
			if (DiamondPattern)
			{
				terrainElements.push_back(startVertIndex);
				terrainElements.push_back(startVertIndex + 1);
				terrainElements.push_back(startVertIndex + numVertX);

				terrainElements.push_back(startVertIndex + 1);
				terrainElements.push_back(startVertIndex + numVertX + 1);
				terrainElements.push_back(startVertIndex + numVertX);
			}
			else
			{
				terrainElements.push_back(startVertIndex + 1);
				terrainElements.push_back(startVertIndex + numVertX + 1);
				terrainElements.push_back(startVertIndex);

				terrainElements.push_back(startVertIndex + numVertX + 1);
				terrainElements.push_back(startVertIndex + numVertX);
				terrainElements.push_back(startVertIndex);
			}
			DiamondPattern = !DiamondPattern;
		}
		if ((numCellsX * numCellsZ) % 2 == 0)
			DiamondPattern = !DiamondPattern;
	}

	//Normals

	//this makes sure the amount of normals is equal to the amount of verticies
	std::vector <glm::vec3> terrainNormals(terrainVertices.size());
	std::fill(terrainNormals.begin(), terrainNormals.end(), glm::vec3(0, 0, 0));

	//initialises the normals to 0
	for (size_t index{ 0 }; index < terrainElements.size(); index += 3)
	{
		int index1 = terrainElements[index + 0];
		int index2 = terrainElements[index + 1];
		int index3 = terrainElements[index + 2];

		assert(index1 >= 0 && index1 < terrainVertices.size());
		assert(index2 >= 0 && index2 < terrainVertices.size());
		assert(index3 >= 0 && index3 < terrainVertices.size());

		glm::vec3 v0 = terrainVertices[index1];
		glm::vec3 v1 = terrainVertices[index2];
		glm::vec3 v2 = terrainVertices[index3];

		glm::vec3 edge1 = v1 - v0;
		glm::vec3 edge2 = v2 - v0;

		glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

		terrainNormals[index1] += normal;
		terrainNormals[index2] += normal;
		terrainNormals[index3] += normal;

	}

	for (glm::vec3& n : terrainNormals)
		n = glm::normalize(n);

	GLuint positionsVBO;
	
	//Generate 1 buffer id, put the resulting identifier in positionsVBO variable.
	glGenBuffers(1, &positionsVBO);

	//Bind the buffer to the context at the GL_ARRAY_BUFFER binding point (target).
	//This is the first time the buffer is used so this also createsthe buffer object.
	glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);
	
	//Fill the bound buffer with the vertices, we pass the size in bytes and a pointer to the data.
	//the last parameter is a hint to open GL that we will not alter the vertices again.
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * terrainVertices.size(), terrainVertices.data(), GL_STATIC_DRAW);
	
	//Clear binding - not absolutely required but a good idea!
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint normalsVBO;
	glGenBuffers(1, &normalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * terrainNormals.size(), terrainNormals.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint elementsEBO;
	
	glGenBuffers(1, &elementsEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * terrainElements.size(), terrainElements.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLuint texcoordVBO;
	glGenBuffers(1, &texcoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, texcoordVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvCoords.size(), uvCoords.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	terrainMesh.numElements = (GLuint)terrainElements.size();

	//create vao s
	// Create a unique id for a vertex array object
	glGenVertexArrays(1, &terrainMesh.VAO);

	// Bind it to be current and since first use this also allocates memory for it
	// Note no target binding point as there is only one type of vao
	glBindVertexArray(terrainMesh.VAO);



	// Enable the first attribute in the program (the vertices) to stream to the vertex shader
	glEnableVertexAttribArray(0);
	// Bind the vertex buffer to the context (records this action in the VAO)
	glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);
	// Describe the make up of the vertex stream
	glVertexAttribPointer(
		0,                  // attribute 0
		3,                  // size in bytes of each item in the stream
		GL_FLOAT,           // type of the item
		GL_FALSE,           // normalized or not (advanced)
		0,                  // stride (advanced)
		(void*)0            // array buffer offset (advanced)
	);


	glEnableVertexAttribArray(1);	glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);


	glEnableVertexAttribArray(2);	glBindBuffer(GL_ARRAY_BUFFER, texcoordVBO);
	glVertexAttribPointer(
		2,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	// Clear VAO binding
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsEBO);
	glBindVertexArray(0);

	glGenTextures(1, &terrainMesh.textureID);
	glBindTexture(GL_TEXTURE_2D, terrainMesh.textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, terrainTexture.Width(), terrainTexture.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, terrainTexture.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);
	
	
	terrain.myMeshVector.push_back(terrainMesh);
	myObjectVector.push_back(terrain);

	return true;
}

void Renderer::SkyboxLoader(const std::string& Name, const std::string& textureName)
{

	Object Skybox;

	Helpers::ModelLoader skyboxLoader;
	if(!skyboxLoader.LoadFromFile(Name))
		std::cerr << "Could not load model" << std::endl;

	Helpers::ImageLoader SkyBoxTexture;
	if (!SkyBoxTexture.Load(textureName))
		std::cerr << "Could not load model" << std::endl;

	for (const Helpers::Mesh& mesh : skyboxLoader.GetMeshVector())
	{
		MyMesh modelMesh;
		//create vbo s

		GLuint normalsVBO;
		GLuint elementsEBO;
		GLuint texcoordVBO;

		GLuint positionsVBO;
		//Generate 1 buffer id, put the resulting identifier in positionsVBO variable.
		glGenBuffers(1, &positionsVBO);

		//Bind the buffer to the context at the GL_ARRAY_BUFFER binding point (target).
		//This is the first time the buffer is used so this also createsthe buffer object.
		glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);

		//Fill the bound buffer with the vertices, we pass the size in bytes and a pointer to the data.
		//the last parameter is a hint to open GL that we will not alter the vertices again.
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);

		//Clear binding - not absolutely required but a good idea!
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &normalsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.normals.size(), mesh.normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &elementsEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.elements.size(), mesh.elements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenBuffers(1, &texcoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texcoordVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.uvCoords.size(), mesh.uvCoords.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		modelMesh.numElements = (GLuint)mesh.elements.size();

		/*	Create a Vertex Array Object (VAO) to wrap or 'record' all bindings etc. needed to render
			As well as the make up of any streamed data
			Once we bind the VAO subsequent binds etc. are 'recorded' in the VAO.*/

			// Create a unique id for a vertex array object
		glGenVertexArrays(1, &modelMesh.VAO);

		// Bind it to be current and since first use this also allocates memory for it
		// Note no target binding point as there is only one type of vao
		glBindVertexArray(modelMesh.VAO);

		// Bind the vertex buffer to the context (records this action in the VAO)
		glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);

		// Enable the first attribute in the program (the vertices) to stream to the vertex shader
		glEnableVertexAttribArray(0);

		// Describe the make up of the vertex stream
		glVertexAttribPointer(
			0,                  // attribute 0
			3,                  // size in bytes of each item in the stream
			GL_FLOAT,           // type of the item
			GL_FALSE,           // normalized or not (advanced)
			0,                  // stride (advanced)
			(void*)0            // array buffer offset (advanced)
		);

		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		glBindBuffer(GL_ARRAY_BUFFER, texcoordVBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2,
			2,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		glGenTextures(1, &modelMesh.textureID);
		glBindTexture(GL_TEXTURE_2D, modelMesh.textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SkyBoxTexture.Width(), SkyBoxTexture.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, SkyBoxTexture.GetData());
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsEBO);
		glBindVertexArray(0);

		Skybox.myMeshVector.push_back(modelMesh);
	}

	myObjectVector.push_back(Skybox);
}

// Load / create geometry into OpenGL buffers	
bool Renderer::InitialiseGeometry()
{
	// Load and compile shaders into m_program
	if (!CreateProgram())
		return false;

	ModelLoader("Data\\Models\\Jeep\\jeep.obj", "Data\\Models\\Jeep\\jeep_Army.jpg");

	CreateTerrain(32,32, "Data\\Terrain\\grass11.bmp");

	SkyboxLoader("Data\\Sky\\Mars\\skybox.x", "Data\\Sky\\Mars\\");
	
	// Good idea to check for an error now:	
	Helpers::CheckForGLError();

	// Clear VAO binding
	glBindVertexArray(0);

	return true;
}

// Render the scene. Passed the delta time since last called.
void Renderer::Render(const Helpers::Camera& camera, float deltaTime)
{		
	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Uncomment to render in wireframe (can be useful when debugging)
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Clear buffers from previous frame
	glClearColor(0.0f, 0.0f, 0.0f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Compute viewport and projection matrix
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspect_ratio = viewportSize[2] / (float)viewportSize[3];
	glm::mat4 projection_xform = glm::perspective(glm::radians(45.0f), aspect_ratio, 1.0f, 20000.0f);

	// Compute camera view matrix and combine with projection matrix for passing to shader
	glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
	glm::mat4 combined_xform = projection_xform * view_xform;

	// Use our program. Doing this enables the shaders we attached previously.
	glUseProgram(m_program);

	// Send the combined matrix to the shader in a uniform
	GLuint combined_xform_id = glGetUniformLocation(m_program, "combined_xform");
	glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));

	glm::mat4 model_xform = glm::mat4(1);
	
// Send the model matrix to the shader in a uniform
	GLuint model_xform_id = glGetUniformLocation(m_program, "model_xform");
	glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));

	for (Object &model: myObjectVector)
	{
		for (int i{ 0 }; i < model.myMeshVector.size(); ++i)
		{	glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, model.myMeshVector[i].textureID);
			glUniform1i(glGetUniformLocation(m_program, "sampler_tex"), 0);

			// Bind our VAO and render
			glBindVertexArray(model.myMeshVector[i].VAO);
			glDrawElements(GL_TRIANGLES, model.myMeshVector[i].numElements, GL_UNSIGNED_INT, (void*)0);
		}
	}
		// Always a good idea, when debugging at least, to check for GL errors
		Helpers::CheckForGLError();
}
