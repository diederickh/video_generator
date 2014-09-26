#!/bin/sh

if [ ! -d build.release ] ; then 
    mkdir build.release
fi

d=${PWD}
triplet=""
extern_dir=""
is_mac=n
is_linux=n
is_win=n

if [ ! -d build.release ] ; then 
    mkdir build.release
fi

# Detect system, set triplet. For now the triplet is hardcoded, w/o checks.
if [ "$(uname)" = "Darwin" ]; then
    is_mac=y
    triplet="mac-clang-x86_64"
elif [ "$(expr substr $(uname -s) 1 5)" = "Linux" ]; then
    is_linux=y
    triplet="linux-gcc-x86_64"
elif [ "$(expr substr $(uname -s) 1 10)" = "MINGW32_NT" ]; then
    is_win=y
    triplet="win-vs2012-x86_64"
fi

extern_path=${d}/../extern/${triplet}
install_path=${d}/../install/${triplet}

cd build.release
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${install_path} ../ 
cmake --build . --config Release --target install

if [ -d ${install_path} ] ; then
    cd ${install_path}/bin
    ./example
else
    if [ "${is_win}" = "y" ] ; then
        cd Release
        ./example.exe
    fi
fi
