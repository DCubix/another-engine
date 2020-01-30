#include <iostream>
#include <ctime>

#include "application.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "game_logic.h"
#include "renderer.h"
using namespace ae;

class SimpleApp : public ApplicationAdapter {
public:
	virtual void onSetup(Application& app) override {
		auto& win = app.settings().window;
		win.width = 1280;
		win.height = 720;
	}

	virtual void onCreate(Application& app) override {
		world = std::make_unique<EntityWorld>();
		renderer = std::make_unique<Renderer>();

		renderer->ambient(Vector3(0.001f));

		bunny = world->create();
		camera = world->create();
		floor = world->create();

		bunnyModel = ResourceManager::ston().load<Mesh>("model", "bunny.obj");
		floorModel = ResourceManager::ston().load<Mesh>("floor", "cube.obj");

		camera->createComponent<CameraComponent>();
		auto bcomp = bunny->createComponent<MeshComponent>(bunnyModel);
		auto mcomp = floor->createComponent<MeshComponent>(floorModel);

		floor->scale(Vector3(10.0f, 0.02f, 10.0f));
		floor->position(Vector3(0.0f, -0.7f, 0.0f));

		mcomp->material().textures[Material::SlotDiffuse] = ResourceManager::ston().load<Texture>("diff", "diff.png");
		mcomp->material().textures[Material::SlotNormal] = ResourceManager::ston().load<Texture>("norm", "norm.png");
		mcomp->material().textures[Material::SlotSpecular] = ResourceManager::ston().load<Texture>("spec", "spec.png");
		mcomp->material().shininess = 0.8f;
		mcomp->material().specular = 1.0f;

		bcomp->material().textures[Material::SlotDiffuse] = ResourceManager::ston().load<Texture>("bunny_diff", "bunny_diff.png");
		bcomp->material().textures[Material::SlotNormal] = ResourceManager::ston().load<Texture>("bunny_norm", "bunny_norm.png");
		bcomp->material().specular = 0.01f;
		bcomp->material().shininess = 0.01f;

		Entity* light = world->create();
		light->position(Vector3(7.0f, 8.0f, 7.0f));
		auto lcomp = light->createComponent<LightComponent>();
		lcomp->type(LightType::Spot);
		lcomp->radius(40.0f);
		lcomp->cutOff(consts::QuarPi * 0.25f);
		lcomp->color(Vector3(1.0f, 0.15f, 0.0f));
		light->rotation(Quaternion::lookAt(light->position(), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)));

		light = world->create();
		light->position(Vector3(-7.0f, 8.0f, 7.0f));
		auto lcomp2 = light->createComponent<LightComponent>();
		lcomp2->type(LightType::Spot);
		lcomp2->radius(40.0f);
		lcomp2->cutOff(consts::QuarPi * 0.25f);
		lcomp->color(Vector3(0.0f, 0.25f, 1.0f));
		light->rotation(Quaternion::lookAt(light->position(), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)));

	}

	virtual void onUpdate(Application& app, float dt) override {
		const double fps = (1000.0 / app.millisPerFrame());
		app.setTitle(std::to_string(fps) + " fps");

		camera->position(Vector3(std::sin(angle * 0.5f) * 6.0f, 3.0f, 6.0f));
		camera->rotation(Quaternion::lookAt(camera->position(), Vector3(0.0f), Vector3(0.0f, 1.0f, 0.0f)));

		if (app.input().keyboard().held(KeyCode::Space)) {
			bunny->rotation(bunny->rotation() * Quaternion::axisAngle(Vector3(0.0f, 1.0f, 0.0f), dt * 2.0f));
		}

		angle += dt;
	}

	virtual void onRender(Application& app) override {
		app.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, 0.0f, 0.0f, 0.0f, 1.0f);

		renderer->render(world.get(), app.settings().window.width, app.settings().window.height);

		app.swapBuffers();
	};

	Mesh *bunnyModel, *floorModel;
	Texture *brickDiff, *brickSpec, *brickNorm;

	Entity *bunny, *camera, *floor;

	std::unique_ptr<EntityWorld> world;
	std::unique_ptr<Renderer> renderer;

	float angle{ 0.0f };
};

int main(int argc, char** argv) {
	srand(time(NULL));
	Application app{ new SimpleApp() };
	app.run();
	return 0;
}
