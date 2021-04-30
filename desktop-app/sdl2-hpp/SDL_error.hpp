#pragma once
#include <SDL.h>

namespace SDL {
	inline void ClearError(void) { return SDL_ClearError(); };
	inline const char* GetError(void) { return SDL_GetError(); };
	// TODO inline int SetError(const char* fmt, ...) { return SDL_SetError(const char* fmt, ...); };
}
