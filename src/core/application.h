#ifndef APPLICATION_H
#define APPLICATION_H

#include "sdl.hpp"
#include "integer.hpp"
#include "glad.h"

#include <memory>
#include <string>

namespace ae {
	class Application;
	class ApplicationAdapter {
	public:
		virtual void on_setup(Application& app) {}
		virtual void on_create(Application& app) {}
		virtual void on_update(Application& app, float dt) {}
		virtual void on_render(Application& app) {}
		virtual void on_destroy() {};
	};

	struct ApplicationSettings {
		ApplicationSettings() = default;

		struct {
			uint32 width{ 800 }, height{ 600 };
			std::string title{ "Another Engine" };
			bool fullScreen{ false }, resizable{ false };
		} window;

		struct {
			uint32 frameCap{ 60 };
		} engine;
	};

	class Application {
	public:
		Application() = default;
		~Application() = default;

		explicit Application(ApplicationAdapter* adapter);

		ApplicationSettings& settings() { return m_settings; }

		void set_title(const std::string& title);

		void run();
		double current_time() const;

		void swap_buffers() const;
		void clear(GLenum bits = GL_COLOR_BUFFER_BIT, float r = 0.0f, float g = 0.25f, float b = 0.5f, float a = 1.0f);

		void exit() { m_shouldClose = true; }
		void restart() { exit(); m_shouldRestart = true; }

		double millis_per_frame() const { return m_msFrame; }

	private:
		std::unique_ptr<ApplicationAdapter> m_application;
		ApplicationSettings m_settings{};

		bool m_running{ false }, m_shouldClose{ false }, m_shouldRestart{ false };

		uint32 m_frames{ 0 };
		double m_msFrame{ 0.0 };

		SDL_Window* m_window;
		SDL_GLContext m_context;

		void main_loop();
	};
}

#endif // APPLICATION_H






