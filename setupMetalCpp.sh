#!/bin/bash

set -e

buildDir="$1"

if [[ ! -d "$buildDir/_deps/downloads" ]]; then
   mkdir -p "$buildDir/_deps/downloads"
fi 

if [[ ! -f "$buildDir/_deps/downloads/metal-cpp_macOS15.2_iOS18.2.zip" ]]; then
    pushd "$buildDir/_deps/downloads/"
    wget https://developer.apple.com/metal/cpp/files/metal-cpp_macOS15.2_iOS18.2.zip
    popd 
fi

if [[ ! -d "$buildDir/_deps/metal-cpp" ]]; then
    pushd "$buildDir/_deps"
    unzip downloads/metal-cpp_macOS15.2_iOS18.2.zip
    popd
fi
