#ifndef SHADER_H
#define SHADER_H

#include <unordered_map>
#include <vector>
#include <string>

#include "integer.hpp"
#include "glad.h"
#include "vec_math.hpp"

namespace ae {
	enum ShaderType {
		VertexShader = GL_VERTEX_SHADER,
		FragmentShader = GL_FRAGMENT_SHADER,
		GeometryShader = GL_GEOMETRY_SHADER,
		ComputeShader = GL_COMPUTE_SHADER
	};

	enum DepthTest {
		Never = GL_NEVER,
		Less = GL_LESS,
		Equal = GL_EQUAL,
		LessEqual = GL_LEQUAL,
		Greater = GL_GREATER,
		GreaterEqual = GL_GEQUAL,
		Always = GL_ALWAYS
	};

	enum BlendMode {
		Opaque = 0,
		Alpha,
		Add
	};

	namespace intern {
		struct Uniform {
			void operator =(int v);
			void operator =(float v);
			void operator =(const Vector2& v);
			void operator =(const Vector3& v);
			void operator =(const Vector4& v);
			void operator =(Matrix4 v);
			uint32 loc;
		};
	}

	class Shader {
	public:
		Shader() = default;
		~Shader() = default;

		Shader& create();
		void free();

		Shader& addShader(const std::string& source, ShaderType type);
		Shader& link();

		void bind();
		void unbind();

		int32 getUniformLocation(const std::string name);

		DepthTest depthTest() const { return m_depthTest; }
		void depthTest(DepthTest v) { m_depthTest = v; }

		BlendMode blendMode() const { return m_blendMode; }
		void blendMode(BlendMode v) { m_blendMode = v; }

		bool depthWrite() const { return m_depthWrite; }
		void depthWrite(bool v) { m_depthWrite = v; }

		intern::Uniform& operator[](const std::string& name);

	private:
		uint32 m_program{ 0 };

		DepthTest m_depthTest{ DepthTest::Less };
		BlendMode m_blendMode{ BlendMode::Opaque };
		bool m_depthWrite{ true };

		std::unordered_map<std::string, int32> m_uniforms;
		std::unordered_map<uint32, std::string> m_shaders;

		intern::Uniform m_uniformHandler;
	};
}

#endif // SHADER_H
