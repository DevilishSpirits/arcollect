# Arcollect - Your personal visual artwork library

If you like visual arts like me, you may want to save some artworks you find on the net. Right-click and save picture works but is not very convenient, forget the artwork source and classification is complicated.

This personal project aim to fulfill my needs of artwork collection management and to ease it's creation, browsing and growth that I do in a one click thanks to the web-extension which put a *Save in Arcollect* button on artwork pages. Metadata like the source, account, tags and the rating are also saved. I made Arcollect for my needs, but it's something nice to share like artworks you will save.

It's goal is limited to easily browse and save artworks, it won't help you to discover new artworks or make any recommendations. No one can judge what's good or not better than you.

Under the hood, the application is powered with C++17 by the [SDL2](https://www.libsdl.org/) ([this fork currenctly](https://github.com/Cacodemon345/SDL/tree/iccprofile)), [SQLite3](https://www.sqlite.org/) for metadata storage, [OpenImageIO](https://openimageio.readthedocs.org/) for image loading and [LittleCMS](https://littlecms.com/) for color management and many other components listed under `THIRD-PARTY.md`.

This application respect your privacy and will never judge you.

## Supported platforms
Currently these platforms are supported :

* **[ArtStation](https://www.artstation.com/)** - You have `Save in Arcollect` next to the download button on artworks. Porfolios are not supported yet.
* **[DeviantArt](https://www.deviantart.com/)** - You have a `Save in Arcollect` button below the artwork. Support is limited.
* **[e621](https://e621.net/)**/**[e926](https://e926.net)** - You have a `Save in Arcollect` button below the artwork.
* **[FurAffinity](https://www.furaffinity.net/)** - You have a `Save in Arcollect` button below the artwork.

## Contributing
I am open to contributions but keep in mind that this is a personal project, not a community driven thing. Arcollect mostly have what I use and can do with time, but it is built with extensibility and platform agnostism in mind.

Your favorite platform is likely missing, with a content-script in `webextension` you can add support into Arcollect. Chromiums and macOS support are also welcome. This project also lack internationalization, translations and real font rendering.

Please, keep things simple and cross-patform, you have C++, a modern SDL and a bunch of dependencies, use them. Platform specific integrations are good while lightweight and simple. As an example, Arcollect is a GNOME search provider not using GNOME libs but FreeDesktop ones.

## Installation
Recent Linux distributions and Microsoft Windows with Firefox are supported.

I recommend you to grab a prebuilt package from the [latest release](https://github.com/DevilishSpirits/arcollect/releases/tag/v0.13) and to checkout his [README.md](https://github.com/DevilishSpirits/arcollect/tree/v0.13#readme). You must install/upgrade the webextension in your web-browser to use Arcollect.

If you want to build the software yourself, grab the source code, install Meson (>0.59.0), dbus on UNIX-like, SQLite, SDL2_ttf, OpenImageIO, lcms2 and libcurl. CMake is also required to configure some dependencies. Then checkout the [packaging guide](https://github.com/DevilishSpirits/arcollect/tree/master/packaging#readme) for your system or open a shell in source root and then :

```sh
	# Ensure that you have a working version of Meson
	pip3 install meson>=0.59.0
	# Configure the project with release configuration
	# More configuration may be needed like -Dc_link_args=-static and -Dcpp_link_args=-static
	meson build -Dbuildtype=release -Db_lto=true -Dstrip=true -Denable_webextension=false
	# Don't do that but you can install the program in /usr/local prefix
	# This won't works on Windows target, generate the MSI instead.
	#ninja install -C build
```

Note that I actually use [ArchLinux](https://archlinux.org/) and this program might need bleeding edge dependencies to build and run. Most are automatically "wrap" by the Meson build-system, a private up-to-date version is downloaded, built and embeded into executables. This does have some limits (like Boost that is not wrapped yet). After installing Meson 0.59.0 (at least) from PyPI, things should works out-of-the-box and does on GitHub Actions Ubuntu 20.04 and Windows 2019 (with Boost libs) VM. Ubuntu 18.04 might works with Clang (`CC=clang CXX=clang++ meson build ...`) and an updated version of Boost if you insist. I won't complexify the program to support older systems.

## Tests
Arcollect have many tests and most notably fully automated "real-life" tests that start a program controlled web-browser. Tests requiring network access are not enabled by default. You can enable them with this set of `meson` options :

* `-Dtests_online=true` To enable off-browser online tests.
* `-Dtests_browser=true` To enable in-browser online tests.
* `-Dtests_nsfw=true` To enable NSFW online tests with mature and adult content.

**🔞️ Warning!** In order to test the rating extraction system on real data, tests can access mature and adult oriented content if told to so with the `-Dtests_nsfw=true` option. Also files with `-nsfw` suffix link to and contain adult content. Using `-Dtests_nsfw=true` will download adult content on your system and for browser automated test will show you this content. **By using `-Dtests_nsfw=true` you agree that you can legally download, view and are not offended by adult oriented content.** Generally you must be 18 years old at least. *Such content is carefully selected to avoid shocking general adult public. But this still is real NSFW.*
