#include "core/MeshGenerator.hpp"
#include "core/Mesh.hpp"
#include <GL/glew.h>
#include <cmath>

std::shared_ptr<Mesh> MeshGenerator::pointMesh = nullptr;
std::shared_ptr<Mesh> MeshGenerator::cursorMesh = nullptr;
std::shared_ptr<Mesh> MeshGenerator::gridMesh = nullptr;

std::shared_ptr<Mesh> MeshGenerator::GetCubeMesh()
{
	// Each face: 4 vertices with position (3) + normal (3) = 6 floats per vertex
	std::vector<float> vertices = {
		// Front face (z = +0.5, normal 0,0,1)
		-0.5f, -0.5f,  0.5f,  0.f, 0.f, 1.f,
		 0.5f, -0.5f,  0.5f,  0.f, 0.f, 1.f,
		 0.5f,  0.5f,  0.5f,  0.f, 0.f, 1.f,
		-0.5f,  0.5f,  0.5f,  0.f, 0.f, 1.f,

		// Back face (z = -0.5, normal 0,0,-1)
		 0.5f, -0.5f, -0.5f,  0.f, 0.f, -1.f,
		-0.5f, -0.5f, -0.5f,  0.f, 0.f, -1.f,
		-0.5f,  0.5f, -0.5f,  0.f, 0.f, -1.f,
		 0.5f,  0.5f, -0.5f,  0.f, 0.f, -1.f,

		// Top face (y = +0.5, normal 0,1,0)
		-0.5f,  0.5f,  0.5f,  0.f, 1.f, 0.f,
		 0.5f,  0.5f,  0.5f,  0.f, 1.f, 0.f,
		 0.5f,  0.5f, -0.5f,  0.f, 1.f, 0.f,
		-0.5f,  0.5f, -0.5f,  0.f, 1.f, 0.f,

		// Bottom face (y = -0.5, normal 0,-1,0)
		-0.5f, -0.5f, -0.5f,  0.f, -1.f, 0.f,
		 0.5f, -0.5f, -0.5f,  0.f, -1.f, 0.f,
		 0.5f, -0.5f,  0.5f,  0.f, -1.f, 0.f,
		-0.5f, -0.5f,  0.5f,  0.f, -1.f, 0.f,

		// Right face (x = +0.5, normal 1,0,0)
		 0.5f, -0.5f,  0.5f,  1.f, 0.f, 0.f,
		 0.5f, -0.5f, -0.5f,  1.f, 0.f, 0.f,
		 0.5f,  0.5f, -0.5f,  1.f, 0.f, 0.f,
		 0.5f,  0.5f,  0.5f,  1.f, 0.f, 0.f,

		// Left face (x = -0.5, normal -1,0,0)
		-0.5f, -0.5f, -0.5f, -1.f, 0.f, 0.f,
		-0.5f, -0.5f,  0.5f, -1.f, 0.f, 0.f,
		-0.5f,  0.5f,  0.5f, -1.f, 0.f, 0.f,
		-0.5f,  0.5f, -0.5f, -1.f, 0.f, 0.f,
	};

	std::vector<unsigned> indices;
	for (unsigned face = 0; face < 6; face++)
	{
		unsigned base = face * 4;
		indices.push_back(base + 0);
		indices.push_back(base + 1);
		indices.push_back(base + 2);
		indices.push_back(base + 2);
		indices.push_back(base + 3);
		indices.push_back(base + 0);
	}

	return Mesh::createMesh(vertices, indices, { {3, GL_FLOAT}, {3, GL_FLOAT} });
}

