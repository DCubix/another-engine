#include "mesh.h"

#include "resource_manager.h"
#include "util.hpp"

#include <sstream>
#include <cfloat>
#include <algorithm>

namespace ae {
	void Mesh::create() {
		glGenVertexArrays(1, &m_vao);
		glGenBuffers(1, &m_vbo);
		glGenBuffers(1, &m_ebo);

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);

		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*) offsetof(Vertex, position));
		glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), (void*) offsetof(Vertex, normal));
		glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(Vertex), (void*) offsetof(Vertex, tangent));
		glVertexAttribPointer(3, 2, GL_FLOAT, false, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		glBindVertexArray(0);
	}

	Mesh::Mesh() {
		create();
	}

	Mesh::~Mesh() {
		free();
	}

	void Mesh::bind() {
		glBindVertexArray(m_vao);
	}

	void Mesh::unbind() {
		glBindVertexArray(0);
	}

	void Mesh::draw(PrimitiveType primitive, int32 length, uint32 offset) {
		length = length < 0 ? m_length : length;
		glDrawElements(primitive, length, GL_UNSIGNED_INT, (void*) (offset * sizeof(uint32)));
	}

	void Mesh::dynamic(bool v) {
		m_dynamic = v;
	}

	void Mesh::addVertex(const Vertex& vertex) {
		m_vertices.push_back(vertex);
	}

	void Mesh::addIndex(uint32 i) {
		m_indices.push_back(i);
	}

	void Mesh::addTriangle(uint32 i0, uint32 i1, uint32 i2) {
		m_indices.push_back(i0);
		m_indices.push_back(i1);
		m_indices.push_back(i2);
	}

	void Mesh::setData(const std::vector<Vertex>& vertices, const std::vector<uint32>& indices) {
		m_vertices = vertices;
		m_indices = indices;
	}

	void Mesh::build() {
		const GLenum usage = m_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		if (m_vertices.size() > m_previousVBOSize) {
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertices.size(), m_vertices.data(), usage);
			m_previousVBOSize = m_vertices.size();
		} else {
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * m_vertices.size(), m_vertices.data());
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		if (m_indices.size() > m_previousEBOSize) {
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * m_indices.size(), m_indices.data(), usage);
			m_previousEBOSize = m_indices.size();
		} else {
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint32) * m_indices.size(), m_indices.data());
		}

		buildAABB();

		m_length = m_indices.size();

		m_vertices.clear();
		m_indices.clear();
	}

	void Mesh::normalize() {
		buildAABB();

		const Vector3 size = m_aabb.max - m_aabb.min;
		const Vector3 scale{ 1.0f / size.x, 1.0f / size.y, 1.0f / size.z };
		const float scaleFactor = scale.length();

		for (auto& v : m_vertices) {
			v.position = v.position * scaleFactor;
		}
	}

	void Mesh::centralize() {
		Vector3 center{};
		for (auto& v : m_vertices) {
			center = center + v.position;
		}
		center = center / m_vertices.size();

		const Vector3 target = center * -1.0f;
		for (auto& v : m_vertices) {
			v.position = v.position + target;
		}
	}

	void Mesh::buildAABB() {
		m_aabb.min = Vector3(FLT_MAX);
		m_aabb.max = Vector3(FLT_MIN);
		for (auto& i : m_indices) {
			auto& v = m_vertices[i];
			m_aabb.min.x = std::min(m_aabb.min.x, v.position.x);
			m_aabb.min.y = std::min(m_aabb.min.y, v.position.y);
			m_aabb.min.z = std::min(m_aabb.min.z, v.position.z);
			m_aabb.max.x = std::max(m_aabb.max.x, v.position.x);
			m_aabb.max.y = std::max(m_aabb.max.y, v.position.y);
			m_aabb.max.z = std::max(m_aabb.max.z, v.position.z);
		}
	}

	void Mesh::calculateTriangleNormal(uint32 i0, uint32 i1, uint32 i2) {
		const Vector3 p0 = m_vertices[i0].position;
		const Vector3 p1 = m_vertices[i1].position;
		const Vector3 p2 = m_vertices[i2].position;

		const Vector3 e0 = p1 - p0;
		const Vector3 e1 = p2 - p0;
		const Vector3 n = e0.cross(e1);

		m_vertices[i0].normal = m_vertices[i0].normal + n;
		m_vertices[i1].normal = m_vertices[i1].normal + n;
		m_vertices[i2].normal = m_vertices[i2].normal + n;
	}

	void Mesh::calculateTriangleTangent(uint32 i0, uint32 i1, uint32 i2) {
		const Vector3 v0 = m_vertices[i0].position;
		const Vector3 v1 = m_vertices[i1].position;
		const Vector3 v2 = m_vertices[i2].position;

		const Vector3 t0 = m_vertices[i0].texCoord;
		const Vector3 t1 = m_vertices[i1].texCoord;
		const Vector3 t2 = m_vertices[i2].texCoord;

		const Vector3 e0 = v1 - v0;
		const Vector3 e1 = v2 - v0;

		const Vector3 dt1 = t1 - t0;
		const Vector3 dt2 = t2 - t0;

		float dividend = dt1.x * dt2.y - dt1.y * dt2.x;
		float f = dividend == 0.0f ? 0.0f : 1.0f / dividend;

		Vector3 t = Vector3();
		t.x = (f * (dt2.y * e0.x - dt1.y * e1.x));
		t.y = (f * (dt2.y * e0.y - dt1.y * e1.y));
		t.z = (f * (dt2.y * e0.z - dt1.y * e1.z));

		m_vertices[i0].tangent = m_vertices[i0].tangent + t;
		m_vertices[i1].tangent = m_vertices[i1].tangent + t;
		m_vertices[i2].tangent = m_vertices[i2].tangent + t;
	}

	void Mesh::calculateNormals(PrimitiveType primitive) {
#define index(x) m_indices[(x)]
		switch (primitive) {
			case Points:
			case Lines:
			case LineLoop:
			case LineStrip:
				break;
			case Triangles: {
				for (uint32 i = 0; i < m_indices.size(); i+=3) {
					uint32 i0 = index(i + 0);
					uint32 i1 = index(i + 1);
					uint32 i2 = index(i + 2);
					calculateTriangleNormal(i0, i1, i2);
				}
			} break;
			case TriangleFan: {
				uint32 i0 = index(0);
				for (uint32 i = 0; i < m_indices.size(); i+=2) {
					uint32 i1 = index(i + 0);
					uint32 i2 = index(i + 1);
					calculateTriangleNormal(i0, i1, i2);
				}
			} break;
			case TriangleStrip: {
				for (uint32 i = 0; i < m_indices.size(); i+=2) {
					uint32 i0, i1, i2;
					if (i % 2 == 0) {
						i0 = index(i + 0);
						i1 = index(i + 1);
						i2 = index(i + 2);
					} else {
						i0 = index(i + 2);
						i1 = index(i + 1);
						i2 = index(i + 0);
					}
					calculateTriangleNormal(i0, i1, i2);
				}
			} break;
		}

		for (auto& v : m_vertices) {
			v.normal = v.normal.normalized();
		}
	}

	void Mesh::calculateTangents(PrimitiveType primitive) {
#define index(x) m_indices[(x)]
		switch (primitive) {
			case Points:
			case Lines:
			case LineLoop:
			case LineStrip:
				break;
			case Triangles: {
				for (uint32 i = 0; i < m_indices.size(); i+=3) {
					uint32 i0 = index(i + 0);
					uint32 i1 = index(i + 1);
					uint32 i2 = index(i + 2);
					calculateTriangleTangent(i0, i1, i2);
				}
			} break;
			case TriangleFan: {
				uint32 i0 = index(0);
				for (uint32 i = 0; i < m_indices.size(); i+=2) {
					uint32 i1 = index(i + 0);
					uint32 i2 = index(i + 1);
					calculateTriangleTangent(i0, i1, i2);
				}
			} break;
			case TriangleStrip: {
				for (uint32 i = 0; i < m_indices.size(); i+=2) {
					uint32 i0, i1, i2;
					if (i % 2 == 0) {
						i0 = index(i + 0);
						i1 = index(i + 1);
						i2 = index(i + 2);
					} else {
						i0 = index(i + 2);
						i1 = index(i + 1);
						i2 = index(i + 0);
					}
					calculateTriangleTangent(i0, i1, i2);
				}
			} break;
		}

		for (auto& v : m_vertices) {
			v.tangent = v.tangent.normalized();
		}
	}

	void Mesh::transformTexCoord(const Matrix4& mat) {
		for (auto& v : m_vertices) {
			v.texCoord = (mat * Vector4(v.texCoord)).toVector2();
		}
	}

	void Mesh::free() {
		if (m_vbo) glDeleteBuffers(1, &m_vbo);
		if (m_ebo) glDeleteBuffers(1, &m_ebo);
		if (m_vao) glDeleteVertexArrays(1, &m_vao);
	}

	MeshFactory::MeshFactory(const std::string& fileName) {
		this->fileName(fileName);
	}

	ResourcePtr MeshFactory::load() {
		if (!m_ptr) {
			m_ptr = std::make_shared<Mesh>();
			auto file = FileSystem::ston().open(fileName());
			auto sz = file.size();
			std::string data(sz, '\0');

			if (file.read(data.data(), sz) == sz) {
				std::stringstream ss, outVS, outFS, outGS, outCS;
				ss << data.data();

				std::vector<Face> faces;
				std::vector<Vector3> vertices, normals;
				std::vector<Vector2> texCoords;
				bool reverseVertexOrder = false, calcNormals = false, calcTangs = false, normalize = false, centralize = false;

				std::string line;
				while (std::getline(ss, line)) {
					line = util::rtrim(line);

					std::stringstream ls(line);
					std::string tok; ls >> tok;

					if (tok == "f") {
						// WARNING! : Triangulate your meshes before using them!
						for (uint32 i = 0; i < 3; i++) {
							std::string face; ls >> face;
							auto faceData = util::split(face, '/');
							Face f{};
							if (faceData.size() == 1) { // f v0 ...
								f.vertex = std::stoi(faceData[0])-1;
							} else if (faceData.size() == 2) { // f v0/vt0
								f.vertex = std::stoi(faceData[0])-1;
								f.texCoord = std::stoi(faceData[1])-1;
							} else if (faceData.size() == 3) {
								if (faceData[1].empty()) { // f v0//vn0
									f.vertex = std::stoi(faceData[0])-1;
									f.normal = std::stoi(faceData[2])-1;
								} else { // f v0/vt0/vn0
									f.vertex = std::stoi(faceData[0])-1;
									f.texCoord = std::stoi(faceData[1])-1;
									f.normal = std::stoi(faceData[2])-1;
								}
							}
							faces.push_back(f);
						}
					} else if (tok == "v") {
						std::string comp;
						ls >> comp;
						float x = std::stof(comp);
						ls >> comp;
						float y = std::stof(comp);
						ls >> comp;
						float z = std::stof(comp);
						vertices.push_back(Vector3(x, y, z));
					} else if (tok == "vt") {
						std::string comp;
						ls >> comp;
						float x = std::stof(comp);
						ls >> comp;
						float y = std::stof(comp);
						texCoords.push_back(Vector2(x, y));
					} else if (tok == "vn") {
						std::string comp;
						ls >> comp;
						float x = std::stof(comp);
						ls >> comp;
						float y = std::stof(comp);
						ls >> comp;
						float z = std::stof(comp);
						normals.push_back(Vector3(x, y, z));
					} else if (tok == "reverse_vertices") {
						std::string comp;
						ls >> comp;
						if (comp == "1") reverseVertexOrder = true;
					} else if (tok == "calc_normals") {
						calcNormals = true;
					} else if (tok == "calc_tangents") {
						calcTangs = true;
					} else if (tok == "normalize") {
						normalize = true;
					} else if (tok == "centralize") {
						centralize = true;
					}
				}

				std::vector<Vertex> verticesConverted;
				std::vector<uint32> indices;
				uint32 idx = 0;
				for (uint32 i = 0; i < faces.size(); i += 3) {
					for (uint32 j = 0; j < 3; j++) {
						uint32 k = i + (reverseVertexOrder ? (2 - j) : j);
						Face& f = faces[k];
						Vertex vert{};
						if (f.vertex != -1) vert.position = vertices[f.vertex];
						if (f.normal != -1) vert.normal = normals[f.normal];
						if (f.texCoord != -1) vert.texCoord = texCoords[f.texCoord];
						verticesConverted.push_back(vert);
						indices.push_back(idx++);
					}
				}
				m_ptr->setData(verticesConverted, indices);
				if (centralize) m_ptr->centralize();
				if (normalize) m_ptr->normalize();
				if (calcNormals) m_ptr->calculateNormals(Mesh::Triangles);
				if (calcTangs) m_ptr->calculateTangents(Mesh::Triangles);
				m_ptr->build();
			}
			file.close();
		}
		return m_ptr;
	}

}