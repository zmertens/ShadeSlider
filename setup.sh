#!/usr/bin/bash

# This script bootstraps the Dear ImGui Android+SDL2 project
#   - It pulls down the latest SDL2 from the web and then copies in the necessary files

# Learning Resources: 
# https://wiki.libsdl.org/Android
# https://hg.libsdl.org/SDL/file/default/docs/README-android.md

SDL2_LATEST='https://www.libsdl.org/release/SDL2-2.0.9.tar.gz'

# Example commands for setting up 'Dear ImGui' Android project
wget $SDL2_LATEST && tar xzf SDL2-2.0.9.tar.gz
# Use -rn to ensure we don't clobber existing files
cp -rn ./SDL2-2.0.9/android-project/* ./android-project
cd ./android_project/app/jni/src
mv ../../../../SDL2-2.0.9 ../SDL