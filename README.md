# retro++

`retro++` is a simple retro-pixelart 2D game engine that aims to help to introduce anybody with knowledge of programming (specially with C++) into the world of game development without having to deal with the low-level APIs OpenGL or DirectX, or with newer APIs like Vulkan or Metal (both unsupported in this engine).
 
To develop using this engine, you will need the following:
 - **Windows**:
 - [Visual Studio 2015 or 2017](https://www.visualstudio.com/)
    - [CMake](http://cmake.org)
 - **Debian/Ubuntu**
    - `build-essential` (in Ubuntu is usually installed by default)
    - `cmake`
    - `libsdl2-dev` / `libsdl2-2.0.0`
    - `libsdl2-ttf-dev` / `libsdl2-ttf-2.0.0`
    - `libsdl2-mixer-dev` / `libsdl2-mixer-2.0.0` (recommended to check if `timidity` is installed)
    - An IDE/editor of your election. You can use [Atom](https://atom.io), [Visual Studio Code](https://www.visualstudio.com/), Geany, [Sublime Text](http://sublimetext.com), [CLion](https://www.jetbrains.com/clion/)...
    - A terminal if the IDE/editor doesn't compile the project for you
 - **Arch Linux based distro**
    - `sdl2`
    - `sdl2_ttf`
    - `sdl2_mixer` (recommended to install `timidity++` and `timidity-freepats`)
    - `cmake`
    - An IDE/editor of your election. You can use [Atom](https://atom.io), [Visual Studio Code](https://www.visualstudio.com/), Geany, [Sublime Text](http://sublimetext.com), [CLion](https://www.jetbrains.com/clion/)...
    - A terminal if the IDE/editor doesn't compile the project for you
 - **macOS**
    - [CMake](http://cmake.org)
    - You can use Xcode (easy to use), or any of the programs listed in Debian/Ubuntu/Arch Linux.

## Clone the repo

`git clone --recursive https://github.com/melchor629/retro++`

The repo has submodules on it, so it will initialize them as well when cloning.

## CMake

To build the project, and do it consistently among Windows, Linux and macOS computers, retro++ uses [CMake](https://cmake.org) to generate the necessary files depending on the platform.

The `CMakeFile.txt` on Windows and macOS will call a script that will download the SDL2 libraries and headers for you. If one of the scripts fail, try executing them manually, specially if you are on Windows because the script is a PowerShell script and executing them requires Administrator permissions in some cases.

## inspect.py

This Python 3 script allows you to use the Inspection API quickly. To send commands, write them argument as arguments. If you want to modify an attribute, add at the end `=THE_VALUE`. If the value is an object, use JSON format.

`python3 inspect.py game::currentLevel::uiObjects::0::text="Some text" game::currentLevel::uiObjects::0::font='{"size": 10}'`

## Android

To make an Android version of your game, first follow the following tutorial for your platform [da tutorial][1]. After this, download the sources for [SDL2][2], [SDL_ttf][3] and [SDL_mixer][4]. Put them under the folder `jni`. Then, create symbolic links of `src/base`, `src/editor`, `src/game`, `lib/glm`, `lib/json`, `lib/stb` and `lib/utfcpp`.

```bash
#It's only an example, for illustrate you
ln -s "/Users/melchor9000/Desktop/retro++/src/base" "/Users/melchor9000/Desktop/retro++/src/editor" "/Users/melchor9000/Desktop/retro++/src/game" .
ln -s "/Users/melchor9000/Desktop/retro++/lib/glm" "/Users/melchor9000/Desktop/retro++/lib/json" "/Users/melchor9000/Desktop/retro++/lib/stb" "/Users/melchor9000/Desktop/retro++/lib/utfcpp" .
```

And then, modify `app/src/main/jni/src/Android.mk` to look like the one that you can find in [android/app/src/main/jni/src/Android.mk](https://github.com/melchor629/retro/blob/master/android/app/src/main/jni/src/Android.mk).

Modify `app/src/main/jni/Application.mk` to look like this one [android/app/src/main/jni/Application.mk](https://github.com/melchor629/retro/blob/master/android/app/src/main/jni/Application.mk).

Also modify `app/src/AndroidManifest.xml`, following the commentaries found there.

Copy the contents of `android/app/src/main/java/` (from the [repo](https://github.com/melchor629/retro/blob/master/android/app/src/main/java/)) into your project. Contains only an Activity. That activity calls some C++ code required for the engine. While editing the manifest, create a new Activity that extends `me.melchor9000.retro.RetroActivity`, in your desired package.
Make that new activity your default activity.

If you get an error compiling SDL_mixer, try to download [SDL_mixer][5] from there.

## iOS

To make an iOS version of your game, first download the sources for [SDL2][2], [SDL_ttf][3] and [SDL_mixer][4]. Put them under the root folder of the iOS project. Then, create symbolic links of `src/base`, `src/editor`, `src/game`, `lib/glm`, `lib/json`, `lib/stb` and `lib/utfcpp`.

```bash
#It's only an example, for illustrate you
ln -s "/Users/melchor9000/Desktop/retro++/src/base" "/Users/melchor9000/Desktop/retro++/src/editor" "/Users/melchor9000/Desktop/retro++/src/game" .
ln -s "/Users/melchor9000/Desktop/retro++/lib/glm" "/Users/melchor9000/Desktop/retro++/lib/json" "/Users/melchor9000/Desktop/retro++/lib/stb" "/Users/melchor9000/Desktop/retro++/lib/utfcpp" .
```

Then, go to `SDL2-2.0.7/Xcode-iOS/Template/SDL iOS Application` and copy that folder to `SDL2-2.0.7/SDL iOS Application`. Remove the Xcode project and paste the one inside `ios`. Should work correctly.

The Xcode project defines the macro `__IOS__` to differentiate between macOS and iOS.

## Where's the resources?

For the DemoGame, you will need to download a 8-bit-like font ([like this one](https://github.com/Neko250/sublime-PICO-8/blob/master/font/PICO-8.ttf)) and obtain `onestop.mid` from `C:\Windows\Media\onestop.mid` or from internet.

This is just for now.

  [1]: http://lazyfoo.net/tutorials/SDL/52_hello_mobile/index.php
  [2]: https://www.libsdl.org/release/SDL2-2.0.7.tar.gz
  [3]: https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.14.tar.gz
  [4]: https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.2.tar.gz
  [5]: https://hg.libsdl.org/SDL_mixer/archive/tip.zip
