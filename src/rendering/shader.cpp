#include "shader.h"

#include "log.h"
#include <sstream>

#include "file_system.h"
#include "util.hpp"

namespace ae {
	Shader::Shader() {
		create();
	}

	Shader::~Shader() {
		free();
	}

	void Shader::create() {
		m_program = glCreateProgram();
	}

	void Shader::free() {
		for (auto& [s, src] : m_shaders) {
			glDeleteShader(s);
		}
		m_shaders.clear();
		if (m_program) glDeleteProgram(m_program);
	}

	void Shader::addShader(const std::string& source, ShaderType type) {
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
		}
		
		m_shaders[shader] = source;
		glAttachShader(m_program, shader);
	}

	void Shader::addUnifiedShader(const std::string& unifiedSource) {
		std::stringstream ss, outVS, outFS, outGS, outCS;
		ss << unifiedSource;

		ShaderType type = ShaderType::VertexShader;
		std::string line;
		while (std::getline(ss, line)) {
			bool discard = false;

			line = util::rtrim(line);

			if (line[0] == '#') {
				std::stringstream lineStream(line);
				std::string preprocessor; lineStream >> preprocessor;

				if (preprocessor == "#blendmode") {
					std::string mode; lineStream >> mode;
					BlendMode blend = BlendMode::Opaque;

					if (mode == "ADD") blend = BlendMode::Add;
					else if (mode == "ALPHA") blend = BlendMode::Alpha;

					blendMode(blend);

					discard = true;
				} else if (preprocessor == "#depthtest") {
					std::string mode; lineStream >> mode;
					DepthTest depth = DepthTest::Less;

					if (mode == "ALWAYS") depth = DepthTest::Always;
					else if (mode == "EQUAL") depth = DepthTest::Equal;
					else if (mode == "GREATER") depth = DepthTest::Greater;
					else if (mode == "GREATER_EQUAL") depth = DepthTest::GreaterEqual;
					else if (mode == "LESS") depth = DepthTest::Less;
					else if (mode == "LESS_EQUAL") depth = DepthTest::LessEqual;
					else if (mode == "NEVER") depth = DepthTest::Never;

					depthTest(depth);

					discard = true;
				} else if (preprocessor == "#depthwrite") {
					std::string mode; lineStream >> mode;

					depthWrite(true);
					if (mode == "FALSE" || mode == "false" || mode == "0") depthWrite(false);

					discard = true;
				}
			} else if (line == "[VERTEX_SHADER]") {
				type = ShaderType::VertexShader;
				outVS << "#line 1" << "\n";
				discard = true;
			} else if (line == "[FRAGMENT_SHADER]") {
				type = ShaderType::FragmentShader;
				outFS << "#line 1" << "\n";
				discard = true;
			} else if (line == "[GEOMETRY_SHADER]") {
				type = ShaderType::GeometryShader;
				outGS << "#line 1" << "\n";
				discard = true;
			} else if (line == "[COMPUTE_SHADER]") {
				type = ShaderType::ComputeShader;
				outCS << "#line 1" << "\n";
				discard = true;
			}

			if (!discard) {
				switch (type) {
					case VertexShader: outVS << line << "\n"; break;
					case FragmentShader: outFS << line << "\n"; break;
					case GeometryShader: outGS << line << "\n"; break;
					case ComputeShader: outCS << line << "\n"; break;
				}
			}
		}

		const std::string vs = outVS.str(),
					fs = outFS.str(),
					gs = outGS.str(),
					cs = outCS.str();
		const std::string versionHeader = "#version 440 core\n";
		if (!vs.empty()) addShader(versionHeader + vs, ShaderType::VertexShader);
		if (!fs.empty()) addShader(versionHeader + fs, ShaderType::FragmentShader);
		if (!gs.empty()) addShader(versionHeader + gs, ShaderType::GeometryShader);
		if (!cs.empty()) addShader(versionHeader + cs, ShaderType::ComputeShader);
		link();
	}

	void Shader::fromFile(const std::string& unifiedShaderFile) {
		auto file = FileSystem::ston().open(unifiedShaderFile);
		auto sz = file.size();
		std::string data(sz, '\0');

		if (file.read(data.data(), sz) == sz) {
			addUnifiedShader(data);
		}
		file.close();
	}

	void Shader::link() {
		glLinkProgram(m_program);
		int32 status;
		glGetProgramiv(m_program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			char err[1024];
			glGetProgramInfoLog(m_program, 1024, nullptr, err);
			Log.error(std::string(err));
			free();
		}
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

	intern::Uniform& Shader::get(const std::string& name) {
		m_uniformHandler.loc = getUniformLocation(name);
		return m_uniformHandler;
	}

	void intern::Uniform::set(int v) { glUniform1i(loc, v); }
	void intern::Uniform::set(float v) { glUniform1f(loc, v); }
	void intern::Uniform::set(const Vector2& v) { glUniform2f(loc, v.x, v.y); }
	void intern::Uniform::set(const Vector3& v) { glUniform3f(loc, v.x, v.y, v.z); }
	void intern::Uniform::set(const Vector4& v) { glUniform4f(loc, v.x, v.y, v.z, v.w); }
	void intern::Uniform::set(Matrix4 v) { glUniformMatrix4fv(loc, 1, true, v.data()); }

}