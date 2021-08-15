#pragma once
#include <utility>
#ifndef WITHIN_SDL2_HPP
	#error "Include SDL.hpp, not the individual headers !"
#endif
namespace SDL {
	struct Renderer;
	struct Surface: public SDL_Surface {
		inline void operator delete(void* surface) {
			SDL_FreeSurface((SDL_Surface*)surface);
		}
	};
	struct Texture {
		inline static Texture* CreateFromSurface(Renderer* renderer, Surface *surface) {
			return (Texture*)SDL_CreateTextureFromSurface((SDL_Renderer*)renderer,(SDL_Surface*)surface);
		}
		int QueryTexture(Uint32 *format, int *access = NULL, int *w = NULL, int *h = NULL) {
			return SDL_QueryTexture((SDL_Texture*)this,format,access,w,h);
		}
		int QueryTexture(Uint32 *format, int *access, SDL::Point &size) {
			return QueryTexture(format,access,&size.x,&size.y);
		}
		int QuerySize(SDL::Point &size) {
			return QueryTexture(NULL,NULL,size);
		}
		inline void operator delete(void* renderer) {
			SDL_DestroyTexture((SDL_Texture*)renderer);
		}
	};
	typedef SDL_BlendMode BlendMode;
	constexpr const auto BLENDMODE_NONE  = SDL_BLENDMODE_NONE;
	constexpr const auto BLENDMODE_BLEND = SDL_BLENDMODE_BLEND;
	constexpr const auto BLENDMODE_ADD   = SDL_BLENDMODE_ADD;
	constexpr const auto BLENDMODE_MOD   = SDL_BLENDMODE_MOD;
	struct Renderer {
		typedef SDL_RendererFlip Flip;
		inline int SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
			return SDL_SetRenderDrawColor((SDL_Renderer*)this,r,g,b,a);
		}
		inline int DrawLine(int x1, int y1, int x2, int y2) {
			return SDL_RenderDrawLine((SDL_Renderer*)this,x1,y1,x2,y2);
		};
		inline int DrawLine(const SDL_Point p1, const SDL_Point p2) {
			return SDL_RenderDrawLine((SDL_Renderer*)this,p1.x,p1.y,p2.x,p2.y);
		};
		inline int DrawPoint(int x, int y) {
			return SDL_RenderDrawPoint((SDL_Renderer*)this,x,y);
		};
		inline int DrawPoint(const SDL_Point p1) {
			return DrawPoint(p1.x,p1.y);
		};
		inline int DrawRect(const SDL::Rect rect) {
			return SDL_RenderDrawRect((SDL_Renderer*)this,(const SDL_Rect*)&rect);
		};
		inline int FillRect(const SDL::Rect rect) {
			return SDL_RenderFillRect((SDL_Renderer*)this,(const SDL_Rect*)&rect);
		};
		inline int FillRect(void) {
			return SDL_RenderFillRect((SDL_Renderer*)this,NULL);
		};
		
		inline int Copy(Texture *texture, const Rect *srcrect, const Rect *dstrect) {
			return SDL_RenderCopy((SDL_Renderer*)this,(SDL_Texture*)texture,(SDL_Rect*)srcrect,(SDL_Rect*)dstrect);
		}
		inline int Copy(Texture *texture, const Rect *srcrect, const Rect *dstrect, const double angle, const Point *center, const Flip flip) {
			return SDL_RenderCopyEx((SDL_Renderer*)this,(SDL_Texture*)texture,(SDL_Rect*)srcrect,(SDL_Rect*)dstrect,angle,(SDL_Point*)center,(SDL_RendererFlip)flip);
		}
		
		inline int GetOutputSize(int *w, int *h) {
			return SDL_GetRendererOutputSize((SDL_Renderer*)this,w,h);
		}
		inline int GetOutputSize(int &w, int &h) {
			return GetOutputSize(&w,&h);
		}
		inline int GetOutputSize(SDL::Point &point) {
			return GetOutputSize(point.x,point.y);
		}
		
		inline int SetDrawBlendMode(SDL_BlendMode blendMode) {
			return SDL_SetRenderDrawBlendMode((SDL_Renderer*)this,(SDL_BlendMode)blendMode);
		}
		
		inline void Present(void) {
			return SDL_RenderPresent((SDL_Renderer*)this);
		}
		inline int Clear(void) {
			return SDL_RenderClear((SDL_Renderer*)this);
		}
		
		inline void operator delete(void* renderer) {
			SDL_DestroyRenderer((SDL_Renderer*)renderer);
		}
	};
	inline int CreateWindowAndRenderer(int width, int height, Uint32 window_flags, SDL_Window *&window, SDL::Renderer *&renderer) {
		return SDL_CreateWindowAndRenderer(width,height,window_flags,&window,(SDL_Renderer**)&renderer);
	}
}
/*
    SDL_ComposeCustomBlendMode
    SDL_CreateSoftwareRenderer
    SDL_CreateTexture
    SDL_CreateTextureFromSurface
    SDL_CreateWindowAndRenderer
    SDL_GL_BindTexture
    SDL_GL_UnbindTexture
    SDL_GetNumRenderDrivers
    SDL_GetRenderDrawBlendMode
    SDL_GetRenderDrawColor
    SDL_GetRenderDriverInfo
    SDL_GetRenderTarget
    SDL_GetRenderer
    SDL_GetRendererInfo
    SDL_GetTextureAlphaMod
    SDL_GetTextureBlendMode
    SDL_GetTextureColorMod
    SDL_LockTexture
    SDL_QueryTexture
    SDL_RenderCopy
    SDL_RenderCopyEx
    SDL_RenderDrawLines
    SDL_RenderDrawPoint
    SDL_RenderDrawPoints
    SDL_RenderDrawRect
    SDL_RenderDrawRects
    SDL_RenderFillRects
    SDL_RenderGetClipRect
    SDL_RenderGetIntegerScale
    SDL_RenderGetLogicalSize
    SDL_RenderGetScale
    SDL_RenderGetViewport
    SDL_RenderIsClipEnabled
    SDL_RenderReadPixels
    SDL_RenderSetClipRect
    SDL_RenderSetIntegerScale
    SDL_RenderSetLogicalSize
    SDL_RenderSetScale
    SDL_RenderSetViewport
    SDL_RenderTargetSupported
    SDL_SetRenderTarget
    SDL_SetTextureAlphaMod
    SDL_SetTextureBlendMode
    SDL_SetTextureColorMod
    SDL_UnlockTexture
    SDL_UpdateTexture
    SDL_UpdateYUVTexture
*/
