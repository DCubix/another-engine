#include "mesh.h"

namespace ae {
	Mesh& Mesh::create() {
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

		return *this;
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

	Mesh& Mesh::dynamic(bool v) {
		m_dynamic = v;
		return *this;
	}

	Mesh& Mesh::addVertex(const Vertex& vertex) {
		m_vertices.push_back(vertex);
		return *this;
	}

	Mesh& Mesh::addIndex(uint32 i) {
		m_indices.push_back(i);
		return *this;
	}

	Mesh& Mesh::addTriangle(uint32 i0, uint32 i1, uint32 i2) {
		m_indices.push_back(i0);
		m_indices.push_back(i1);
		m_indices.push_back(i2);
		return *this;
	}

	Mesh& Mesh::setData(const std::vector<Vertex>& vertices, const std::vector<uint32>& indices) {
		m_vertices = vertices;
		m_indices = indices;
		return *this;
	}

	Mesh& Mesh::build() {
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

		m_length = m_indices.size();

		m_vertices.clear();
		m_indices.clear();

		return *this;
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

	Mesh& Mesh::calculateNormals(PrimitiveType primitive) {
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

		return *this;
	}

	Mesh& Mesh::calculateTangents(PrimitiveType primitive) {
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

		return *this;
	}

	Mesh& Mesh::transformTexCoord(const Matrix4& mat) {
		for (auto& v : m_vertices) {
			v.texCoord = (mat * Vector4(v.texCoord)).toVector2();
		}
		return *this;
	}

	void Mesh::free() {
		if (m_vbo) glDeleteBuffers(1, &m_vbo);
		if (m_ebo) glDeleteBuffers(1, &m_ebo);
		if (m_vao) glDeleteVertexArrays(1, &m_vao);
	}

}