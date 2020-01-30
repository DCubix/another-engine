#ifndef INPUT_H
#define INPUT_H

#include "sdl.hpp"
#include "integer.hpp"
#include "vec_math.hpp"
#include <unordered_map>

#define SC(x) ((x) | (1 << 30))

namespace ae {

	enum class KeyCode : uint32 {
		Unknown = 0,

		Return = '\r',
		Escape = '\033',
		Backspace = '\b',
		Tab = '\t',
		Space = ' ',
		Exclaim = '!',
		DoubleQuote = '"',
		Hashtag = '#',
		Percent = '%',
		Dollar = '$',
		Ampersand = '&',
		Quote = '\'',
		LeftParen = '(',
		RightParen = ')',
		Asterisk = '*',
		Plus = '+',
		Comma = ',',
		Minus = '-',
		Period = '.',
		Slask = '/',
		Zero = '0',
		One = '1',
		Two = '2',
		Three = '3',
		Four = '4',
		Five = '5',
		Six = '6',
		Seven = '7',
		Eight = '8',
		Nine = '9',
		Colon = ':',
		Semicolon = ';',
		Less = '<',
		Equals = '=',
		Greater = '>',
		Question = '?',
		At = '@',

		LeftBracket = '[',
		Backslash = '\\',
		RightBracket = ']',
		Caret = '^',
		Underscore = '_',
		Backquote = '`',
		A = 'a',
		B = 'b',
		C = 'c',
		D = 'd',
		E = 'e',
		F = 'f',
		G = 'g',
		H = 'h',
		I = 'i',
		J = 'j',
		K = 'k',
		L = 'l',
		M = 'm',
		N = 'n',
		O = 'o',
		P = 'p',
		Q = 'q',
		R = 'r',
		S = 's',
		T = 't',
		U = 'u',
		V = 'v',
		W = 'w',
		X = 'x',
		Y = 'y',
		Z = 'z',

		Capslock = SC(SDL_SCANCODE_CAPSLOCK),

		F1 = SC(SDL_SCANCODE_F1),
		F2 = SC(SDL_SCANCODE_F2),
		F3 = SC(SDL_SCANCODE_F3),
		F4 = SC(SDL_SCANCODE_F4),
		F5 = SC(SDL_SCANCODE_F5),
		F6 = SC(SDL_SCANCODE_F6),
		F7 = SC(SDL_SCANCODE_F7),
		F8 = SC(SDL_SCANCODE_F8),
		F9 = SC(SDL_SCANCODE_F9),
		F10 = SC(SDL_SCANCODE_F10),
		F11 = SC(SDL_SCANCODE_F11),
		F12 = SC(SDL_SCANCODE_F12),

		PrintScreen = SC(SDL_SCANCODE_PRINTSCREEN),
		ScrollLock = SC(SDL_SCANCODE_SCROLLLOCK),
		Pause = SC(SDL_SCANCODE_PAUSE),
		Insert = SC(SDL_SCANCODE_INSERT),
		Home = SC(SDL_SCANCODE_HOME),
		PageUp = SC(SDL_SCANCODE_PAGEUP),
		Delete = '\177',
		End = SC(SDL_SCANCODE_END),
		PageDown = SC(SDL_SCANCODE_PAGEDOWN),
		RightArrow = SC(SDL_SCANCODE_RIGHT),
		LeftArrow = SC(SDL_SCANCODE_LEFT),
		DownArrow = SC(SDL_SCANCODE_DOWN),
		UpArrow = SC(SDL_SCANCODE_UP),

		NumLockClear = SC(SDL_SCANCODE_NUMLOCKCLEAR),
		NumpadDivide = SC(SDL_SCANCODE_KP_DIVIDE),
		NumpadMultiply = SC(SDL_SCANCODE_KP_MULTIPLY),
		NumpadMinus = SC(SDL_SCANCODE_KP_MINUS),
		NumpadPlus = SC(SDL_SCANCODE_KP_PLUS),
		NumpadEnter = SC(SDL_SCANCODE_KP_ENTER),
		NumpadOne = SC(SDL_SCANCODE_KP_1),
		NumpadTwo = SC(SDL_SCANCODE_KP_2),
		NumpadThree = SC(SDL_SCANCODE_KP_3),
		NumpadFour = SC(SDL_SCANCODE_KP_4),
		NumpadFive = SC(SDL_SCANCODE_KP_5),
		NumpadSix = SC(SDL_SCANCODE_KP_6),
		NumpadSeven = SC(SDL_SCANCODE_KP_7),
		NumpadEight = SC(SDL_SCANCODE_KP_8),
		NumpadNine = SC(SDL_SCANCODE_KP_9),
		NumpadZero = SC(SDL_SCANCODE_KP_0),
		NumpadPeriod = SC(SDL_SCANCODE_KP_PERIOD),

