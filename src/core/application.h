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
		virtual void onSetup(Application& app) {}
		virtual void onCreate(Application& app) {}
		virtual void onUpdate(Application& app, float dt) {}
		virtual void onRender(Application& app) {}
		virtual void onDestroy() {};
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

		void setTitle(const std::string& title);

		void run();
		double currentTime() const;

		void swapBuffers() const;
		void clear(GLenum bits = GL_COLOR_BUFFER_BIT, float r = 0.0f, float g = 0.25f, float b = 0.5f, float a = 1.0f);

		void exit() { m_shouldClose = true; }
		void restart() { exit(); m_shouldRestart = true; }

		double millisPerFrame() const { return m_msFrame; }

	private:
		std::unique_ptr<ApplicationAdapter> m_application;
		ApplicationSettings m_settings{};

		bool m_running{ false }, m_shouldClose{ false }, m_shouldRestart{ false };

		uint32 m_frames{ 0 };
		double m_msFrame{ 0.0 };

		SDL_Window* m_window;
		SDL_GLContext m_context;

		void mainLoop();
	};
}

#endif // APPLICATION_H






