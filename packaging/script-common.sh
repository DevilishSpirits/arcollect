arcollect_version() {
	grep -E -o "[^_]version: '[0-9\.]+'" ../meson.build | grep -Eo "[0-9\.]+"
}

meson_version_req() {
	grep -E -o "meson_version: '>=[0-9\.]+'" ../meson.build | grep -Eo ">=[0-9\.]+"
}

# Source config.txt
# TODO Don't use eval!
eval "$(sed -E 's/#.*//g' ../config.txt | grep -F ':'| sort -u | sed -E "s/[ ]*:[ ]*/='/" | sed -E "s/[ ]*$/'/")"
