# Arcollect - Your personal visual artwork library

If you like visual arts like me, you may want to save artworks you find on the net. Enjoy an artwork? Just click on *Save in Arcollect*, it is saved along metadatas like the source, artist, tags and the rating. No longer right-click and save picture in a complicated folder structure. This personal project aim to fulfill my needs of artwork collection management and to ease it’s creation, browsing and growth. It's open-source because I think that's a nice piece of software I can give back to the artwork community. **Arcollect is free of tracking, clutter, recommendation and any judgement, it just works and respect your privacy.**

Arcollect design is minimalist and completely hides the interface when you stare at an artwork. It's up to you to bring life in Arcollect with the artworks you will save. On the internet you see well integrated *Save in Arcollect* buttons like if it was a built in feature of the website, a single click and if everything go fine the text turns to *Saved* without more disturbance.

Arcollect is written in C++20 with effective and pragmatic code that use a restricted set of high-quality libraries. Your only performance concern is huge artworks that may take long to show and consume large amount of memory, something Arcollect have to enhance currently.

Arcollect network interactions are carefully designed. Of course, the viewer doesn't access the internet. While browsing, there is zero network activity from Arcollect until you click on the *Save in Arcollect* button. Then the strict minimum of network activity is performed (not even DNS in most cases) with artworks transfer encrypted using strong TLS 1.2 at least and HTTP caching to don't redownload up-to-date artworks and icons you already have.

## Supported platforms
Currently these platforms are supported :

* **[ArtStation](https://www.artstation.com/)** - You have `Save in Arcollect` next to the download button on artworks. Porfolios are not supported yet.
* **[DeviantArt](https://www.deviantart.com/)** - You have a `Save in Arcollect` button below the artwork. Support is limited.
* **[e621](https://e621.net/)**/**[e926](https://e926.net)** - You have a `Save in Arcollect` button below the artwork.
* **[FurAffinity](https://www.furaffinity.net/)** - You have a `Save in Arcollect` button below the artwork.
* **[KnowYourMeme](https://knowyourmeme.com/)** - You have a `Save in Arcollect` button below the meme artwork on image details.
* **[Patreon](https://www.patreon.com/)** - You have a `Save in Arcollect` button below the artwork (you must be on the post page currently).
* **[Pixiv](https://www.pixiv.com/)** - You have a `Save in Arcollect` next to the *Nice!* button (only illustrations are supported so far).
* **[Tumbex](https://www.tumbex.com/)** - Click on the share menu (bottom-right) then `Save in Arcollect`.
* **[Twitter](https://twitter.com/)** - Click on the photo you want to save and `Save in Arcollect` button appear in the bottom. Support is slightly buggy.
* **[xkcd](https://xkcd.com/)** - You have a `Save in Arcollect` button on buttons bars.

## About privacy
Arcollect by itself does not collect any data or statistics, it is not a spyware. Only your artworks and settings are stored inside your personal folder and stay there.

When saving artworks, Arcollect download artworks politely to the server (explicit [`User-Agent`](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/User-Agent), caching, ...) like your normal browser would do, plus with stricter encryption requirements.

### Webextension permissions
The webextension require some permissions to works:

* [Access your data for X website](https://support.mozilla.org/en-US/kb/permission-request-messages-firefox-extensions#w_access-your-data-for-named-site) -- To extract artwork, metadata and insert the *Save in Arcollect* button.
* [Exchange messages with programs other than Firefox](https://support.mozilla.org/en-US/kb/permission-request-messages-firefox-extensions#w_exchange-messages-with-programs-other-than-firefox) (`nativeMessaging`) -- This external program is the `webext-adder` that insert artworks into your collection.
* [Access browser activity during navigation](https://support.mozilla.org/en-US/kb/permission-request-messages-firefox-extensions#w_access-browser-activity-during-navigation) (`webRequestBlocking`) -- To listen for Twitter API activity.
* The `dns` authorization -- To use browser DNS resolver and cache and transmit entries to the `webext-adder`.
* The `cookies` authorization -- To transmit required cookies to the `webext-adder` on platforms where that is needed.

## Contributing
I am open to contributions but keep in mind that this is a personal project, not a community driven thing. Arcollect mostly have what I use and can do with time, but it is built with extensibility and platform agnostism in mind.

Your favorite platform is likely missing, with a content-script in `webextension` you can add support into Arcollect.
macOS support is also welcome.
Better Windows integration too, especially font rendering.
Some KDE integrations also.

Please, keep things simple and cross-patform, you have C++, a modern SDL and a bunch of dependencies, use them. Platform specific integrations are good while lightweight and simple. As an example, Arcollect is a GNOME search provider not using GNOME libs but FreeDesktop ones.

## Installation
Only recent Linux distributions and Microsoft Windows with Mozilla Firefox are supported. Checkout releases that contain ready to use binary and source packages, that is highly recommended. Note that the Windows MSI is not signed and Microsoft SmartScreen filter incorrectly flag it as hazardous, you must insist to download the file, then open MSI file properties, check *Unblock* in the bottom and *Ok*. The MSI should now works.

If you really want to build Arcollect yourself. Arcollect is built using the [Meson](https://mesonbuild.com/) build-system, you will need the version 0.59.0 at least. Note that I actually use [ArchLinux](https://archlinux.org/) and Arcollect need many bleeding edge compilers and dependencies to build and run. During the configuration process, unmet dependencies are automatically downloaded, built and statically linked in the final executables (excluding very common ones and compilers/tools).

Build instructions are available in the [packaging guide](./packaging/README.md).

### Upgrading
When upgrading, **keep in sync the webextension and binary programs versions!** Mismatching these is unsupported, it should works to some extents but lack of the latest features and bug fixes.

Some updates upgrades the database schema, this is a transparent operation but then you may not use an older Arcollect to open your database, this won't works and even though the database schema is robust, it may severely mess up with your personnal collection.

* On Linux systems the next step is to upgrade the Arcollect package.
* On Windows the next steps are to uninstall Arcollect in the Windows configuration panel and install the latest MSI. This operation will not erase your collection.

## Tests
Arcollect have many tests and most notably fully automated "real-life" tests that start a program controlled web-browser. Tests requiring network access are not enabled by default. You can enable them with this set of `meson` options :

* `-Dtests_online=true` To enable off-browser online tests.
* `-Dtests_browser=true` To enable in-browser online tests.
* `-Dtests_nsfw=true` To enable NSFW online tests with mature and adult content.

**🔞️ Warning!** In order to test the rating extraction system on real data, tests can access mature and adult oriented content if told to so with the `-Dtests_nsfw=true` option. Also files with `-nsfw` suffix link to and contain adult content. Using `-Dtests_nsfw=true` will download adult content on your system and for browser automated test will show you this content. **By using `-Dtests_nsfw=true` you agree that you can legally download, view and are not offended by adult oriented content.** Generally you must be 18 years old at least. *Such content is carefully selected to avoid shocking general adult public. But this still is real NSFW.*