		Application = SC(SDL_SCANCODE_APPLICATION),
		Power = SC(SDL_SCANCODE_POWER),
		NumpadEquals = SC(SDL_SCANCODE_KP_EQUALS),
		F13 = SC(SDL_SCANCODE_F13),
		F14 = SC(SDL_SCANCODE_F14),
		F15 = SC(SDL_SCANCODE_F15),
		F16 = SC(SDL_SCANCODE_F16),
		F17 = SC(SDL_SCANCODE_F17),
		F18 = SC(SDL_SCANCODE_F18),
		F19 = SC(SDL_SCANCODE_F19),
		F20 = SC(SDL_SCANCODE_F20),
		F21 = SC(SDL_SCANCODE_F21),
		F22 = SC(SDL_SCANCODE_F22),
		F23 = SC(SDL_SCANCODE_F23),
		F24 = SC(SDL_SCANCODE_F24),
		Execute = SC(SDL_SCANCODE_EXECUTE),
		Help = SC(SDL_SCANCODE_HELP),
		Menu = SC(SDL_SCANCODE_MENU),
		Select = SC(SDL_SCANCODE_SELECT),
		Stop = SC(SDL_SCANCODE_STOP),
		Again = SC(SDL_SCANCODE_AGAIN),
		Undo = SC(SDL_SCANCODE_UNDO),
		Cut = SC(SDL_SCANCODE_CUT),
		Copy = SC(SDL_SCANCODE_COPY),
		Paste = SC(SDL_SCANCODE_PASTE),
		Find = SC(SDL_SCANCODE_FIND),
		Mute = SC(SDL_SCANCODE_MUTE),
		VolumeUp = SC(SDL_SCANCODE_VOLUMEUP),
		VolumeDown = SC(SDL_SCANCODE_VOLUMEDOWN),
		NumpadComma = SC(SDL_SCANCODE_KP_COMMA),

		AltErase = SC(SDL_SCANCODE_ALTERASE),
		SysReq = SC(SDL_SCANCODE_SYSREQ),
		Cancel = SC(SDL_SCANCODE_CANCEL),
		Clear = SC(SDL_SCANCODE_CLEAR),
		Prior = SC(SDL_SCANCODE_PRIOR),
		Return2 = SC(SDL_SCANCODE_RETURN2),
		Separator = SC(SDL_SCANCODE_SEPARATOR),
		Out = SC(SDL_SCANCODE_OUT),
		Oper = SC(SDL_SCANCODE_OPER),
		ClearAgain = SC(SDL_SCANCODE_CLEARAGAIN),

		NumpadColon = SC(SDL_SCANCODE_KP_COLON),
		NumpadHash = SC(SDL_SCANCODE_KP_HASH),

		LeftCtrl = SC(SDL_SCANCODE_LCTRL),
		LeftShift = SC(SDL_SCANCODE_LSHIFT),
		LeftAlt = SC(SDL_SCANCODE_LALT),
		RightCtrl = SC(SDL_SCANCODE_RCTRL),
		RightSHift = SC(SDL_SCANCODE_RSHIFT),
		RightAlt = SC(SDL_SCANCODE_RALT),

		Mode = SC(SDL_SCANCODE_MODE)
	};

	namespace devices {
		struct InputState {
			bool pressed{ false }, released{ false }, held{ false };
		};

		class IDevice {
		public:
			virtual void processEvent(SDL_Event e) = 0;
			virtual void reset() {}
		};

		class Keyboard : public IDevice {
		public:
			void processEvent(SDL_Event e) override;
			void reset() override;

			inline bool pressed(KeyCode key) { return m_state[uint32(key)].pressed; }
			inline bool released(KeyCode key) { return m_state[uint32(key)].released; }
			inline bool held(KeyCode key) { return m_state[uint32(key)].held; }

		private:
			std::unordered_map<uint32, InputState> m_state{};
		};

		class Mouse : public IDevice {
		public:
			void processEvent(SDL_Event e) override;
			void reset() override;

			inline bool pressed(uint8 button) { return m_state[button].pressed; }
			inline bool released(uint8 button) { return m_state[button].released; }
			inline bool held(uint8 button) { return m_state[button].held; }

			const Vector2& mousePosition() const { return m_mousePosition; }

		private:
			std::unordered_map<uint8, InputState> m_state{};
			Vector2 m_mousePosition{ 0.0f };
		};
	}
	
	class InputManager {
		friend class Application;
	public:
		InputManager() = default;
		virtual ~InputManager() = default;

		devices::Keyboard& keyboard() { return m_keyboard; }
		devices::Mouse& mouse() { return m_mouse; }

		bool shouldQuit() const { return m_shouldQuit; }

		void processEvents();

	private:
		devices::Keyboard m_keyboard{};
		devices::Mouse m_mouse{};
		bool m_shouldQuit{ false };
	};

}

#endif // INPUT_H
