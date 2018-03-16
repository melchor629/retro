#!/bin/bash

#SDL libraries version
SDL_VERSION="2.0.8"
SDL_TTF_VERSION="2.0.14"
SDL_MIXER_VERSION="2.0.2"

function print() {
    echo -e " > \033[31m$1\033[0m"
}

function dosdl() {
    # $1 URL
    # $2 Name
    print "Downloading $1"
    curl -s -o $2.dmg "$1"
    print "Mounting $2.dmg"
    hdiutil attach $2.dmg > /dev/null || exit $?
    print "Coping $2.framework into macOS"
    cp -HR /Volumes/$2/$2.framework . || exit $?
    print "Unmounting and removing $2.dmg"
    hdiutil detach /Volumes/$2/ > /dev/null || exit $?
    rm $2.dmg || exit $?
}

rm -r *.framework 2> /dev/null

dosdl \
    "https://www.libsdl.org/release/SDL2-$SDL_VERSION.dmg" \
    SDL2
dosdl \
    "https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-$SDL_TTF_VERSION.dmg" \
    SDL2_ttf
dosdl \
    "https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-$SDL_MIXER_VERSION.dmg" \
    SDL2_mixer

print "Dependencies installed"
