#include "input.h"

namespace ae {

	namespace devices {
		void Keyboard::processEvent(SDL_Event e) {
			switch (e.type) {
				case SDL_KEYDOWN: {
					uint32 key = e.key.keysym.sym;
					m_state[key].pressed = true;
					m_state[key].held = true;
				} break;
				case SDL_KEYUP: {
					uint32 key = e.key.keysym.sym;
					m_state[key].released = true;
					m_state[key].held = false;
				} break;
			}
		}

		void Keyboard::reset() {
			for (auto&& [k, state] : m_state) {
				state.pressed = false;
				state.released = false;
			}
		}

		void Mouse::processEvent(SDL_Event e) {
			switch (e.type) {
				case SDL_MOUSEBUTTONDOWN: {
					uint8 button = e.button.button;
					m_state[button].pressed = true;
					m_state[button].held = true;
				} break;
				case SDL_MOUSEBUTTONUP: {
					uint8 button = e.button.button;
					m_state[button].released = true;
					m_state[button].held = false;
				} break;
				case SDL_MOUSEMOTION: {
					m_mousePosition.x = e.motion.x;
					m_mousePosition.y = e.motion.y;
				} break;
			}
		}

		void Mouse::reset() {
			for (auto&& [k, state] : m_state) {
				state.pressed = false;
				state.released = false;
			}
		}
	}

	void InputManager::processEvents() {
		m_keyboard.reset();
		m_mouse.reset();

		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_QUIT: m_shouldQuit = true; break;
			}
			m_keyboard.processEvent(e);
			m_mouse.processEvent(e);
		}
	}

}