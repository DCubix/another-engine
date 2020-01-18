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
		
	}

	virtual void onUpdate(Application& app, float dt) override {
		const double fps = (1000.0 / app.millisPerFrame());
		app.setTitle(std::to_string(fps) + " fps");
	}

	virtual void onRender(Application& app) override {
		app.clear();

		app.swapBuffers();
	};

};

int main(int argc, char** argv) {
	Application app{ new SimpleApp() };
	app.run();
	return 0;
}