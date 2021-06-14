# Arcollect - Your personal visual artwork library

(**This personnal project is firstly for my needs.** But I think it's something good to share. I planned to make it public with more feature but I published it sooner that initially intended. Currently I removed the issues tab and will reject *Pull Requests* for now. **I will accept contributions only once I feel ready.**)

If you like visual arts like me, you may want to save some artworks you find on the net. Right-click and save picture works but is not very convenient, forget the artwork source and classification is complicated.

This personal project aim to fulfill my needs of artwork collection management and to ease it's creation, browsing and growth that I do in a few clicks thanks to the web-extension which put a save button on artwork pages. Metadata are also saved like the source, account, tags and the rating.

Note that it's goal is to provide an easy way to browse your saved artworks. It will not help you to discover new artworks or make any recommendations, it's outside his scope. In fact, there is no mention of any platforms excluding if you use it or checkout the list of supported platforms and (trust me) it's better this way.

Under the hood, the application is powered with C++17 by the SDL2 and SQLite3 for metadata storage.

This application respect your privacy and will never judge you.

## Supported platforms
Currently these platforms are supported :

* **[e621](https://e621.net/)**/**[e926](https://e926.net)** - You have a `Save in Arcollect` button below the artwork.
* **[FurAffinity](https://www.furaffinity.net/)** - You have a `Save in Arcollect` button below the artwork.

## Contributing
**This is a preview and I currently don't accept any contribution.** When I'll accept contribution, you might read something like this draft that imply that I made and published more work that I intend to do :

*Currently Arcollect is limited to what I use and my abilities. But it is built with extensibility and platform agnostism in mind.*

*Your favorite platform may be missing. Read `NEW-PLATFORM.md` (this is important !) to learn how to integrate your platform then feel free to code the extension and share your work.*

*Currently only furry oriented platforms are supported (and a very few amount). Arcollect is not furry oriented and you are really welcome to add more platforms !*

*Chromiums, Windows and macOS support are also welcome. Please, keep things simple and cross-patform, you have C++ and the SDL, use them. However, I am not against platform specific integration.*

*This project also lack internationalization, translations, color management and real font rendering.*

## Installation
Only recent Linux distributions with Firefox are supported right-now.

I recommend you to build the source code from the [latest release](https://github.com/DevilishSpirits/arcollect/releases) (be sure to pick the *Latest release*, not a *Pre-release*).

After getting the source code, install Meson, SDL2. You should also install SQLite and SDL2 image and ttf modules.

To build and install Arcollect on a Linux machine, open a shell in source root and then :

```sh
	# Configure the project with release configuration
	meson build -Dbuildtype=release -Dunity=on -Db_lto=true -Dstrip=true -Denable_webextension=false
	# Install the program in /usr/local prefix (require root privileges)
	ninja install -C build
```
Note that I actually use [ArchLinux](https://archlinux.org/) and this program might need bleeding edge versions of dependencies to run. Most problematic ones are automatically "wrap" by the Meson build-system, a private up-to-date version is downloaded, built and embeded into executables. After installing Meson 0.57.1 (at least) from PyPI, things should works out-of-the-box and does on GitHub Actions Ubuntu 20.04 VM. Actually Ubuntu 18.04 won't works and you should upgrade anyway.

 Here is a matrix of versions requirements to help troubleshooting. The *Wrap* column show you the version that is automatically embeded if you have a *Known buggy* version of that library.

| Dependency | Known buggy |  Wrap  | ArchLinux | Ubuntu 20.10 | Fedora 34 |
|------------|------------:|-------:|:---------:|:------------:|:---------:|
| SQLite     |     <3.35.0 | 3.35.5 | ![](https://repology.org/badge/version-for-repo/arch/sqlite.svg?header=&minversion=3.35.0) | ![](https://repology.org/badge/version-for-repo/ubuntu_20_10/sqlite.svg?header=&minversion=3.35.0) | ![](https://repology.org/badge/version-for-repo/fedora_34/sqlite.svg?header=&minversion=3.35.0) |
| SDL2       |      Unknow | 2.0.12 | ![](https://repology.org/badge/version-for-repo/arch/sdl2.svg?header=) | ![](https://repology.org/badge/version-for-repo/ubuntu_20_10/sdl2.svg?header=) | ![](https://repology.org/badge/version-for-repo/fedora_34/sdl2.svg?header=) |
| SDL2_image |      Unknow |  2.0.5 | ![](https://repology.org/badge/version-for-repo/arch/sdl2-image.svg?header=) | ![](https://repology.org/badge/version-for-repo/ubuntu_20_10/sdl2-image.svg?header=) | ![](https://repology.org/badge/version-for-repo/fedora_34/sdl2-image.svg?header=) |
| SDL2_ttf   |     <2.0.12 | 2.0.12 | ![](https://repology.org/badge/version-for-repo/arch/sdl2-ttf.svg?header=&minversion=2.0.12) | ![](https://repology.org/badge/version-for-repo/ubuntu_20_10/sdl2-ttf.svg?header=&minversion=2.0.12) | ![](https://repology.org/badge/version-for-repo/fedora_34/sdl2-ttf.svg?header=&minversion=2.0.12) |
| curl       |      Unknow | 7.77.0 | ![](https://repology.org/badge/version-for-repo/arch/curl.svg?header=) | ![](https://repology.org/badge/version-for-repo/ubuntu_20_10/curl.svg?header=) | ![](https://repology.org/badge/version-for-repo/fedora_34/curl.svg?header=) |
| Meson      |     <0.57.1 |   N/A  | ![](https://repology.org/badge/version-for-repo/arch/meson.svg?header=&minversion=0.57.1) | ![](https://repology.org/badge/version-for-repo/ubuntu_20_10/meson.svg?header=&minversion=0.57.1) | ![](https://repology.org/badge/version-for-repo/fedora_34/meson.svg?header=&minversion=0.57.1) |
