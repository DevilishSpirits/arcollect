/* Arcollect -- An artwork collection manager
 * Copyright (C) 2021 DevilishSpirits (aka D-Spirits or Luc B.)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "../config.hpp"
#include "artwork.hpp"
#include "artwork-loader.hpp"
#include "account.hpp"
#include "db.hpp"
#include <arcollect-paths.hpp>
#include <arcollect-sqls.hpp>
#include <OpenImageIO/imageio.h>
#include <iostream>
#include <fstream>
#define CMSREGISTER // Remove warnings about 'register' keyword
#include "lcms2.h"
#include <cstdlib>

static std::unordered_map<sqlite_int64,std::shared_ptr<Arcollect::db::artwork>> artworks_pool;
std::list<std::reference_wrapper<Arcollect::db::artwork>> Arcollect::db::artwork::last_rendered;
extern SDL::Renderer *renderer;
extern bool debug_icc_profile;
// FIXME This as global is bad
extern cmsHPROFILE    cms_screenprofile;
cmsHPROFILE cms_screenprofile = NULL; // Filled in Arcollect::gui::init()
extern SDL_Surface* IMG_Load(const char* path);
SDL_Surface* IMG_Load(const char* path)
{
	auto image = OIIO::ImageInput::open(std::string(path));
	if (!image) {
		std::cerr << "Failed to open " << path << ". " << OIIO::geterror();
		return NULL;
	}
	const OIIO::ImageSpec &spec = image->spec();
	int pixel_format;
	switch (spec.nchannels) {
		case 4:pixel_format = SDL_PIXELFORMAT_ABGR8888;break;
		case 3:pixel_format = SDL_PIXELFORMAT_BGR888;break;
		case 1:pixel_format = SDL_PIXELFORMAT_BGR888;break;
		default:return NULL;
	}
	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0,spec.width,spec.height,spec.nchannels*8,pixel_format);
	if (!surface) {
		std::cerr << "Failed to load pixels from " << path << ". " << image->geterror();
		return NULL;
	}
	image->read_scanlines(0,0,0,spec.height-1,0,0,spec.nchannels,OIIO::TypeDesc::UINT8,surface->pixels,surface->format->BytesPerPixel,surface->pitch);
	if (spec.nchannels == 1) {
		// FIXME Optimize that
		// Monochrome picture, populate green and blue
	image->read_scanlines(0,0,0,spec.height-1,0,0,spec.nchannels,OIIO::TypeDesc::UINT8,&((char*)surface->pixels)[1],surface->format->BytesPerPixel,surface->pitch);
	image->read_scanlines(0,0,0,spec.height-1,0,0,spec.nchannels,OIIO::TypeDesc::UINT8,&((char*)surface->pixels)[2],surface->format->BytesPerPixel,surface->pitch);
	}
	
	// Set pixel format for lcms2
	cmsUInt32Number cms_pixel_format;
	switch (surface->format->BytesPerPixel) {
		case 4:cms_pixel_format = TYPE_RGBA_8;break;
		case 3:cms_pixel_format = TYPE_RGB_8;break;
		case 1:cms_pixel_format = TYPE_RGB_8;break;
	}
	
	// Get image profile
	cmsHPROFILE image_profile = NULL;
	const OIIO::ParamValue *icc_profile = spec.find_attribute("ICCProfile");
	if (debug_icc_profile)
		std::cerr << path << ":";
	if (icc_profile) {
		image_profile = cmsOpenProfileFromMem(icc_profile->data(),icc_profile->datasize());
		if (debug_icc_profile) {
			std::cerr << " embed ICC profile ";
			if (image_profile) {
				char description[64];
				char manufacturer[64];
				char model[64];
				char copyright[64];
				cmsGetProfileInfoASCII(image_profile,cmsInfoDescription,cmsNoLanguage,cmsNoCountry,description,sizeof(description));
				cmsGetProfileInfoASCII(image_profile,cmsInfoManufacturer,cmsNoLanguage,cmsNoCountry,manufacturer,sizeof(manufacturer));
				cmsGetProfileInfoASCII(image_profile,cmsInfoModel,cmsNoLanguage,cmsNoCountry,model,sizeof(model));
				cmsGetProfileInfoASCII(image_profile,cmsInfoCopyright,cmsNoLanguage,cmsNoCountry,copyright,sizeof(copyright));
				std::cerr << "from " << manufacturer << " \"" << model << "\" (" << copyright << "): " << description << ".";
			} else std::cerr << "that failed to load.";
		}
	}
	if (!image_profile) {
		// oiio:ColorSpace string
		static const cmsCIExyY D65 = {0.3127,0.3291,1};
		static const cmsCIExyYTRIPLE     sRGBPrimaries = {{0.64,0.33,1},{0.30,0.60,1},{0.15,0.06,1}};
		static const cmsCIExyYTRIPLE AdobeRGBPrimaries = {{0.64,0.33,1},{0.21,0.71,1},{0.15,0.06,1}};
		struct GammaTriplet {
			cmsToneCurve *curve;
			cmsToneCurve* triplet[3];
			GammaTriplet(cmsFloat64Number gamma) : curve(cmsBuildGamma(NULL,gamma)), triplet{curve,curve,curve} {}
			operator cmsToneCurve**(void) {
				return triplet;
			};
		};
		static GammaTriplet gamma1_0(1.0);
		static GammaTriplet gamma2_2(2.2);
		static GammaTriplet gamma2_4(2.4);
		
		auto color_space = spec.get_string_attribute("oiio:ColorSpace","no");
		if (debug_icc_profile)
			std::cerr << " OIIO report " << color_space << " color-space.";
		if (color_space == "Linear") {
			if (debug_icc_profile)
				std::cerr << " sRGB with gamma=1.";
			image_profile = cmsCreateRGBProfile(&D65,&sRGBPrimaries,gamma1_0);
		}
		else if (color_space == "Rec709") {
			if (debug_icc_profile)
				std::cerr << " sRGB with gamma=2.4.";
			image_profile = cmsCreateRGBProfile(&D65,&sRGBPrimaries,gamma2_4);
		}
		// TODO else if (color_space == "ACES")
		else if (color_space == "AdobeRGB") {
			if (debug_icc_profile)
				std::cerr << " Use AdobeRGB.";
			image_profile = cmsCreateRGBProfile(&D65,&AdobeRGBPrimaries,gamma2_2);
		}
		// TODO else if (color_space == "KodakLog")
		// else if (color_space == "sRGB") // This is the fallback anyway
	}
	if (!image_profile) {
		// Fallback to sRGB
		if (debug_icc_profile)
			std::cerr << " Fallback to sRGB.";
		image_profile = cmsCreate_sRGBProfile();
	}
	if (cms_screenprofile) {
		cmsHTRANSFORM hTransform = cmsCreateTransform(image_profile,cms_pixel_format,cms_screenprofile,cms_pixel_format,Arcollect::config::littlecms_intent,Arcollect::config::littlecms_flags);
		if (hTransform) {
			if (debug_icc_profile)
				std::cerr << " Colors are managed.";
			for (int y = 0; y < spec.height; y++) {
				char* pixels = static_cast<char*>(surface->pixels) + y*surface->pitch;
				cmsDoTransform(hTransform,pixels,pixels,spec.width);
			}
			cmsDeleteTransform(hTransform);
		} else if (debug_icc_profile)
			std::cerr << " cmsCreateTransform() failed!";
	}
	if (debug_icc_profile)
		std::cerr << std::endl;
	cmsCloseProfile(image_profile);
	return surface;
}

Arcollect::db::artwork::artwork(Arcollect::db::artwork_id art_id) :
	data_version(-2),
	art_id(art_id)
{
	db_sync();
}
std::shared_ptr<Arcollect::db::artwork> &Arcollect::db::artwork::query(Arcollect::db::artwork_id art_id)
{
	std::shared_ptr<Arcollect::db::artwork> &pointer = artworks_pool.try_emplace(art_id).first->second;
	if (!pointer)
		pointer = std::shared_ptr<Arcollect::db::artwork>(new Arcollect::db::artwork(art_id));
	return pointer;
}

void Arcollect::db::artwork::queue_for_load(void)
{
	Arcollect::db::artwork_loader::pending_main.push_back(query(art_id));
	last_render_timestamp = SDL_GetTicks();
	if (load_state == UNLOADED)
		load_state = LOAD_SCHEDULED;
}
SDL::Surface *Arcollect::db::artwork::load_surface(void) const
{
	switch (artwork_type) {
		case ARTWORK_TYPE_IMAGE: {
			const std::filesystem::path path = Arcollect::path::artwork_pool / std::to_string(art_id);
			return (SDL::Surface*)IMG_Load(path.string().c_str());
		} break;
		default: {
			// Load the thumbnail
			const std::filesystem::path path = Arcollect::path::artwork_pool / (std::to_string(art_id)+".thumbnail");
			return (SDL::Surface*)IMG_Load(path.string().c_str());
		} break;
	}
}
void Arcollect::db::artwork::texture_loaded(std::unique_ptr<SDL::Texture> &texture)
{
	// Set texture
	text = std::move(texture);
	// Set size if missing in the DB
	if (!art_size.x || !art_size.y) {
		// Read size
		text->QuerySize(art_size);
		// Set size in the database
		std::unique_ptr<SQLite3::stmt> set_size_stmt;
		database->prepare("UPDATE artworks SET art_width=?, art_height=? WHERE art_artid = ?;",set_size_stmt); // TODO Error checking
		set_size_stmt->bind(1,art_size.x);
		set_size_stmt->bind(2,art_size.y);
		set_size_stmt->bind(3,art_id);
		set_size_stmt->step();
		database->exec("COMMIT;");
	}
	// Set last_rendered_iterator
	last_rendered.emplace_front(*this);
	last_rendered_iterator = last_rendered.begin();
	// Increase image memory usage
	Arcollect::db::artwork_loader::image_memory_usage += image_memory();
	
	load_state = LOADED;
}
void Arcollect::db::artwork::texture_unload(void)
{
	// Free texture
	text.reset();
	// Erase myself from last_rendered
	last_rendered.erase(last_rendered_iterator);
	// Decrease image memory usage
	Arcollect::db::artwork_loader::image_memory_usage -= image_memory();
	
	load_state = UNLOADED;
}
std::unique_ptr<SDL::Texture> &Arcollect::db::artwork::query_texture(void)
{
	static std::unique_ptr<SDL::Texture> null_text;
	// Enforce rating
	// Note: Returning null_text IS a bug. This is a safety to avoid accidents.
	if (art_rating > Arcollect::config::current_rating)
		return null_text;
	
	if (!text)
		queue_for_load();
	last_render_timestamp = SDL_GetTicks();
	
	return text;
}

const Arcollect::gui::font::Elements &Arcollect::db::artwork::query_font_elements(void)
{
	if (artwork_text_elements.empty()) {
		// Load the artwork
		const std::filesystem::path path = Arcollect::path::artwork_pool / std::to_string(art_id);
		switch (artwork_subtype.text) {
			case artwork_subtype.ARTWORK_TYPE_TEXT_PLAIN: {
				// TODO Use charset from the MIME type. Assume all UTF-8 for now.
				std::string file_content;
				std::getline(std::ifstream(path),file_content,'\0');
				artwork_text_elements << std::move(file_content);
			} break;
		}
	}
	return artwork_text_elements;
}

int Arcollect::db::artwork::render(const SDL::Rect *dstrect)
{
	std::unique_ptr<SDL::Texture> &text = query_texture();
	if (text) {
		// Bump me in last_rendered list
		last_rendered.splice(last_rendered.begin(),last_rendered,last_rendered_iterator);
		// Render
		return renderer->Copy(text.get(),NULL,dstrect);
	}else {
		// Render a placeholder
		renderer->SetDrawColor(0,0,0,192);
		renderer->FillRect(*dstrect);
		if (dstrect) {
			SDL::Rect placeholder_rect{dstrect->x,dstrect->y,dstrect->h/4,dstrect->h/8};
			if (placeholder_rect.w > dstrect->w*3/4) {
				placeholder_rect.w = dstrect->w*3/4;
			}
			placeholder_rect.x += (dstrect->w-placeholder_rect.w)/2;
			placeholder_rect.y += (dstrect->h-placeholder_rect.h)/2;
			int bar_count;
			const int max_bar_count = 3;
			switch (load_state) {
				case UNLOADED: {
					bar_count = 0;
				} break;
				case LOAD_SCHEDULED:
				case LOAD_PENDING: {
					bar_count = 1;
				} break;
				case READING_PIXELS: {
					bar_count = 2;
				} break;
				case SURFACE_AVAILABLE:
				case LOADED: {
					bar_count = 3;
				} break;
			}
			renderer->SetDrawColor(255,255,255,128);
			renderer->DrawRect(placeholder_rect);
			// Draw inside bars
			auto border = placeholder_rect.w/8;
			placeholder_rect.x += border;
			placeholder_rect.y += border;
			placeholder_rect.h -= border*2;
			placeholder_rect.w -= border*(1+max_bar_count);
			placeholder_rect.w /= max_bar_count;
			const SDL_Color bar_colors[] = {
				{0,0,0,0},
				{128,128,128,255},
				{0,255,0,255},
				{0,255,0,255},
			};
			for (auto i = bar_count; i; i--) {
				const SDL_Color &color = bar_colors[bar_count];
				renderer->SetDrawColor((color.r+(rand()%256))/2,(color.g+(rand()%256))/2,(color.b+(rand()%256))/2,64);
				renderer->FillRect(placeholder_rect);
				renderer->SetDrawColor(color.r,color.g,color.b,128);
				renderer->DrawRect(placeholder_rect);
				placeholder_rect.x += placeholder_rect.w+border;
			}
		}
		return 0;
	}
}

static std::string column_string_default(std::unique_ptr<SQLite3::stmt> &stmt, int col)
{
	if (stmt->column_type(col) == SQLITE_NULL)
		return "";
	else return stmt->column_string(col);
}

void Arcollect::db::artwork::db_sync(void)
{
	if (data_version != Arcollect::data_version) {
		std::unique_ptr<SQLite3::stmt> stmt;
		database->prepare("SELECT art_title, art_desc, art_source, art_width, art_height, art_rating, art_mimetype FROM artworks WHERE art_artid = ?;",stmt); // TODO Error checking
		stmt->bind(1,art_id);
		if (auto code = stmt->step() == SQLITE_ROW) {
			art_title  = column_string_default(stmt,0);
			art_desc   = column_string_default(stmt,1);
			art_source = stmt->column_string(2);
			// Load picture size if unknow
			if (!art_size.x || !art_size.y) {
				art_size.x = stmt->column_int64(3);
				art_size.y = stmt->column_int64(4);
			}
			
			art_rating = static_cast<Arcollect::config::Rating>(stmt->column_int64(5));
			
			// Detect artwork type
			art_mimetype = stmt->column_string(6);
			if ((art_mimetype.size() >= 6)
			 &&(art_mimetype[0] == 'i')
			 &&(art_mimetype[1] == 'm')
			 &&(art_mimetype[2] == 'a')
			 &&(art_mimetype[3] == 'g')
			 &&(art_mimetype[4] == 'e')
			 &&(art_mimetype[5] == '/'))
				artwork_type = ARTWORK_TYPE_IMAGE;
			else if ((art_mimetype.size() >= 10)
			 &&(art_mimetype[0] == 't')
			 &&(art_mimetype[1] == 'e')
			 &&(art_mimetype[2] == 'x')
			 &&(art_mimetype[3] == 't')
			 &&(art_mimetype[4] == '/')
			 &&(art_mimetype[5] == 'p')
			 &&(art_mimetype[6] == 'l')
			 &&(art_mimetype[7] == 'a')
			 &&(art_mimetype[8] == 'i')
			 &&(art_mimetype[9] == 'n')) {
				artwork_type = ARTWORK_TYPE_TEXT;
				artwork_subtype.text = artwork_subtype.ARTWORK_TYPE_TEXT_PLAIN;
			} else artwork_type = ARTWORK_TYPE_UNKNOWN;
			
			data_version = Arcollect::data_version;
		} else {
		}
		
		linked_accounts.clear();
	}
}

const std::vector<std::shared_ptr<Arcollect::db::account>> &Arcollect::db::artwork::get_linked_accounts(const std::string &link)
{
	db_sync();
	auto iterbool = linked_accounts.emplace(link,std::vector<std::shared_ptr<account>>());
	std::vector<std::shared_ptr<account>> &result = iterbool.first->second;
	if (iterbool.second) {
		std::unique_ptr<SQLite3::stmt> stmt;
		database->prepare("SELECT acc_arcoid FROM art_acc_links WHERE art_artid = ? AND artacc_link = ?;",stmt); // TODO Error checking
		;
		stmt->bind(1,art_id);
		stmt->bind(2,link.c_str());
		while (stmt->step() == SQLITE_ROW) {
			result.emplace_back(Arcollect::db::account::query(stmt->column_int64(0)));
			
			data_version = Arcollect::data_version;
		}
	}
	return result;
}

int Arcollect::db::artwork::db_delete(void)
{
	int code;
	std::cerr << "Deleting \"" << art_title << "\" (" << art_id << ")" << std::endl;
	if ((code = database->exec("BEGIN IMMEDIATE;")) != SQLITE_OK) {
		switch (code) {
			case SQLITE_BUSY: {
				std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), \"BEGIN IMMEDIATE;\" failed with SQLITE_BUSY. Another is writing on the database. Abort." << std::endl;
			} return SQLITE_BUSY;
			default: {
				std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), \"BEGIN IMMEDIATE;\" failed: " << database->errmsg() << ". Ignoring..." << std::endl;
			} break;
		}
	}
	std::unique_ptr<SQLite3::stmt> stmt;
	
	// Run all substeps from 'delete_artwork.sql'
	const char *zSql = Arcollect::db::sql::delete_artwork.c_str();
	int substep = 0;
	while (*zSql) {
		substep++;
		if (database->prepare(zSql,-1,stmt,zSql) != SQLITE_OK) {
			std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to prepare substep " << substep << ": " << database->errmsg() << ". Rollback." << std::endl;
			database->exec("ROLLBACK;");
			return SQLITE_ERROR;
		}
		if (stmt->bind(1,art_id) != SQLITE_OK) {
			std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to bind art_artid at substep " << substep << ": " << database->errmsg() << ". Rollback." << std::endl;
			database->exec("ROLLBACK;");
			return SQLITE_ERROR;
		}
		if (stmt->step() != SQLITE_DONE) {
			std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to run substep " << substep << ": " << database->errmsg() << ". Rollback." << std::endl;
			database->exec("ROLLBACK;"); // TODO Error checkings
			return SQLITE_ERROR;
		}
	}
	
	// Commit changes
	if (database->exec("COMMIT;") != SQLITE_OK) {
		std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to commit changes: " << database->errmsg() << ". Rollback." << std::endl;
		database->exec("ROLLBACK;"); // TODO Error checkings
		return SQLITE_ERROR;
	}
	// Erase on disk
	std::filesystem::remove(Arcollect::path::artwork_pool / std::to_string(art_id));
	std::filesystem::remove(Arcollect::path::artwork_pool / (std::to_string(art_id)+".thumbnail"));
	std::cerr << "Artwork \"" << art_title << "\" (" << art_id << ") has been deleted" << std::endl;
	// Update data_version
	Arcollect::local_data_version_changed();
	return 0;
}

std::size_t Arcollect::db::artwork::image_memory(void)
{
	Uint32 format = SDL_PIXELFORMAT_RGBA32; // Fallback to 32bpp
	if (text)
		text->QueryTexture(&format);
	return SDL_BYTESPERPIXEL(format)*sizeof(Uint8)*art_size.x*art_size.y; // Assume 8-bits RGBA
}

void Arcollect::db::artwork::open_url(void)
{
	SDL_OpenURL(source().c_str());
}
