#include "../sdl2-hpp/SDL.hpp"
#include <SDL_ttf.h>
#include <memory>
#include <string>
#include <arcollect-roboto.hpp>
namespace Arcollect {
	namespace gui {
		class Font {
			public:
				/** Render a line of text
				 */
				std::unique_ptr<SDL::Surface> render_line(int font_height, SDL_Color font_color, const char* text, int style = TTF_STYLE_NORMAL) {
					// FIXME This is awful
					TTF_Font *font = TTF_OpenFontRW(SDL_RWFromConstMem(Arcollect::Roboto::Light.data(),Arcollect::Roboto::Light.size()),1,font_height);
					TTF_SetFontStyle(font,style);
					std::unique_ptr<SDL::Surface> result((SDL::Surface*)TTF_RenderUTF8_Blended(font,text,font_color));
					TTF_CloseFont(font);
					return result;
				}
				/** Render a paragraph of text
				 */
				std::unique_ptr<SDL::Surface> render_paragraph(int font_height, SDL_Color font_color, Uint32 width, const char* text, int style = TTF_STYLE_NORMAL) {
					// FIXME This is awful
					TTF_Font *font = TTF_OpenFontRW(SDL_RWFromConstMem(Arcollect::Roboto::Light.data(),Arcollect::Roboto::Light.size()),1,font_height);
					TTF_SetFontStyle(font,style);
					std::unique_ptr<SDL::Surface> result((SDL::Surface*)TTF_RenderUTF8_Blended_Wrapped(font,text,font_color,width));
					TTF_CloseFont(font);
					return result;
				}
		};
		/** Cached text line
		 *
		 * This class cache a line of text avoiding costful text rerender at each frame.
		 */
		class TextLine {
			private:
				int font_height;
				SDL_Color font_color;
				int font_style;
				std::unique_ptr<SDL::Texture> cached_render;
				std::string text;
			public:
				/** The text font
				 *
				 */
				Font &font;
				/** Change the text
				 */
				void set_text(const std::string &new_text) {
					cached_render.reset();
					text = new_text;
				}
				/** Change the font height
				 */
				void set_font_height(int new_font_height) {
					cached_render.reset();
					font_height = new_font_height;
				}
				/** Change the font style
				 */
				void set_font_style(int new_font_style) {
					cached_render.reset();
					font_style = new_font_style;
				}
				/** Render the line of text
				 * \return The texture, DO NOT FREE ! Texture can be destroyed upon each function call to this #TextLine, so you cannot store the returned pointer.
				 */
				SDL::Texture* render(void);
				TextLine(Font &font, const std::string& text, int font_height, int font_style = TTF_STYLE_NORMAL, SDL_Color font_color = {255,255,255,255});
		};
		/** Cached paragraph line
		 *
		 * This class cache a line of text avoiding rerendering text at each frame.
		 */
		class TextPar {
			private:
				int font_height;
				SDL_Color font_color;
				int font_style;
				std::unique_ptr<SDL::Texture> cached_render;
				Uint32 cached_width; 
				std::string text;
			public:
				/** The text font
				 *
				 */
				Font &font;
				/** Change the text
				 */
				void set_text(const std::string &new_text) {
					cached_render.reset();
					text = new_text;
				}
				/** Change the font height
				 */
				void set_font_height(int new_font_height) {
					cached_render.reset();
					font_height = new_font_height;
				}
				/** Change the font style
				 */
				void set_font_style(int new_font_style) {
					cached_render.reset();
					font_style = new_font_style;
				}
				/** Render the line of text
				 * \param width
				 * \return The texture, DO NOT FREE ! Texture can be destroyed upon each function call to this #TextLine, so you cannot store the returned pointer.
				 */
				SDL::Texture* render(Uint32 width);
				TextPar(Font &font, const std::string& text, int font_height, int font_style = TTF_STYLE_NORMAL, SDL_Color font_color = {255,255,255,255});
		};
	}
}
