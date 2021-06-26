# Arcollect packaging files

This directory contain files used to generate packages. You probably won't need these unless you want to package a custom version.

**Important!** You must have a working internet connection to build packages from source tarballs. The configuration process download additionnal dependencies.

## How to generate a package for my system ?
Configure the project into the `build` directory but do not build it, package generation will do it's own configuration/build process anyway. Then `cd` into `build/packaging` that contain configured versions of files under `packaging` and follow system specific instructions :

```sh
	# cd to_source_root
	meson build        # Configure in 'build' directory
	cd build/packaging
```

You may want to take a look into the *release* GitHub Actions workflow at [`.github/workflows/release.yml`](https://github.com/DevilishSpirits/arcollect/blob/master/.github/workflows/release.yml) to see exactly how releases are generated.

### [ArchLinux](https://archlinux.org/) and derived
Run [`makepkg`](https://man.archlinux.org/man/makepkg.8) **with the `PKGBUILD.local` file !** Else you will download a release from GitHub, it's either useless or unwanted if you want to build a version with local modifications. Note that the PKGBUILD force a release configuration with LTO and optimizations turned on.

```
	makepkg -p PKGBUILD.local
```
