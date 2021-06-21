#include "modal.hpp"
#include "font.hpp"
#include <config.h>

extern SDL::Renderer *renderer;

void Arcollect::gui::modal::render_titlebar(SDL::Rect target, int window_width)
{
	// Render icon
	SDL::Rect icon_rect{target.x,target.y,target.h,target.h};
	// TODO renderer->Copy(arcollect_icon,NULL,&icon_rect);
	// Render title
	const int title_border = target.h/4;
	Arcollect::gui::Font font;
	Arcollect::gui::TextLine title_line(font,"Arcollect " ARCOLLECT_VERSION_STR,target.h-2*title_border);
	SDL::Texture* title_line_text(title_line.render());
	SDL::Point title_line_size;
	title_line_text->QuerySize(title_line_size);
	SDL::Rect title_line_dstrect{target.x+title_border+target.h,target.y+title_border,title_line_size.x,title_line_size.y};
	renderer->Copy(title_line_text,NULL,&title_line_dstrect);
}
