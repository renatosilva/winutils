#!/bin/bash

# WinUtils Source Bundle Generator
# Copyright (C) 2015, 2016 Renato Silva

# binary[::source]
packages=(gcc-libs::gcc
          gettext
          libiconv
          libmongoose
          libwinpthread-git::winpthreads-git)

package_architecture=$(gcc -dumpmachine)
package_architecture="${package_architecture%%-*}"
architecture="${package_architecture/x86_64/x64}"
architecture="${architecture/i686/x86}"
app_version=$(grep 'define VERSION' winutils.nsi | awk -F'"' '{ printf $2 }')
zip_file="WinUtils-${app_version}-${architecture}-Source.zip"
libraries="libraries/${architecture}"
mkdir -p "${libraries}"

for package in "${packages[@]}"; do
    binary_name="${package%::*}"
    source_name="${package#*::}"
    package_version=$(pacman -Q mingw-w64-${package_architecture}-${binary_name} | sed 's/.* //')
    source_package_full="mingw-w64-${package_architecture}-${source_name}-${package_version}.src.tar.gz"
    source_package_simple="mingw-w64-${source_name}-${package_version}.src.tar.gz"
    test -s "${libraries}/${source_package_full}"   && continue
    test -s "${libraries}/${source_package_simple}" && continue
    rm -fv "${libraries}"/mingw-w64-*"${source_name}"*.src.tar.gz
    wget --no-verbose --directory-prefix "${libraries}" "http://repo.msys2.org/mingw/sources/${source_package_full}"   ||
    wget --no-verbose --directory-prefix "${libraries}" "http://repo.msys2.org/mingw/sources/${source_package_simple}" || exit
done

echo 'Exporting repository'
git archive --format=zip --output="${zip_file}" HEAD

echo 'Adding libraries'
zip -9 -r "${zip_file}" ${libraries}/*.src.tar.gz
