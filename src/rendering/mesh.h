#ifndef MESH_H
#define MESH_H

#include "integer.hpp"
#include "glad.h"
#include "vec_math.hpp"
#include "resource_manager.h"

#include <vector>

namespace ae {
	struct Vertex {
		Vector3 position, normal, tangent;
		Vector2 texCoord;
	};

	class Mesh : public Resource {
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

		Mesh();
		~Mesh();

		void create();
		void free();

		void bind();
		void unbind();

		void draw(PrimitiveType primitive, int32 length = -1, uint32 offset = 0);

		void dynamic(bool v = true);
		void addVertex(const Vertex& vertex);
		void addIndex(uint32 i);
		void addTriangle(uint32 i0, uint32 i1, uint32 i2);
		void setData(const std::vector<Vertex>& vertices, const std::vector<uint32>& indices);
		void fromFile(const std::string& fileName) override;
		void calculateNormals(PrimitiveType primitive);
		void calculateTangents(PrimitiveType primitive);
		void transformTexCoord(const Matrix4& mat);
		void build();

		void normalize();
		void centralize();

		bool dynamic() const { return m_dynamic; }

		const AABB& aabb() const { return m_aabb; }
		GLuint vao() const { return m_vao; }
		GLuint vbo() const { return m_vbo; }
		GLuint ebo() const { return m_ebo; }

	private:
		struct Face {
			uint32 vertex, texCoord, normal;
		};

		GLuint m_vao{ 0 }, m_vbo{ 0 }, m_ebo{ 0 };
		uint32 m_previousVBOSize{ 0 }, m_previousEBOSize{ 0 }, m_length{ 0 };

		bool m_dynamic{ false };

		std::vector<Vertex> m_vertices;
		std::vector<uint32> m_indices;

		AABB m_aabb{};

		void calculateTriangleNormal(uint32 i0, uint32 i1, uint32 i2);
		void calculateTriangleTangent(uint32 i0, uint32 i1, uint32 i2);

		void buildAABB();
	};

}

#endif // MESH_H
