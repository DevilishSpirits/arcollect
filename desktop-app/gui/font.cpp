#include <arcollect-roboto.hpp>
#include "font.hpp"
#include <unordered_map>

extern SDL::Renderer *renderer;

TTF_Font *Arcollect::gui::font::query_font(Uint32 font_size)
{
	Uint32 key{font_size};
	static std::unordered_map<decltype(key),TTF_Font*> cache;
	auto iter = cache.find(key);
	if (iter == cache.end()) {
		TTF_Font* new_font = TTF_OpenFontRW(SDL_RWFromConstMem(Arcollect::Roboto::Light.data(),Arcollect::Roboto::Light.size()),1,font_size);
		cache.emplace(key,new_font);
		return new_font;
	} else return iter->second;
}

Arcollect::gui::font::Renderable::Renderable(const Elements& elements)
{
	// FIXME This is awful
	const char* text = NULL;
	switch (elements[0].index()) {
		case ELEMENT_STRING: {
			text = std::get<std::string>(elements[0]).data();
		} break;
		case ELEMENT_STRING_VIEW: {
			text = std::get<std::string_view>(elements[0]).data();
		} break;
	}
	
	std::unique_ptr<SDL::Surface> surf((SDL::Surface*)TTF_RenderUTF8_Blended(query_font(elements.initial_height),text,elements.initial_color));
	result_size.x = ((SDL_Surface&)*surf).w;
	result_size.y = ((SDL_Surface&)*surf).h;
	result.reset(SDL::Texture::CreateFromSurface(renderer,surf.get()));
}
Arcollect::gui::font::Renderable::Renderable(const Elements& elements, Uint32 wrap_width)
{
	// FIXME This is awful
	const char* text = NULL;
	switch (elements[0].index()) {
		case ELEMENT_STRING: {
			text = std::get<std::string>(elements[0]).data();
		} break;
		case ELEMENT_STRING_VIEW: {
			text = std::get<std::string_view>(elements[0]).data();
		} break;
	}
	
	TTF_Font *font = TTF_OpenFontRW(SDL_RWFromConstMem(Arcollect::Roboto::Light.data(),Arcollect::Roboto::Light.size()),1,elements.initial_height);
	std::unique_ptr<SDL::Surface> surf((SDL::Surface*)TTF_RenderUTF8_Blended_Wrapped(font,text,elements.initial_color,wrap_width));
	TTF_CloseFont(font);
	result_size.x = ((SDL_Surface&)*surf).w;
	result_size.y = ((SDL_Surface&)*surf).h;
	result.reset(SDL::Texture::CreateFromSurface(renderer,surf.get()));
}
static inline Arcollect::gui::font::Elements ArcollectRenderableconstcharstarHelper(const char* text, int font_size)
{
	Arcollect::gui::font::Elements elements;
	elements << std::string_view(text);
	elements.initial_height = font_size;
	return elements;
}
Arcollect::gui::font::Renderable::Renderable(const char* text, int font_size) : Renderable(ArcollectRenderableconstcharstarHelper(text,font_size))
{
}
Arcollect::gui::font::Renderable::Renderable(const char* text, int font_size, Uint32 wrap_width) : Renderable(ArcollectRenderableconstcharstarHelper(text,font_size),wrap_width)
{
}

void Arcollect::gui::font::Renderable::render_tl(int x, int y)
{
	SDL::Rect dest{x,y,size().x,size().y};
	renderer->Copy(result.get(),NULL,&dest);
}
