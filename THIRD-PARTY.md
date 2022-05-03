# Third-party license

This project include some third-party programs with different licensing conditions :

* [**inih**](https://github.com/benhoyt/inih) from *Ben Hoyt* released under the "New BSD" license and retrieved by a Meson Wrap
* A part of [*nst*](https://github.com/nst)'s [**JSONTestSuite**](https://github.com/nst/JSONTestSuite) released under the MIT license and located under [`webext-adder/tests/nst_JSONTestSuite/`](webext-adder/tests/nst_JSONTestSuite). It is used to test the JSON parser and is not included in built binaries.
* **[Roboto](https://fonts.google.com/specimen/Roboto)** fonts from *Google* released under the Apache 2.0 license and retrieved by a Meson Wrap
* [**uBlock Origin**](https://github.com/gorhill/uBlock) from *[ Raymond Hill ](https://github.com/gorhill)* released under the GPL-3.0 license and located in a built form into `webextension/tests/`. It is only used during in-browser tests to fasten them and hide questionable ads.
* More projects might be downloaded and embeded into the final Arcollect binary. Checkout [`subprojects/<project>.wrap`](subprojects) files for the list of softwares.

Most of these projects, if not provided by a system library, are built using customized build-system files that generate a library statically linked inside Arcollect binaries. Checkout the about window for a list of libraries embeded this way in your Arcollect build.
