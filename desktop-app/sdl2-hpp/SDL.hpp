#pragma once
#include <SDL.h>
namespace SDL {
	constexpr const Uint32 INIT_TIMER          = SDL_INIT_TIMER;
	constexpr const Uint32 INIT_AUDIO          = SDL_INIT_AUDIO;
	constexpr const Uint32 INIT_VIDEO          = SDL_INIT_VIDEO;
	constexpr const Uint32 INIT_JOYSTICK       = SDL_INIT_JOYSTICK;
	constexpr const Uint32 INIT_HAPTIC         = SDL_INIT_HAPTIC;
	constexpr const Uint32 INIT_GAMECONTROLLER = SDL_INIT_GAMECONTROLLER;
	constexpr const Uint32 INIT_EVENTS         = SDL_INIT_EVENTS;
	constexpr const Uint32 INIT_EVERYTHING     = SDL_INIT_EVERYTHING;
	
	inline int  Init(Uint32 flags)          { return SDL_Init(flags); };
	inline int  InitSubSystem(Uint32 flags) { return SDL_InitSubSystem(flags); };
	inline void QuitSubSystem(Uint32 flags) { return SDL_QuitSubSystem(flags); };
	inline void Quit(void)                  { return SDL_Quit(); };
}

#define WITHIN_SDL2_HPP
#include "SDL_pixels.hpp"
#include "SDL_error.hpp"
#include "SDL_rect.hpp"
#include "SDL_event.hpp"
#include "SDL_video.hpp"
#include "SDL_render.hpp"
#include "SDL_hint.hpp"
#undef WITHIN_SDL2_HPP
