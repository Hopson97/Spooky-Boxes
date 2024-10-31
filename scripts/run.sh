#!/bin/bash

if [ "$1" = "release" ]
then
    ./build/release/spooky-boxes
else
    ./build/debug/spooky-boxes
fi