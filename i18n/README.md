# Arcollect i18n/l10n

This is the home of Arcollect multilingual capabilities. It uses a homemade framework that allow to centralize all translations in this single place, including metadatas files such as the AppStream file and the webextension translations.
In case you ask yourself, the inner workings are a bit cursed but well oiled and allow a perfect DRY principle along automated translations that just works.

I have been inspired bythe idea of [Fluent](https://projectfluent.org/) but it is unavailable for C++ and adding Rust, a full programming language, in my build system just for i18n isn't worth.

## How translation works in Arcollect ?
Translations are grouped in independants *module* that can be separately imported:

* The `common` module contain static strings used everywhere.
* The `desktop_app` module contain translations in the desktop-app.
* The `webextension` module contain translations in the webextension.

A module is imported using `#include <arcollect-i18n-<module>.hpp>`and define a `struct Arcollect::i18n::<module>` structure that hold translations, in Meson add `i18n_deps['<module>']` to your dependencies. In the C++, initialize the i18n structure and call `apply_locale()` to setup a translation, multiple calls are possible to accumulate various translations such are a Britanny citizen in France who may prefer Breton over French, so you would call `apply_locale("fr")` then `apply_locale("br")`. Then you mostly have `std::string_view` but some translations can be more complex and implemented using function pointers, reading the source**s** there is the only reliable source. The "s" has been emphased because translations can come from C++ source code, PO files and more.

Every modules are declared, configured and exploined in the top of the [`meson.build`](meson.build) file, read over this file now as it is part of the documentation.

Each language is located in the `l10n-lang_COUNTRY` subdirectory, with default untranslated text (so in english) in `l10n-C`.

## Using translations (i18n)
To use a translation, add the `i18n_deps['<module>']` dependency in your target depdencies, in the C++ code `#include <arcollect-i18n-<module>.hpp>` and initialize a `struct Arcollect::i18n::<module>` and call `apply_locale()` one, more or even zero times, calls to this functions are cumulatives, for a Britanny user you may apply the *fr_FR* locale then the *br* one so the software is in Breton then French locales, the structure is default initialized with the C locale. Then access fields of this structure which are mostly `std::string_view` but can be technically everything like functions generating a `Arcollect::gui::font::Elements`.

The structure define multiple introspections fields:

- A static `translations` array that list available translations.
- A static `po_strings` for strings in the PO file.
- A `get_po_string()` that take a `po_strings` and returns it's translation.

These are use by metadatas files generators to avoid spreading translations everywhere. AppStream, .desktop files and the webextension generator make use of these fields to automatically generate translated files. That's slighly complex to use but powerful since the japenese adding a translation doesn't have to edit anything outside the `i18n` directory and figure out where to translate the metadatas files, it gets replicated everywhere (app, webextension and metadata files) with a simple rebuild, rarely a reconfiguration as with the *Don't Repeat Yourself* principle.

# Making translations (l10n)
When translating a word the hardest part is to locate the translation, from the `i18n` folder run 'grep -R "word/part of the text to translate"' to figure out where it is. If there is a PO in the language you want to translate, make the translation. If not, you need to copy the part from the `i10n-C` folder and translate it, keep things in order. It is possible that the module is not translated in your language, locate the module where the translation and checkout the [`meson.build`](meson.build) file for this construct:

```meson
i18n_configuration = {
	# ...
	'<module>': {
		'locales': [
			'de',
			'fr',
		],
```

Then add your languages, let say you want to add a `jp` translation, add it in the list of locales (keep the list sorted):
```meson
i18n_configuration = {
	# ...
	'<module>': {
		'locales': [
			'de',
			'fr',
			'jp',
		],
```

The `ab_CD` language code format is supported, `jp_JP` is valid but Arcollect then need a generic `jp` translation.

Next you must copy all `l10n-C/<module>.*` files in the `l10n-jp/` folder, if there is a `l10n-jp/<module>.cpp` file, locate this line:

```cpp
void Arcollect::i18n::<module>::apply_C(void) noexcept {
```

And replace *C* with *jp*, the `C` language is the untranslated language (hence why we copied files from `l10n-C`) and you need to replace each instance of this language identifier :
```cpp
void Arcollect::i18n::<module>::apply_jp(void) noexcept {
```

Translate strings generated by this C++ code. If there wasn't a `l10n-jp/<module>.cpp` file do nothing! If you don't know C++, remove everything so the file become:

```cpp
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
#include "arcollect-i18n-<module>.hpp"
void Arcollect::i18n::<module>::apply_jp(void) noexcept {
}
```

If there was no file like this, do not create it.

No we can move on to the `l10n-jp/<module>.po` file, this is a standard PO file, as a translator you may have already used this kind of file, note that Arcollect does not support anything more than a simple text-to-text translation (no plural forms, ...).

## Testing translations
Build and run Arcollect.

The Arcollect i18n system is 100% automatic so a simple rebuild is sufficient to apply your translations everywhere.
