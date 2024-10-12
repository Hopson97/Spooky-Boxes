#!/bin/bash

echo "Running OGL Template"
if [ "$1" = "release" ]
then
    ./build/release/bin/ogl_template  
else
    ./build/debug/bin/ogl_template  
fi