#include <iostream>
#include <ctime>

#include "application.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "game_logic.h"
#include "renderer.h"
using namespace ae;

#define LIGHTS 4
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

		renderer->ambient(Vector3(0.02f));

		bunny = world->create();
		camera = world->create();
		floor = world->create();

		bunnyModel = ResourceManager::ston().load<Mesh>("model", "bunny.obj");
		floorModel = ResourceManager::ston().load<Mesh>("floor", "cube.obj");

		camera->createComponent<CameraComponent>();
		auto bcomp = bunny->createComponent<MeshComponent>(bunnyModel);
		auto mcomp = floor->createComponent<MeshComponent>(floorModel);

		floor->scale(Vector3(10.0f, 0.02f, 10.0f));
		floor->position(Vector3(0.0f, -1.0f, 0.0f));

		mcomp->material().textures[Material::SlotDiffuse] = ResourceManager::ston().load<Texture>("diff", "diff.png");
		mcomp->material().textures[Material::SlotNormal] = ResourceManager::ston().load<Texture>("norm", "norm.png");
		mcomp->material().textures[Material::SlotSpecular] = ResourceManager::ston().load<Texture>("spec", "spec.png");
		mcomp->material().shininess = 0.8f;
		mcomp->material().specular = 2.0f;

		bcomp->material().textures[Material::SlotDiffuse] = ResourceManager::ston().load<Texture>("bunny_diff", "bunny_diff.png");
		bcomp->material().textures[Material::SlotNormal] = ResourceManager::ston().load<Texture>("bunny_norm", "bunny_norm.png");
		bcomp->material().specular = 0.01f;
		bcomp->material().shininess = 0.01f;

		for (uint32 i = 0; i < LIGHTS; i++) {
			lights[i] = world->create();
			auto light = lights[i]->createComponent<LightComponent>();
			light->type(LightType::Point);
			light->radius(mathutils::random(10.0f, 30.0f));
			light->color(Vector3(
				mathutils::random(0.25f, 1.0f),
				mathutils::random(0.25f, 1.0f),
				mathutils::random(0.25f, 1.0f)
			));
		}
	}

	virtual void onUpdate(Application& app, float dt) override {
		const double fps = (1000.0 / app.millisPerFrame());
		app.setTitle(std::to_string(fps) + " fps");

		camera->position(Vector3(std::sin(angle * 0.5f) * 6.0f, 3.0f, 6.0f));
		camera->rotation(Quaternion::lookAt(camera->position(), Vector3(0.0f), Vector3(0.0f, 1.0f, 0.0f)));

		for (uint32 i = 0; i < LIGHTS; i++) {
			uint32 k = i + 1;
			auto comp = lights[i]->getComponent<LightComponent>();
			float blink = std::sin(angle * k) * 0.5f + 0.5f;
			comp->intensity(blink);
			lights[i]->position(Vector3(
				std::sin(k * angle * 0.1f) * 6.0f,
				0.5f,
				std::cos(k * angle * 0.1f) * 6.0f
			));
		}

		angle += dt;
	}

	virtual void onRender(Application& app) override {
		app.clear();

		renderer->render(world.get(), app.settings().window.width, app.settings().window.height);

		app.swapBuffers();
	};

	Mesh *bunnyModel, *floorModel;
	Texture *brickDiff, *brickSpec, *brickNorm;

	Entity *bunny, *camera, *floor;
	Entity* lights[LIGHTS];

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
