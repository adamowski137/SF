#pragma once
#include <vector>
#include <memory>
#include <GL/glew.h>

class Shader;

class Mesh {
	unsigned VAO;
	unsigned VBO;
	unsigned IBO;
	unsigned nIndices;
	unsigned maxVertexBufferSize;
	unsigned maxIndexBufferSize;
public:
	Mesh();
	~Mesh();
	static std::shared_ptr<Mesh> createMesh(const std::vector<float>& vertices, 
		const std::vector<unsigned>& indices, const std::vector<std::pair<int, int>>& dataLayout = { {3, GL_FLOAT} });
	static std::shared_ptr<Mesh> createSimpleMesh(const std::vector<float>& vertices, const std::vector<std::pair<int, int>>& dataLayout = {{3, GL_FLOAT}});
	void updateMesh(const std::vector<float>& vertices, const std::vector<unsigned>& indices);
	int getIndiciesCount() const;
	void bind() const;
};
