#include "modal.hpp"
#include "font.hpp"
#include <config.h>

std::list<Arcollect::gui::modal_stack_variant> Arcollect::gui::modal_stack;

void Arcollect::gui::modal::render_titlebar(render_context render_ctx)
{
	// Render icon
	#if 0
	SDL::Rect icon_rect{target.x,target.y,target.h,target.h};
	renderer->Copy(TODO,NULL,&icon_rect);
	#endif
	// Render title
	static int cached_height;
	const int title_border = render_ctx.titlebar_target.h/4;
	static std::unique_ptr<Arcollect::gui::font::Renderable> cached_render;
	if (!cached_render||(cached_height != render_ctx.titlebar_target.h)) {
		auto text = Arcollect::gui::font::Elements::build(Arcollect::gui::font::ExactFontSize(render_ctx.titlebar_target.h-2*title_border),U"Arcollect "sv ARCOLLECT_VERSION_STR);
		cached_render = std::make_unique<Arcollect::gui::font::Renderable>(text);
		cached_height = render_ctx.titlebar_target.h;
	}
	cached_render->render_cl(render_ctx.titlebar_target.x+title_border,render_ctx.titlebar_target.y,cached_height);
}
