#!/bin/bash

function stripFrameworks() {
    echo " > $1"
    for file in "$1"/*.framework; do
        framework=${file/$1/}
        framework=${framework:1}
        framework=${framework/.framework/}
        dir="$file/Versions/Current"
        echo "  -> $file"

        ditto --rsrc --arch x86_64 $dir/$framework $dir/${framework}2
        rm $dir/$framework
        mv $dir/${framework}2 $dir/$framework

        if [[ -d "$dir/Frameworks" ]]; then
            stripFrameworks "$dir/Frameworks"
        fi
    done
}

stripFrameworks "$1"
