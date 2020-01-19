#include "application.h"

#include "log.h"

#ifndef NDEBUG
static void APIENTRY GLDebug(
		GLenum source, GLenum type, GLuint id,
		GLenum severity, GLsizei length,
		const GLchar* msg, const void* ud
) {
	std::string src = "";
	switch (source) {
		case GL_DEBUG_SOURCE_API_ARB: src = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB: src = "WINDOW SYSTEM"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: src = "SHADER COMPILER"; break;
		case GL_DEBUG_SOURCE_APPLICATION_ARB: src = "APPLICATION"; break;
		default: src = "OTHER"; break;
	}

	std::string typ = "";
	switch (type) {
		case GL_DEBUG_TYPE_ERROR_ARB: typ = "ERROR"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: typ = "DEPRECATED"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB: typ = "U.B."; break;
		case GL_DEBUG_TYPE_PERFORMANCE_ARB: typ = "PERFORMANCE"; break;
		default: src = "OTHER"; break;
	}

	switch (severity) {
		case GL_DEBUG_SEVERITY_LOW_ARB: Log.info("OpenGL(" + src + " [" + typ + "]): " + msg); break;
		case GL_DEBUG_SEVERITY_MEDIUM_ARB: Log.warn("OpenGL(" + src + " [" + typ + "]): " + msg); break;
		case GL_DEBUG_SEVERITY_HIGH_ARB: Log.error("OpenGL(" + src + " [" + typ + "]): " + msg); break;
		default: break;
	}
}
#endif

namespace ae {
	Application::Application(ApplicationAdapter* adapter) {
		m_application = std::unique_ptr<ApplicationAdapter>(adapter);
	}

	void Application::run() {
		Log.assert(m_application != nullptr, "Invalid Application Adapter. Shouldn't be null!");

		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			Log.error("SDL Error: " + std::string(SDL_GetError()));
			return;
		}

		m_application->onSetup(*this);

		Log.assert(
			!(m_settings.window.fullScreen && m_settings.window.resizable),
			"A window cannot be both full screen and resizable."
		);

		uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
		if (m_settings.window.fullScreen) flags |= SDL_WINDOW_FULLSCREEN;
		else if (m_settings.window.resizable) flags |= SDL_WINDOW_RESIZABLE;

		m_window = SDL_CreateWindow(
			m_settings.window.title.c_str(),
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			m_settings.window.width, m_settings.window.height,
			flags
		);
		if (m_window == nullptr) {
			SDL_Quit();
			Log.error("SDL Error: " + std::string(SDL_GetError()));
			return;
		}

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#ifndef NDEBUG
		int32 contextFlags = 0;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &contextFlags);
		contextFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, contextFlags);
#endif

		m_context = SDL_GL_CreateContext(m_window);

		if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
			SDL_Quit();
			Log.error("Unable to initialize OpenGL.");
			return;
		}

#ifndef NDEBUG
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallbackARB((GLDEBUGPROCARB) GLDebug, NULL);
#endif

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		mainLoop();
	}

	void Application::setTitle(const std::string& title) {
		m_settings.window.title = title;
		SDL_SetWindowTitle(m_window, title.c_str());
	}

	double Application::currentTime() const {
		return double(SDL_GetTicks()) / 1000.0;
	}

	void Application::swapBuffers() const {
		SDL_GL_SwapWindow(m_window);
	}

	void Application::clear(GLenum bits, float r, float g, float b, float a) {
		glClearColor(r, g, b, a);
		glClear(bits);
	}

	void Application::mainLoop() {
		m_application->onCreate(*this);

		const double timeStep = 1.0 / double(m_settings.engine.frameCap);
		double startTime = currentTime();
		double initialTime = startTime;
		double accum = 0.0;

		SDL_Event event;

		m_running = true;
		while (m_running) {
			bool canRender = false;
			double current = currentTime();
			double delta = current - startTime;
			startTime = current;
			accum += delta;

			// TODO: Move this into an input manager
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_QUIT: m_shouldClose = true; break;
				}
			}
			//

			while (accum >= timeStep) {
				accum -= timeStep;
				m_application->onUpdate(*this, float(timeStep));
				canRender = true;
			}

			if (canRender) {
				m_frames++;
				if (currentTime() - initialTime >= 1.0) {
					m_msFrame = 1000.0 / double(m_frames);
					m_frames = 0;
					initialTime += 1.0;
				}
				m_application->onRender(*this);
			}

			if (m_shouldClose) m_running = false;
		}
		m_application->onDestroy();

		SDL_GL_DeleteContext(m_context);
		SDL_DestroyWindow(m_window);
		SDL_Quit();

		if (m_shouldRestart) {
			run();
			m_shouldRestart = false;
			m_shouldClose = false;
		}
	}
}
