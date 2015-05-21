#!/bin/bash

# WinUtils Source Bundle Generator
# Copyright (C) 2015 Renato Silva

# binary[::source]
packages=(gcc-libs::gcc
          gettext
          libiconv
          libmongoose-git
          libwinpthread-git::winpthreads-git)

package_architecture=$(gcc -dumpmachine)
package_architecture="${package_architecture%%-*}"
mingw_package_prefix="mingw-w64-${package_architecture}"
app_version=$(grep 'define VERSION' winutils.nsi | awk -F'"' '{ printf $2 }')
zip_file="WinUtils ${app_version} Source.zip"
mkdir -p libraries

for package in "${packages[@]}"; do
    binary_package="${mingw_package_prefix}-${package%::*}"
    package_version=$(pacman -Q ${binary_package} | sed 's/.* //')
    source_prefix="mingw-w64-i686-${package#*::}"
    source_package="${source_prefix}-${package_version}.src.tar.gz"
    source_url="https://sourceforge.net/projects/msys2/files/REPOS/MINGW/Sources/${source_package}"

    [[ -s "libraries/${source_package}" ]] && continue || rm -fv libraries/${source_prefix}*src.tar.gz
    wget --no-verbose --directory-prefix 'libraries' "${source_url}" || exit
done

echo 'Exporting repository'
git archive --format=zip --output="${zip_file}" HEAD

echo 'Adding libraries'
zip -9 -r "${zip_file}" libraries/*.src.tar.gz
