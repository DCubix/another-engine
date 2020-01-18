#ifndef MESH_H
#define MESH_H

#include "integer.hpp"
#include "glad.h"
#include "vec_math.hpp"

#include <vector>

namespace ae {
	struct Vertex {
		Vector3 position, normal, tangent;
		Vector2 texCoord;
	};

	class Mesh {
	public:
		enum PrimitiveType {
			Points = GL_POINTS,
			Lines = GL_LINES,
			LineLoop = GL_LINE_LOOP,
			LineStrip = GL_LINE_STRIP,
			Triangles = GL_TRIANGLES,
			TriangleFan = GL_TRIANGLE_FAN,
			TriangleStrip = GL_TRIANGLE_STRIP
		};

		Mesh() = default;
		~Mesh() = default;

		Mesh& create();
		void free();

		void bind();
		void unbind();

		void draw(PrimitiveType primitive, int32 length = -1, uint32 offset = 0);

		Mesh& dynamic(bool v = true);
		Mesh& addVertex(const Vertex& vertex);
		Mesh& addIndex(uint32 i);
		Mesh& addTriangle(uint32 i0, uint32 i1, uint32 i2);
		Mesh& setData(const std::vector<Vertex>& vertices, const std::vector<uint32>& indices);
		Mesh& calculateNormals(PrimitiveType primitive);
		Mesh& calculateTangents(PrimitiveType primitive);
		Mesh& transformTexCoord(const Matrix4& mat);
		Mesh& build();

		bool dynamic() const { return m_dynamic; }

	private:
		GLuint m_vao{ 0 }, m_vbo{ 0 }, m_ebo{ 0 };
		uint32 m_previousVBOSize{ 0 }, m_previousEBOSize{ 0 }, m_length{ 0 };

		bool m_dynamic{ false };

		std::vector<Vertex> m_vertices;
		std::vector<uint32> m_indices;

		void calculateTriangleNormal(uint32 i0, uint32 i1, uint32 i2);
		void calculateTriangleTangent(uint32 i0, uint32 i1, uint32 i2);
	};
}

#endif // MESH_H
