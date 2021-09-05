#pragma once
#ifndef WITHIN_SDL2_HPP
	#error "Include SDL.hpp, not the individual headers !"
#endif
namespace SDL {
	struct Color: public SDL_Color {
		constexpr Color(Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha = 255) {
			r = red;
			g = green;
			b = blue;
			a = alpha;
		}
		constexpr Color &operator=(Uint32 rgba) {
			r = (rgba >> 24) & 0xff;
			g = (rgba >> 16) & 0xff;
			b = (rgba >>  8) & 0xff;
			a = (rgba >>  0) & 0xff;
			return *this;
		}
		constexpr Color(Uint32 rgba) {
			operator=(rgba);
		}
		Color(void) = default;
		Color(const Color&) = default;
		constexpr Color(const SDL_Color &rhs) : Color(rhs.r,rhs.g,rhs.b,rhs.a) {}
		Color(Color&&) = default;
		Color &operator=(const Color&) = default;
	};
}
