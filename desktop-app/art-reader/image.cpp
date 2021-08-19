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
#include "art-reader.hpp"
#include "../config.hpp"
#define CMSREGISTER // Remove warnings about 'register' keyword
#include <iostream>
#include "lcms2.h"
#include <OpenImageIO/imageio.h>
extern bool debug_icc_profile;
// FIXME This as global is bad
extern cmsHPROFILE    cms_screenprofile;
cmsHPROFILE cms_screenprofile = NULL; // Filled in Arcollect::gui::init()

SDL::Surface* Arcollect::art_reader::image(const std::filesystem::path &path)
{
	auto image = OIIO::ImageInput::open(path.string());
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
	SDL::Surface* surface = reinterpret_cast<SDL::Surface*>(SDL_CreateRGBSurfaceWithFormat(0,spec.width,spec.height,spec.nchannels*8,pixel_format));
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