std::shared_ptr<Mesh> MeshGenerator::GetSphereMesh()
{
	std::vector<float> vertices;
	std::vector<unsigned> indices;

	const int n_subdiv = 50;
	for (int i = 0; i < n_subdiv; i++)
	{
		float theta = i * 3.1415f * 2 / n_subdiv;
		for (int j = 0; j <= n_subdiv; j++)
		{
			float phi = j * 3.1415f / n_subdiv;
			float x = cosf(theta) * sinf(phi);
			float y = sinf(theta) * sinf(phi);
			float z = cosf(phi);
			// position + normal (sphere normal = position on unit sphere)
			vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
			vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
		}
	}

	for (int i = 0; i < n_subdiv; i++)
	{
		for (int j = 0; j < n_subdiv; j++)
		{
			indices.push_back(i * (n_subdiv + 1) + j);
			indices.push_back(i * (n_subdiv + 1) + j + 1);
			indices.push_back(((i + 1) % n_subdiv) * (n_subdiv + 1) + j + 1);

			indices.push_back(((i + 1) % n_subdiv) * (n_subdiv + 1) + j + 1);
			indices.push_back(((i + 1) % n_subdiv) * (n_subdiv + 1) + j);
			indices.push_back(i * (n_subdiv + 1) + j);
		}
	}

	return Mesh::createMesh(vertices, indices, { {3, GL_FLOAT}, {3, GL_FLOAT} });
}

std::shared_ptr<Mesh> MeshGenerator::GetPointMesh()
{
	if (pointMesh == nullptr)
	{
		std::vector<float> vertices = {
			-0.5f, -0.5f, 0.f,
			0.5f, -0.5f, 0.f,
			0.5f, 0.5f, 0.f,
			-0.5f, 0.5f, 0.f,
		};

		std::vector<unsigned int> indices = {
			0, 1, 2, 2, 3, 0
		};

		pointMesh = Mesh::createMesh(vertices, indices);
	}

	return pointMesh;
}

std::shared_ptr<Mesh> MeshGenerator::GetGridMesh()
{
	const float gridSize = 50.f;
	if (gridMesh == nullptr)
	{
		std::vector<float> verticies;
		for (float x = -gridSize / 2; x <= gridSize / 2; x += 1.f)
		{
			verticies.push_back(x);
			verticies.push_back(0);
			verticies.push_back(-gridSize / 2);

			verticies.push_back(x);
			verticies.push_back(0);
			verticies.push_back(gridSize / 2);
		}

		for (float z = -gridSize / 2; z <= gridSize / 2; z += 1.f)
		{
			verticies.push_back(-gridSize / 2);
			verticies.push_back(0);
			verticies.push_back(z);

			verticies.push_back(gridSize / 2);
			verticies.push_back(0);
			verticies.push_back(z);
		}

		std::vector<unsigned int> indices;
		for (int i = 0; 3 * (i + 1) < (int)verticies.size(); i += 2)
		{
			indices.push_back(i);
			indices.push_back(i + 1);
		}

		gridMesh = Mesh::createMesh(verticies, indices);
	}

	return gridMesh;
}

std::shared_ptr<Mesh> MeshGenerator::GetCursorMesh()
{
	if (cursorMesh == nullptr)
	{
		std::vector<float> vertices = {
			-0.25f, 0.f, 0.f, 1.f, 0.f, 0.f,
			0.25f, 0.f, 0.f, 1.f, 0.f, 0.f,
			0.f, -0.25f, 0.f, 0.f, 1.f, 0.f,
			0.f, 0.25f, 0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, -0.25f, 0.f, 0.f, 1.f,
			0.f, 0.f, 0.25f, 0.f, 0.f, 1.f,
		};

		std::vector<unsigned int> indices = {
			0, 1, 2, 3, 4, 5
		};

		cursorMesh = Mesh::createMesh(vertices, indices, { {3, GL_FLOAT}, {3, GL_FLOAT} });
	}

	return cursorMesh;
}

