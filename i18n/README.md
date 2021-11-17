# Arcollect i18n/l10n

This is the home of Arcollect multilingual capabilities. It uses a homemade C++
framework based on dynamic polymorphism, I use structures that contains pointers
to locale specific data.

Every translations and so on are located within this diretory, this include
metadata files like the .desktop file and the webextension translations.
*In case you ask yourself, the inner workings are well oiled but a bit cursed.*

Why? Because I love the idea of [Fluent](https://projectfluent.org/) but it is
unavailable for C++, it is for Rust but the cost of adding a full programming
language in my build just for i18n isn't worth.

Wanna translate Arcollect? Static strings use the PO file format.
For anything else, you need to know C++ for that, keep reading this
documentation though it may still helps.

## Modules
A module is a set of translatable elements, there is many:

* The `common` module contain static strings used everywhere.

## Translation tutorial
Arcollect use structures in the `Arcollect::i18n` that contains all locale
specific data, there is one per module (desktop-app, ...). There is the
base implementation (the *C* locale) in english, localization is achieved by a
set of functions which alter this structure with locale specific changes.
Adding a translation is basically creating this function and telling the
build-system that this translation exists.
Static strings are stored and automatically extracted from in tree PO files.

We'll take an example with the desktop-app itself with french from France, we
assume that this translation doesn't exist.

We create a file named `l10n-fr_FR/desktop_app.cpp` (naming is very important!)
with the standard boiler-plate code for that :

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
#include "arcollect-i18n-desktop_app.hpp"
constexpr void Arcollect::i18n::desktop_app::apply_fr_FR(void) noexcept {
	#include "arcollect-i18n-desktop_app-apply_fr_FR.cpp"
	// TODO Localization
}
```

Within this function you will edit the `Arcollect::i18n::desktop_app` object
translations and even more, you may define annex functions in the file.

Let's translate the *Edit artwork…* label, To find out where it is located, open
[`l10n-C/desktop_app.cpp`](./l10n-C/desktop_app.cpp), the *C* locale is the
default one used by Arcollect, it contain the same function with `fr_FR`
replaced by`C` and implement an english locale. Press Ctrl+F in your text editor
and search for `Edit artwork…` (note that we use an UTF-8 ellipsis `…` and not
three separated ASCII dots `...`), copy paste the line in you french translation
file and change the string with `"Modifier l'oeuvre…"`. You function look-like
this now :

```cpp
constexpr void Arcollect::i18n::desktop_app::apply_fr_FR(void) noexcept {
	Arcollect::i18n::desktop_app::apply_fr(); // Apply shared french translation
	topmenu_slideshow_edit_artwork = "Modifier l'oeuvre…";
}
```

When adding more translations, keep the function sorted for better readability.
Then copy `l10n-C/desktop_app.po` to `l10n-fr_FR/desktop_app.po`.
In fact, the *topmenu_slideshow_edit_artwork* is stored there and should be
translated in the PO file. Remove the C++ line and add the translation in the PO
file, there is many GUI for that, you can also just edit the text file :

```po
msgctxt "topmenu_slideshow_edit_artwork"
msgid "Edit artwork…"
msgstr "Modifier l'oeuvre…"
```

Required files are okay, we need to tell the build-system that our translation
exists, open [`meson.build`](./meson.build) in the `i18n` folder, it contains
the `l10n_cpp_translations` dictionary of array that list modules and availables 
translations, add your language code in the module array. Again sort items :

```meson
l10n_cpp_translations = {
	'desktop_app': [
		'fr_FR',
	]
}
```

Now rebuild Arcollect, there will be a whole reconfiguration process once.
Run the desktop-app in the build tree and you can see the translation working if
you are in France french locale. You can rename all occurences of `fr_FR` to
`fr` to make a translation for all french variants, you don't need to duplicate
the content of `fr` to `fr_FR` since the later call the `fr` variant in the
beginning. In fact, a Breizh speaker would likely have in sequence the `fr`,
`fr_FR` and `br` locales loaded, it depend of his system and
[SDL_GetPreferredLocales](https://wiki.libsdl.org/SDL_GetPreferredLocales).
You don't have to deal with language detection, this is left to programs.

## New module tutorial
Create `arcollect-i18n-<module>.hpp` with this content :

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
#include <arcollect-i18n.hpp>
namespace Arcollect {
	namespace i18n {
		struct <module> {
			// TODO Locale dependant data
			#include "arcollect-i18n-<module>-struct-autogenerated.hpp"
		};
	}
}
```

The `#include` will add declarations for `apply_*()` functions and the default
constructor, this code is automatically generated by Meson.
Now create members that contains locale dependant data and the translation for
the `C` locale with the guide in the previous section. And tadaa!

Now make use of your module.

## Internalizing/localizating non C++ files
Localization is simple as for C++ files, just localize the module and then it's
magic.

Internationalization is the part that make things magic.
You have to write custom C++ generator for these files.
Add the `i18n_infos['<module>']` dependencies to them, they expose
`arcollect-l10_infos-<module>.hpp` headers that contains useful informations
like the list of languages in `Arcollect::i18n_infos::<module>.langs()`.
Use the standard API to generate translations.
