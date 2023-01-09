/* Arcollect -- An artwork collection manager
 * Copyright (C) 2021-2022 DevilishSpirits (aka D-Spirits or Luc B.)
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
/** \file desktop-app/art-reader/image-thumb-xdg.cpp
 *  \brief XDG and fallback thumbnail implementation
 *
 * This file implement the default thumbnailing system.
 * \note The `-DWITH_XDG' switch alter it's behavior. When turned off, there is
 *       a partial support that may use the XDG thumbnail cache but generated
 *       thumbnails are not compliant with the standard.
 * \see The reference https://specifications.freedesktop.org/thumbnail-spec/thumbnail-spec-0.9.0.html spec implemented.
 */
#include <OpenImageIO/imageio.h> // Enable some stuff
#include "image.hpp"
#include <cstdlib>
#include <arcollect-debug.hpp>
#include <config.h>
#include <arcollect-paths.hpp>
#include <md5.hpp>
#include <zlib.h>
#if WITH_XDG
#include <sys/stat.h>
#endif


static std::string xdg_make_uri(const std::filesystem::path &path) {
	std::string workpath("file://");
	workpath += path.string();
	if (path.preferred_separator == '\\') {
		// Switch to URI '/' separator
		for (char& chr: workpath)
			if (chr == path.preferred_separator)
				chr = '/';
	}
	return workpath;
}
static std::string xdg_hash_uri(const std::filesystem::path &path) {
	return std::to_string(MD5_CTX::hash(path.native()));
}

static const std::filesystem::path thumbnails_dir = "thumbnails";

static std::filesystem::path thumbnails_root;
static std::filesystem::path lookup_thumbnails_root(void)
{
	// Lookup for XDG_CACHE_HOME
	const char* cache_root = std::getenv("XDG_CACHE_HOME");
	if (cache_root)
		return std::filesystem::path(cache_root)/thumbnails_dir;
	// Query XDG compliant fallback
	#if WITH_XDG
	cache_root = std::getenv("HOME");
	if (cache_root) {
		static const std::filesystem::path cache_subdir = ".cache";
		return std::filesystem::path(cache_root)/cache_subdir/thumbnails_dir;
	}
	#endif
	// Fallback in the collection
	return Arcollect::path::arco_data_home/thumbnails_dir;
}

static const std::pair<int,std::filesystem::path> dirs_sizes[] = {
	// Keep sorted in ascending size!
	{128,"normal"},
	{256,"large"},
	{512,"x-large"},
	{1024,"xx-large"},
};
static constexpr auto dirs_sizes_n = sizeof(dirs_sizes)/sizeof(dirs_sizes[0]);
static const auto min_thumbnail_size = dirs_sizes[0].first;
static const auto max_thumbnail_size = dirs_sizes[dirs_sizes_n-1].first;

