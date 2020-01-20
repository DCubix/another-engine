#include <iostream>

#include "application.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
using namespace ae;

class SimpleApp : public ApplicationAdapter {
public:
	virtual void onSetup(Application& app) override {
		auto& win = app.settings().window;
		win.width = 1280;
		win.height = 720;
	}

	virtual void onCreate(Application& app) override {
		ResourceManager::ston().subscribe("shader", ShaderFactory("shader.glsl"));
		ResourceManager::ston().subscribe("model", MeshFactory("bunny.obj"));
		ResourceManager::ston().subscribe("texture", TextureFactory("matcap.png"));

		shader = ResourceManager::ston().load<Shader>("shader");
		model = ResourceManager::ston().load<Mesh>("model");
		texture = ResourceManager::ston().load<Texture>("texture");

		proj = Matrix4::perspective(
			mathutils::toRadians(60.0f),
			float(app.settings().window.width) / app.settings().window.height,
			0.01f, 1000.0f
		);
		view = Matrix4::translation(Vector3(0, 0, -6));
	}

	virtual void onUpdate(Application& app, float dt) override {
		const double fps = (1000.0 / app.millisPerFrame());
		app.setTitle(std::to_string(fps) + " fps");

		angle += dt;
	}

	virtual void onRender(Application& app) override {
		app.clear();

		modelMat = Matrix4::rotationY(angle);

		texture->bind();

		shader->bind();
		shader->get("proj").set(proj);
		shader->get("view").set(view);
		shader->get("model").set(modelMat);
		shader->get("tex").set(0);

		model->bind();
		model->draw(Mesh::Triangles);

		app.swapBuffers();
	};

	Matrix4 proj, view, modelMat;

	ShaderPtr shader;
	MeshPtr model;
	TexturePtr texture;

	float angle{ 0.0f };
};

int main(int argc, char** argv) {
	Application app{ new SimpleApp() };
	app.run();
	return 0;
}
