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
#include <fontconfig/fontconfig.h>

namespace Fc {
	using Type = FcType;
	using MatchKind = FcMatchKind;
	struct Matrix: public FcMatrix {
		constexpr void Init(void) {
			FcMatrixInit(this);
		}
		bool operator==(const Matrix& right) const {
			return FcMatrixEqual(this,&right);
		}
		Matrix operator*(const Matrix& right) const {
			Matrix result;
			FcMatrixMultiply(&result,this,&right);
			return result;
		}
		void Rotate(FcMatrix *m, double c, double s) {
			return FcMatrixRotate(this,c,s);
		}
		void Scale(FcMatrix *m, double sx, double sy) {
			return FcMatrixScale(this,sx,sy);
		}
		void Shear(FcMatrix *m, double sh, double sv) {
			return FcMatrixShear(this,sh,sv);
		}
	};
	struct CharSet {
		FcCharSet* handle;
		operator FcCharSet*(void) {
			return handle;
		}
		operator const FcCharSet*(void) const {
			return handle;
		}
		CharSet(void) : handle(FcCharSetCreate()) {}
		CharSet(FcCharSet* handle) : handle(handle) {}
		CharSet(const CharSet& other) : handle(FcCharSetCopy(other.handle)) {}
		~CharSet(void) {
			FcCharSetDestroy(handle);
		}
		bool operator==(const FcCharSet *other) const {
			return FcCharSetEqual(handle,other);
		}
		
		CharSet operator&(const FcCharSet *other) const {
			return FcCharSetIntersect(handle,other);
		}
		CharSet operator|(const FcCharSet *other) const {
			return FcCharSetUnion(handle,other);
		}
		CharSet operator-(const FcCharSet *other) const {
			return FcCharSetSubtract(handle,other);
		}
		CharSet& operator-=(const FcCharSet *other) {
			FcCharSet* new_handle = FcCharSetSubtract(handle,other);
			FcCharSetDestroy(handle);
			handle = new_handle;
			return *this;
		}
		FcBool AddChar(FcChar32 ucs4) {
			return FcCharSetAddChar(handle,ucs4);
		}
		FcBool DelChar(FcChar32 ucs4) {
			return FcCharSetDelChar(handle,ucs4);
		}
		FcBool Merge(const FcCharSet *b) {
			return FcCharSetMerge(handle,b,nullptr);
		}
		FcBool Merge(const FcCharSet *b, FcBool &changed) {
			return FcCharSetMerge(handle,b,&changed);
		}
		FcBool HasChar(FcChar32 ucs4) const {
			return FcCharSetHasChar(handle,ucs4);
		}
		FcChar32 Count(void) const {
			return FcCharSetCount(handle);
		}
		FcChar32 IntersectCount(const FcCharSet *other) const {
			return FcCharSetIntersectCount(handle,other);
		}
		FcChar32 SubtractCount(const FcCharSet *other) const {
			return FcCharSetSubtractCount(handle,other);
		}
		FcBool IsSubset(const FcCharSet *other) const {
			return FcCharSetIsSubset(handle,other);
		}
		FcChar32 FirstPage(FcChar32 map[FC_CHARSET_MAP_SIZE]) {
			return FcCharSetFirstPage(handle, map,nullptr);
		}
		FcChar32 FirstPage(FcChar32 map[FC_CHARSET_MAP_SIZE], FcChar32 &next) {
			return FcCharSetFirstPage(handle, map,&next);
		}
		FcChar32 NextPage(FcChar32 map[FC_CHARSET_MAP_SIZE]) {
			return FcCharSetNextPage(handle, map,nullptr);
		}
		FcChar32 NextPage(FcChar32 map[FC_CHARSET_MAP_SIZE], FcChar32 &next) {
			return FcCharSetNextPage(handle, map,&next);
		}
	};
	struct Pattern {
		FcPattern *handle;
		operator FcPattern *(void) {
			return handle;
		}
		operator const FcPattern *(void) const {
			return handle;
		}
		
		Pattern(void) : handle(FcPatternCreate()) {}
		Pattern(const Fc::Pattern& other) : handle(other.handle) {
			FcPatternReference(handle);
		}
		Pattern(FcPattern&&) = delete;
		Pattern(FcPattern *pattern) : handle(pattern) {}
		~Pattern(void) {
			FcPatternDestroy(handle);
		}
		Pattern &operator=(Pattern& other) {
			FcPatternReference(other.handle);
			FcPatternDestroy(handle);
			handle = other.handle;
			return *this;
		}
		Pattern &operator=(const Pattern& other) = delete;
		Pattern &operator=(Pattern&& other) = delete;
		
		int ObjectCount(void) const {
			return FcPatternObjectCount(handle);
		}
		bool Add(const char *object, int i) {
			return FcPatternAddInteger(handle,object,i);
		}
		bool Add(const char *object, double d) {
			return FcPatternAddDouble(handle,object,d);
		}
		bool Add(const char *object, const FcChar8 *s) {
			return FcPatternAddString(handle,object,s);
		}

		//FcBool FcPatternAddMatrix (FcPattern *p, const char *object, const FcMatrix *s);

		bool Add(const char *object, const FcCharSet *c) {
			return FcPatternAddCharSet(handle,object,c);
		}

		bool Add(const char *object, bool b) {
			return FcPatternAddBool(handle,object,b);
		}
	};
	struct Config {
		FcConfig *handle;
		operator FcConfig *(void) {
			return handle;
		}
		operator const FcConfig *(void) const {
			return handle;
		}
		
		static FcChar8* Home(void) {
			return FcConfigHome();
		}
		static bool EnableHome(bool enable) {
			return FcConfigEnableHome(enable);
		}
		
		Config &Reference(void) {
			FcConfigReference(handle);
			return *this;
		}
		static Config Current(void) {
			return Config(FcConfigReference(FcConfigGetCurrent()));
		}
		static bool SetCurrent(FcConfig *config) {
			return FcConfigSetCurrent(config);
		}
		static bool SetCurrent(Config &config) {
			return FcConfigSetCurrent(config.handle);
		}
		bool MakeCurrent(void) {
			return SetCurrent(*this);
		}
		
		Config(void) : handle(FcConfigCreate()) {}
		Config(FcConfig* config) : handle(config) {}
		~Config(void) {
			FcConfigDestroy(handle);
		}
		bool Substitute(FcPattern *pattern, MatchKind kind) {
			return FcConfigSubstitute(handle,pattern,kind);
		}
		bool Substitute(Pattern &pattern, MatchKind kind) {
			return FcConfigSubstitute(handle,pattern.handle,kind);
		}
	};
}
