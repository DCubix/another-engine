#include "shader.h"

#include "log.h"

namespace ae {
	Shader& Shader::create() {
		m_program = glCreateProgram();
		return *this;
	}

	void Shader::free() {
		for (auto& [s, src] : m_shaders) {
			glDeleteShader(s);
		}
		m_shaders.clear();
		if (m_program) glDeleteProgram(m_program);
	}

	Shader& Shader::addShader(const std::string& source, ShaderType type) {
		uint32 shader = glCreateShader(type);

		const char* src = source.c_str();
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		int32 status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) {
			char err[1024];
			glGetShaderInfoLog(shader, 1024, nullptr, err);
			Log.error(std::string(err));
			glDeleteShader(shader);
			return *this;
		}
		
		m_shaders[shader] = source;
		glAttachShader(m_program, shader);

		return *this;
	}

	Shader& Shader::link() {
		glLinkProgram(m_program);
		int32 status;
		glGetProgramiv(m_program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			char err[1024];
			glGetProgramInfoLog(m_program, 1024, nullptr, err);
			Log.error(std::string(err));
			free();
			return *this;
		}
		return *this;
	}

	void Shader::bind() {
		switch (m_blendMode) {
			case Opaque: glDisable(GL_BLEND); break;
			case Alpha: {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			} break;
			case Add: {
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
			} break;
		}
		glDepthMask(m_depthWrite ? GL_TRUE : GL_FALSE);
		glDepthFunc(m_depthTest);
		glUseProgram(m_program);
	}

	void Shader::unbind() {
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glUseProgram(0);
	}

	int32 Shader::getUniformLocation(const std::string name) {
		auto pos = m_uniforms.find(name);
		if (pos == m_uniforms.end()) {
			uint32 loc = glGetUniformLocation(m_program, name.c_str());
			m_uniforms[name] = loc;
			return loc;
		}
		return pos->second;
	}

	intern::Uniform& Shader::operator[](const std::string& name) {
		m_uniformHandler.loc = getUniformLocation(name);
		return m_uniformHandler;
	}

	void intern::Uniform::operator =(int v) { glUniform1i(loc, v); }
	void intern::Uniform::operator =(float v) { glUniform1f(loc, v); }
	void intern::Uniform::operator =(const Vector2& v) { glUniform2f(loc, v.x, v.y); }
	void intern::Uniform::operator =(const Vector3& v) { glUniform3f(loc, v.x, v.y, v.z); }
	void intern::Uniform::operator =(const Vector4& v) { glUniform4f(loc, v.x, v.y, v.z, v.w); }
	void intern::Uniform::operator =(Matrix4 v) { glUniformMatrix4fv(loc, 1, true, v.data()); }

}