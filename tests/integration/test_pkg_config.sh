#!/bin/sh

set -eu

stage=${1:?staging directory is required}
compiler=${2:?C compiler is required}
pkg_config=${3:?pkg-config executable is required}
version=${4:?library version is required}

export PKG_CONFIG_PATH="$stage/usr/lib/pkgconfig"
export PKG_CONFIG_LIBDIR="$stage/usr/lib/pkgconfig"
export PKG_CONFIG_SYSROOT_DIR="$stage"

actual_version=$("$pkg_config" --modversion readline_stats)
if [ "$actual_version" != "$version" ]; then
    printf 'ÉCHEC : version pkg-config attendue "%s", obtenue "%s"\n' \
        "$version" "$actual_version" >&2
    exit 1
fi

"$compiler" -std=c17 -Wall -Wextra -Wpedantic \
    $("$pkg_config" --cflags readline_stats) \
    examples/count_file.c \
    $("$pkg_config" --libs readline_stats) \
    -o "$stage/count_file"

sh tests/integration/test_library_example.sh "$stage/count_file"
printf 'OK : client compilé via pkg-config\n'
