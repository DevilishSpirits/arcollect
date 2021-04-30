#pragma once
#ifndef WITHIN_SDL2_HPP
	#error "Include SDL.hpp, not the individual headers !"
#endif
namespace SDL {
	struct Rect;
	struct Point {
		int x;
		int y;
		inline bool InRect(const SDL::Rect &rect) const {
			return SDL_PointInRect((const SDL_Point*)this,(const SDL_Rect*)&rect) == SDL_TRUE;
		}
		SDL::Point operator+(const SDL::Point& right) const {
			return {x+right.x,y+right.y};
		}
		SDL::Point operator-(const SDL::Point& right) const {
			return {x+right.x,y+right.y};
		}
	};
	struct FPoint {
		float x;
		float y;
		SDL::FPoint operator+(const SDL::FPoint& right) const {
			return {x+right.x,y+right.y};
		}
		SDL::FPoint operator-(const SDL::FPoint& right) const {
			return {x+right.x,y+right.y};
		}
	};
	struct Rect {
		int x, y;
		int w, h;
		inline bool Empty(void) const {
			return SDL_RectEmpty((const SDL_Rect*)this) == SDL_TRUE;
		}
		inline bool operator==(const SDL::Rect& right) const {
			return SDL_RectEquals((const SDL_Rect*)this,(const SDL_Rect*)&right) == SDL_TRUE;
		}
		inline bool HasIntersection(const SDL::Rect &with) const {
			return SDL_HasIntersection((const SDL_Rect*)this,(const SDL_Rect*)&with) == SDL_TRUE;
		}
		inline SDL::Rect IntersectRect(const SDL::Rect &with) const {
			SDL::Rect rect;
			SDL_IntersectRect((const SDL_Rect*)this,(const SDL_Rect*)&with,(SDL_Rect*)&rect);
			return rect;
		}
		inline SDL::Rect UnionRect(const SDL::Rect &with) const {
			SDL::Rect rect;
			SDL_UnionRect((const SDL_Rect*)this,(const SDL_Rect*)&with,(SDL_Rect*)&rect);
			return rect;
		}
		inline bool IntersectLine(int &X1, int &Y1, int &X2, int &Y2) {
			return SDL_IntersectRectAndLine((const SDL_Rect*)this,&X1,&Y1,&X2,&Y2);
		}
		inline bool IntersectLine(SDL::Point &P1, SDL::Point &P2) {
			return IntersectLine(P1.x,P1.y,P2.x,P1.y);
		}
	};
	struct FRect {
		float x, y;
		float w, h;
	};
/**
 * Calculate a minimal rectangle enclosing a set of points.
 *
 * If `clip` is not NULL then only points inside of the clipping rectangle
 * are considered.
 *
 * \param points an array of SDL_Point structures representing points to be
 *               enclosed
 * \param count the number of structures in the `points` array
 * \param clip an SDL_Rect used for clipping or NULL to enclose all points
 * \param result an SDL_Rect structure filled in with the minimal enclosing
 *               rectangle
 * \returns SDL_TRUE if any points were enclosed or SDL_FALSE if all the
 *          points were outside of the clipping rectangle.
 */
#if 0
TODO extern DECLSPEC SDL_bool SDLCALL SDL_EnclosePoints(const SDL_Point * points,
                                                   int count,
                                                   const SDL_Rect * clip,
                                                   SDL_Rect * result);
#endif
}