OIIO::ImageInput::unique_ptr Arcollect::art_reader::load_thumbnail(const std::filesystem::path &path, SDL::Point size)
{
	
	const std::string uri = xdg_make_uri(path);
	const std::filesystem::path thumbnail_filename(xdg_hash_uri(uri)+".png");
	auto target_thumbnail_size = std::max(size.x,size.y);
	if (Arcollect::debug.thumbnails)
		std::cerr << "Loading " << target_thumbnail_size << "px thumbnail for " << path.string() << " (" << thumbnail_filename << ") ";
	#if WITH_XDG
	// Stat the file
	struct stat source_stat;
	if (stat(path.c_str(),&source_stat)) {
		if (Arcollect::debug.thumbnails)
			std::cerr << "stat(" << path.string() << ") failed, abort" << std::endl;
		return OIIO::ImageInput::unique_ptr();
	}
	#endif
	// Compute thumbnails_root if empty
	if (thumbnails_root.empty()) {
		thumbnails_root = lookup_thumbnails_root();
	}
	// Attempt to load thumbnails
	for (const auto& dir: dirs_sizes) {
		// Check the thumbnail size
		if (dir.first <= target_thumbnail_size)
			continue;
		if (Arcollect::debug.thumbnails)
			std::cerr << dir.first << "/" << dir.second.string() << "... ";
		// Try to load the thumbnail
		const std::filesystem::path thumbnail_path = thumbnails_root/dir.second/thumbnail_filename;
		auto image = OIIO::ImageInput::open(thumbnail_path.native());
		if (!image) {
			std::filesystem::remove(thumbnail_path); // Erase because it's defective.
			if (Arcollect::debug.thumbnails)
				std::cerr << "OIIO failed to open! ";
			continue;
		}
		const OIIO::ImageSpec &spec = image->spec();
		#if WITH_XDG
		// Perform XDG specific checks
		if (atol(spec.get_string_attribute("Thumb::MTime","1").c_str()) != source_stat.st_mtim.tv_sec) {
			std::filesystem::remove(thumbnail_path);
			if (Arcollect::debug.thumbnails)
				std::cerr << "Thumb::MTime mismatch! ";
			continue;
		}
		#endif
		if (spec.get_string_attribute("Thumb::URI") != uri) {
			if (Arcollect::debug.thumbnails)
				std::cerr << "Thumb::URI mismatch! ";
			continue;
		}
		// The thumbnail seem good, return it
		if (Arcollect::debug.thumbnails)
			std::cerr << "loaded successfully." << std::endl;
		return image;
	}
		if (Arcollect::debug.thumbnails)
			std::cerr << "no thumbnail loaded." << std::endl;
	return OIIO::ImageInput::unique_ptr();
}
void Arcollect::art_reader::write_thumbnail(const std::filesystem::path &path, SDL::Surface& surface, OIIO::ImageSpec spec)
{
	// Check if making a thumbnail makes sense
	const auto largest_surf_edge = std::max(surface.w,surface.h);
	if (largest_surf_edge <= min_thumbnail_size)
		return;
	// Prepare things
	const std::string uri = xdg_make_uri(path);
	const std::filesystem::path thumbnail_filename(xdg_hash_uri(uri)+".png");
	const SDL_PixelFormat* const surf_format = surface.format;
	if (Arcollect::debug.thumbnails)
		std::cerr << "Writing thumbnails for " << path.string() << " (" << thumbnail_filename << ") ";
	#if WITH_XDG
	struct stat source_stat;
	if (stat(path.c_str(),&source_stat)) {
		// TODO Check errno
		if (Arcollect::debug.thumbnails)
			std::cerr << "stat(" << path.string() << ") failed, abort" << std::endl;
		return; // Don't generate thumbnails
	}
	spec.attribute("Thumb::MTime",std::to_string(source_stat.st_mtim.tv_sec));
	#endif
	spec.attribute("Thumb::URI",uri);
	static std::string software_string = []() {
		int oiio_version = OIIO::openimageio_version();
		return "Arcollect/" ARCOLLECT_VERSION_STR " OpenImageIO/" +
			std::to_string(oiio_version/10000)+"."+
			std::to_string((oiio_version%10000)/100)+"."+
			std::to_string(oiio_version%100);
	}();
	spec.attribute("Software",software_string);
	spec.attribute("png:compressionLevel",Z_BEST_COMPRESSION); 
	spec.format = OIIO::TypeDesc::UINT8;
	// Generate the output thumbnail surface
	SDL::Rect thumb_out{0,0,surface.w,surface.h};
	if (thumb_out.w > thumb_out.h) {
		thumb_out.h *= max_thumbnail_size;
		thumb_out.h /= thumb_out.w;
		thumb_out.w  = max_thumbnail_size;
	} else  {
		thumb_out.w *= max_thumbnail_size;
		thumb_out.w /= thumb_out.h;
		thumb_out.h  = max_thumbnail_size;
	}
	std::unique_ptr<SDL::Surface> thumbnail_surf(reinterpret_cast<SDL::Surface*>(SDL_CreateRGBSurfaceWithFormat(0,thumb_out.w,thumb_out.h,surf_format->BitsPerPixel,surf_format->format)));
	// Make thumbnails
	for (const auto& dir: dirs_sizes) {
		// Check the thumbnail size
		if (dir.first >= largest_surf_edge)
			break;
		// Generate metadata
		const auto div_ratio = max_thumbnail_size/dir.first;
		spec.width  = thumb_out.w = thumbnail_surf->w/div_ratio;
		spec.height = thumb_out.h = thumbnail_surf->h/div_ratio;
		if (Arcollect::debug.thumbnails)
			std::cerr << dir.second <<" (" << thumb_out.w << "×" << thumb_out.h << ")... ";
		// Open the thumbnail file
		// TODO Support O_TMPFILE
		std::filesystem::path thumbnail_path = thumbnails_root/dir.second;
		std::filesystem::create_directories(thumbnail_path);
		thumbnail_path /= thumbnail_filename;
		std::filesystem::path tmp_thumbnail_path(thumbnail_path);
		tmp_thumbnail_path += ".arcollect-tmp.png";
		auto out_thumbnail = OIIO::ImageOutput::create("png");
		if (!out_thumbnail->open(tmp_thumbnail_path.native(),spec)) {
			if (Arcollect::debug.thumbnails)
				std::cerr << "failed to create() " << tmp_thumbnail_path.string() << "!";
			continue;
		}
		// Scale down and write the image
		SDL_BlitScaled(&surface,NULL,thumbnail_surf.get(),(SDL_Rect*)&thumb_out);
		if (out_thumbnail->write_image(OIIO::TypeDesc::UINT8,thumbnail_surf->pixels,surf_format->BytesPerPixel,thumbnail_surf->pitch))
			std::filesystem::rename(tmp_thumbnail_path,thumbnail_path);
		else {
			if (Arcollect::debug.thumbnails)
				std::cerr << "failed to write_image() " << tmp_thumbnail_path.string() << "!";
		}
		std::filesystem::remove(tmp_thumbnail_path);
	}
			if (Arcollect::debug.thumbnails)
				std::cerr << std::endl;
}
