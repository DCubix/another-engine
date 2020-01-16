#include <iostream>

#include "application.h"
using namespace ae;

class SimpleApp : public ApplicationAdapter {
public:
	virtual void on_setup(Application& app) override {
		auto& win = app.settings().window;
		win.width = 1280;
		win.height = 720;
	}

	virtual void on_update(Application& app, float dt) override {
		const double fps = (1000.0 / app.millis_per_frame());
		app.set_title(std::to_string(fps) + " fps");
	}

	virtual void on_render(Application& app) override {
		app.clear();

		app.swap_buffers();
	};
};

int main(int argc, char** argv) {
	Application app{ new SimpleApp() };
	app.run();
	return 0;
}