std::pair<std::vector<float>, std::vector<unsigned int>> MeshGenerator::GetTorusVerticies(float r, float R, int div_r, int div_R)
{
	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	for (int i = 0; i <= div_R; i++)
	{
		float beta = i * 2.0f * 3.14159265f / div_R;
		for (int j = 0; j <= div_r; j++)
		{
			float alpha = j * 2.0f * 3.14159265f / div_r;

			float x = (R + r * cosf(alpha)) * cosf(beta);
			float y = r * sinf(alpha);
			float z = (R + r * cosf(alpha)) * sinf(beta);

			float nx = cosf(alpha) * cosf(beta);
			float ny = sinf(alpha);
			float nz = cosf(alpha) * sinf(beta);

			vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
			vertices.push_back(nx); vertices.push_back(ny); vertices.push_back(nz);
		}
	}

	for (int i = 0; i < div_R; i++)
	{
		for (int j = 0; j < div_r; j++)
		{
			int curr = i * (div_r + 1) + j;
			int next_i = (i + 1) * (div_r + 1) + j;
			int next_j = i * (div_r + 1) + (j + 1);
			int diag = (i + 1) * (div_r + 1) + (j + 1);

			indices.push_back(curr);
			indices.push_back(next_j);
			indices.push_back(next_i);

			indices.push_back(next_i);
			indices.push_back(next_j);
			indices.push_back(diag);
		}
	}

	return { vertices, indices };
}

std::shared_ptr<Mesh> MeshGenerator::GetTorusMesh(float r, float R, int div_r, int div_R)
{
	auto vertexData = GetTorusVerticies(r, R, div_r, div_R);
	return Mesh::createMesh(vertexData.first, vertexData.second, { {3, GL_FLOAT}, {3, GL_FLOAT} });
}

namespace {
	std::pair<std::vector<float>, std::vector<unsigned int>> GetCylinderVertices(
		float radius, float height, int div_circ, int div_height)
	{
		std::vector<float> vertices;
		std::vector<unsigned int> indices;
		float half_h = height * 0.5f;

		for (int i = 0; i <= div_height; i++)
		{
			float y = -half_h + i * height / div_height;
			for (int j = 0; j <= div_circ; j++)
			{
				float theta = j * 2.0f * 3.14159265f / div_circ;
				float x = radius * cosf(theta);
				float z = radius * sinf(theta);
				vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
				vertices.push_back(cosf(theta)); vertices.push_back(0.0f); vertices.push_back(sinf(theta));
			}
		}

		for (int i = 0; i < div_height; i++)
		{
			for (int j = 0; j < div_circ; j++)
			{
				int curr = i * (div_circ + 1) + j;
				int next_i = (i + 1) * (div_circ + 1) + j;
				int next_j = curr + 1;
				int diag = next_i + 1;

				indices.push_back(curr); indices.push_back(next_i); indices.push_back(next_j);
				indices.push_back(next_i); indices.push_back(diag); indices.push_back(next_j);
			}
		}

		// Top cap
		int top_center_index = (int)(vertices.size() / 6);
		vertices.push_back(0.0f); vertices.push_back(half_h); vertices.push_back(0.0f);
		vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
		for (int j = 0; j <= div_circ; j++) {
			float theta = j * 2.0f * 3.14159265f / div_circ;
			vertices.push_back(radius * cosf(theta)); vertices.push_back(half_h); vertices.push_back(radius * sinf(theta));
			vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
		}
		for (int j = 0; j < div_circ; j++) {
			indices.push_back(top_center_index);
			indices.push_back(top_center_index + j + 2);
			indices.push_back(top_center_index + j + 1);
		}

		// Bottom cap
		int bottom_center_index = (int)(vertices.size() / 6);
		vertices.push_back(0.0f); vertices.push_back(-half_h); vertices.push_back(0.0f);
		vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);
		for (int j = 0; j <= div_circ; j++) {
			float theta = j * 2.0f * 3.14159265f / div_circ;
			vertices.push_back(radius * cosf(theta)); vertices.push_back(-half_h); vertices.push_back(radius * sinf(theta));
			vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);
		}
		for (int j = 0; j < div_circ; j++) {
			indices.push_back(bottom_center_index);
			indices.push_back(bottom_center_index + j + 1);
			indices.push_back(bottom_center_index + j + 2);
		}

		return { vertices, indices };
	}
}

std::shared_ptr<Mesh> MeshGenerator::GetCylinderMesh(float r, float h, int div_r, int div_h)
{
	auto vertexData = GetCylinderVertices(r, h, div_r, div_h);
	return Mesh::createMesh(vertexData.first, vertexData.second, { {3, GL_FLOAT}, {3, GL_FLOAT} });
}
