#!/bin/bash
#
# Copyright (C) Université Paris-Saclay, UVSQ
# Copyright (C) National University of Singapore
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

if [ -t 1 ]
then
    RED="\033[31m"
    GREEN="\033[32m"
    YELLOW="\033[33m"
    BOLD="\033[1m"
    OFF="\033[0m"
else
    RED=
    GREEN=
    YELLOW=
    BOLD=
    OFF=
fi

set -e

VERSION=061f8dd6d48c3a6441d8300e697696bf415683a4

# STEP (1): install e9patch if necessary:
if [ ! -x e9patch-$VERSION/e9patch ]
then
    if [ ! -f e9patch-$VERSION.zip ]
    then
        echo -e "${GREEN}$0${OFF}: downloading e9patch-$VERSION.zip..."
        wget -O e9patch-$VERSION.zip https://github.com/GJDuck/e9patch/archive/$VERSION.zip
    fi

    echo -e "${GREEN}$0${OFF}: extracting e9patch-$VERSION.zip..."
    unzip e9patch-$VERSION.zip

    echo -e "${GREEN}$0${OFF}: building e9patch..."
    cd e9patch-$VERSION
    ./build.sh
    cd ..
    echo -e "${GREEN}$0${OFF}: e9patch has been built..."
else
	echo -e "${GREEN}$0${OFF}: using existing e9patch..."
fi

# STEP (2): build the runtime:
echo -e "${GREEN}$0${OFF}: building the adad runtime..."
e9patch-$VERSION/e9compile.sh src/adad-rt.c -I e9patch-$VERSION/examples/ \
    -I e9patch-$VERSION/src/e9patch/ -DNO_GLIBC=1
chmod a-x adad-rt

# STEP (3): build the installation package:
rm -rf install adad-patch adad-report e9tool
mkdir -p install
cp e9patch-$VERSION/e9patch install
cp e9patch-$VERSION/e9tool install
mv adad-rt install
cp src/adad-patch.sh install/adad-patch
cp src/adad-report.py install/adad-report
chmod a+x install/adad-patch
chmod a+x install/adad-report
ln -s install/e9tool
ln -s install/adad-rt
ln -s install/adad-patch
ln -s install/adad-report


echo -e "${GREEN}$0${OFF}: done!"
