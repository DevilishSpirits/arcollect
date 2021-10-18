#include "modal.hpp"
#include "font.hpp"
#include <config.h>

extern SDL::Renderer *renderer;
std::list<Arcollect::gui::modal_stack_variant> Arcollect::gui::modal_stack;

void Arcollect::gui::modal::render_titlebar(SDL::Rect target, int window_width)
{
	// Render icon
	#if 0
	SDL::Rect icon_rect{target.x,target.y,target.h,target.h};
	renderer->Copy(TODO,NULL,&icon_rect);
	#endif
	// Render title
	const int title_border = target.h/4;
	static std::unique_ptr<Arcollect::gui::font::Renderable> cached_render;
	static SDL::Point topleft_corner;
	if (!cached_render) {
		cached_render = std::make_unique<Arcollect::gui::font::Renderable>(Arcollect::gui::font::Elements(U"Arcollect "s ARCOLLECT_VERSION_STR,target.h-2*title_border));
		topleft_corner.x = target.x+title_border;
		topleft_corner.y = target.y+title_border;
	}
	cached_render->render_tl(topleft_corner);
}
