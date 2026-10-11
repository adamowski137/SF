#include "core/Mesh.hpp"
#include "core/error.hpp"
#include <numeric>
#include <algorithm>

Mesh::Mesh() : VAO(0), VBO(0), IBO(0), maxIndexBufferSize(0), maxVertexBufferSize(0), nIndices(0)
{
}

Mesh::~Mesh()
{
	if (VBO) glDeleteBuffers(1, &VBO);
	if (IBO) glDeleteBuffers(1, &IBO);
	if (VAO) glDeleteVertexArrays(1, &VAO);
}

std::shared_ptr<Mesh> Mesh::createMesh(const std::vector<float>& vertices, const std::vector<unsigned>& indices, const std::vector<std::pair<int, int>>& dataLayout)
{
	std::shared_ptr<Mesh> m = std::make_shared<Mesh>();

	Call(glGenVertexArrays(1, &m->VAO));
	Call(glGenBuffers(1, &m->VBO));
	Call(glBindVertexArray(m->VAO));
	Call(glBindBuffer(GL_ARRAY_BUFFER, m->VBO));
	Call(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW));
	unsigned vertexDataSize = std::accumulate(std::begin(dataLayout), std::end(dataLayout), 0, [](unsigned q0, std::pair<unsigned, unsigned> p) {
		return q0 + p.first;
		});
	unsigned vertexDataOffset = 0;
	for (int i = 0; i < (int)dataLayout.size(); i++)
	{
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, dataLayout[i].first, dataLayout[i].second, GL_FALSE,
			vertexDataSize * sizeof(float), (const void*)(sizeof(float) * vertexDataOffset));
		vertexDataOffset += dataLayout[i].first;
	}

	Call(glBindBuffer(GL_ARRAY_BUFFER, 0));
	Call(glGenBuffers(1, &m->IBO));
	Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->IBO));
	Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), indices.data(), GL_DYNAMIC_DRAW));
	Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

	Call(glBindVertexArray(0));
	m->nIndices = (unsigned)indices.size();
	m->maxVertexBufferSize = (unsigned)(sizeof(float) * vertices.size());
	m->maxIndexBufferSize = (unsigned)(sizeof(unsigned) * indices.size());

	return m;
}

std::shared_ptr<Mesh> Mesh::createSimpleMesh(const std::vector<float>& vertices, const std::vector<std::pair<int, int>>& dataLayout)
{
	std::vector<unsigned int> indices;
	for (int i = 0; i < (int)vertices.size() / 3; i++)
		indices.push_back(i);
	return Mesh::createMesh(vertices, indices, dataLayout);
}

void Mesh::updateMesh(const std::vector<float>& vertices, const std::vector<unsigned>& indices)
{
	if (maxVertexBufferSize >= vertices.size() * sizeof(float) && maxIndexBufferSize >= indices.size() * sizeof(unsigned))
	{
		Call(glBindBuffer(GL_ARRAY_BUFFER, VBO));
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
		Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO));
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(unsigned), indices.data());
	}
	else
	{
		Call(glBindBuffer(GL_ARRAY_BUFFER, VBO));
		Call(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW));
		Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO));
		Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), indices.data(), GL_DYNAMIC_DRAW));

		maxVertexBufferSize = (unsigned)(sizeof(float) * vertices.size());
		maxIndexBufferSize = (unsigned)(sizeof(unsigned) * indices.size());
	}

	nIndices = (unsigned)indices.size();
}

int Mesh::getIndiciesCount() const
{
	return this->nIndices;
}

void Mesh::bind() const
{
	Call(glBindVertexArray(VAO));
	Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO));
}
