#!/bin/bash

target_release() {
    cd release
    cmake -DCMAKE_BUILD_TYPE=Release ../..
    make
    echo "Built target in build/release/"
    cd ../..
}

target_debug() {
    cd debug 
    cmake -DCMAKE_BUILD_TYPE=Debug ../..
    make
    echo "Built target in build/debug/"
    cd ../..
}

# Create folder for distribution
if [ "$1" = "release" ]
then
    if [ -d "$spooky-boxes" ]
    then
        rm -rf -d spooky-boxes
    fi

    mkdir -p spooky-boxes
fi

# Creates the folder for the buildaries
mkdir -p spooky-boxes 
mkdir -p spooky-boxes/assets
mkdir -p build
mkdir -p build/release
mkdir -p build/debug
cd build

# Builds target
if [ "$1" = "release" ]
then
    target_release
    cp build/release/spooky-boxes spooky-boxes/spooky-boxes
else
    target_debug
fi

cp -R assets spooky-boxes/
