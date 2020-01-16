#ifndef SDL_HPP
#define SDL_HPP

#if defined(__has_include)
#	if __has_include("SDL2/SDL.h")
#		include "SDL2/SDL.h"
#	else
#		include "SDL.h"
#	endif
#else
#		include "SDL.h"
#endif

#endif // SDL_HPP
