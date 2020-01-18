#include <iostream>

#include "application.h"
#include "mesh.h"
#include "shader.h"
using namespace ae;

class SimpleApp : public ApplicationAdapter {
public:
	virtual void onSetup(Application& app) override {
		auto& win = app.settings().window;
		win.width = 1280;
		win.height = 720;
	}

	virtual void onCreate(Application& app) override {
		mesh = Mesh()
			.create()
			.addVertex({ Vector3(-0.5f, -0.5f, 0.0f), Vector3(), Vector3(), Vector2() })
			.addVertex({ Vector3(0.5f, -0.5f, 0.0f), Vector3(), Vector3(), Vector2() })
			.addVertex({ Vector3(0.0f, 0.5f, 0.0f), Vector3(), Vector3(), Vector2() })
			.addTriangle(0, 1, 2)
			.dynamic(false)
			.calculateNormals(Mesh::Triangles)
			.build();
		
		auto vs = R"(#version 440 core
		layout (location = 0) in vec3 vPosition;
		layout (location = 1) in vec3 vNormal;
		layout (location = 2) in vec3 vTangent;
		layout (location = 3) in vec2 vTexCoord;

		out vec3 outColor;
		void main() {
			gl_Position = vec4(vPosition, 1.0);
			outColor = (vPosition / vec3(0.5)) * 0.5 + 0.5;
		})";

		auto fs = R"(#version 440 core
		out vec4 fragColor;
		in vec3 outColor;
		void main() {
			fragColor = vec4(outColor, 1.0);
		})";

		shader = Shader()
			.create()
			.addShader(vs, ShaderType::VertexShader)
			.addShader(fs, ShaderType::FragmentShader)
			.link();
	}

	virtual void onUpdate(Application& app, float dt) override {
		const double fps = (1000.0 / app.millisPerFrame());
		app.setTitle(std::to_string(fps) + " fps");
	}

	virtual void onRender(Application& app) override {
		app.clear();

		shader.bind();

		mesh.bind();
		mesh.draw(Mesh::Triangles);
		mesh.unbind();

		app.swapBuffers();
	};

	Shader shader;
	Mesh mesh;
};

int main(int argc, char** argv) {
	Application app{ new SimpleApp() };
	app.run();
	return 0;
}