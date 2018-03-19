# retro++

`retro++` is a simple retro-pixelart 2D game engine that aims to help to introduce anybody with knowledge of programming (specially with C++) into the world of game development without having to deal with the low-level APIs OpenGL or DirectX, or with newer APIs like Vulkan or Metal (both unsupported in this engine).

## Clone the repo

`git clone --recursive https://github.com/melchor629/retro`

The repo has submodules on it, so it will initialize them when cloning.

## CMake

To build the project, and do it consistently among Windows, Linux and macOS computers, retro++ uses [CMake][6] to generate the necessary files depending on the platform.

The `CMakeFile.txt` on Windows and macOS will call a script that will download the SDL2 libraries and headers for you. If one of the scripts fail, try executing them manually, specially if you are on Windows because the script is a PowerShell script and executing them requires Administrator permissions in some cases. In both platforms, using CMake GUI is enough and it is not needed to make any changes to the configuration to make it work.

On Linux, while debugging, you should set `CMAKE_BUILD_TYPE` to `Debug` using either the CMake GUI or in terminal with `-DCMAKE_BUILD_TYPE=Debug`.

## How to prepare the project

### Windows

Mingw nor Cygwin are supported, so you must use [Visual Studio 2015 or 2017][7] with C/C++ support to compile the project. You can install the compilers only. It's up to you.

The project is created through [CMake][6]. Download the [app][8] (search for "Latest Release...") and install it (if you downloaded the installer) or decompress it anywhere (if you downloaded the zip).

If you have Windows 7, ensure that PowerShell is installed to. The CMake script executes a PowerShell script that download the libraries needed for you :)

To prepare the Visual Studio solution, open CMake. In the first text field, labelled with _Where is the source code?_ select the path where you cloned or downloaded this repo. Then copy that path and paste it in the second text field, labelled with _Where to build the source code_ and append `/build` at the end. An example:

  - Where is the source code? `C:/Users/melchor629/Documents/GitHub/retro++`
  - Where to build the source code? `C:/Users/melchor629/Documents/GitHub/retro++/build`

Once done, press the **Configure** button. A window will open telling you to "specify the generator for this project", and more. For that dropdown, check that says _Visual Studio ?? 201? Win64_ where you must select 2015 or 2017 version depending on what you have installed. Important that the _generator_ ends with Win64. 32bit Windows is not supported. When the _generator_ is selected, press **Finish**. When the configuration step is done, you can press **Generate** and will generate the solution. Now you can press **Open Project**. Happy coding :)

### Debian and Ubuntu based Linux distros

First, you must install the compilers, some tools and the dependencies. This is done with the following command:

```bash
sudo apt update
sudo apt install build-essential g++ cmake libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev timidity
```

Then, execute `g++ --version`. If the version is below 7, then you must install GNU G++ 7 (6 is untested, might work without using 7). To do it in Ubuntu and Linux Mint, execute:

```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-7
```

On Debian 8 or 9, see https://stackoverflow.com/questions/43151627/installing-g-7-0-1-on-debian-8-7.

After installing dependencies and the compilers, `cd` to the cloned directory. Then create a `build` directory (`mkdir build`) and `cd` to it. If you didn't have to install g++-7 separately, you can execute:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j2 #You can put another number, if you have 8 cores, put 8 fe
./retro++ #This executes the game, if everything went good
```

If you had to install g++-7 separately, you will execute:

```bash
cmake -DCMAKE_CXX_COMPILER=$(which g++-7) -DMAKE_C_COMPILER=$(which gcc-7) -DCMAKE_BUILD_TYPE=Debug ..
make -j2 #You can put another number, if you have 8 cores, put 8 fe
./retro++ #This executes the game, if everything went good
```

And that's all. Now you have the project files to compile the game. Happy coding :)

To make a release build, to be able to export or install anywhere, execute the `cmake` command changing _Debug_ with _Release_. Also could be good to specify where you want to "export" the game. That can be done by adding `-DCMAKE_INSTALL_PREFIX=YOUR_PATH` before the dots. Then, with `make install` (may need `sudo`) you will have an exportable version of your game.

### Arch Linux based Linux distros

First, you must install the compilers, some tools and the dependencies. This is done with the following command:

```bash
sudo pacman -S cmake sdl2 sdl2_ttf sdl2_mixer #-Syu is also valid instead of -S
```

Then, `cd` to the cloned directory. Then create a `build` directory (`mkdir build`) and `cd` to it. Execute this:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j2 #You can put another number, if you have 8 cores, put 8 fe
./retro++ #This executes the game, if everything went good
```

And that's all. Now you have the project files to compile the game. Happy coding :)

To make a release build, to be able to export or install anywhere, execute the `cmake` command changing _Debug_ with _Release_. Also could be good to specify where you want to "export" the game. That can be done by adding `-DCMAKE_INSTALL_PREFIX=YOUR_PATH` before the dots. Then, with `make install` (may need `sudo`) you will have an exportable version of your game.

### macOS

First you must install [Xcode][9]. Then download [CMake app][8] and put it where you want. When both apps are ready, open Xcode, if it is the first time you do, it will install the compilers and some tools.

Then, open CMake. In the first text field, labelled with _Where is the source code?_ select the path where you cloned or downloaded this repo. Then copy that path and paste it in the second text field, labelled with _Where to build the source code_ and append `/build` at the end. An example:

  - Where is the source code? `/Users/melchor629/Documents/GitHub/retro++`
  - Where to build the source code? `/Users/melchor629/Documents/GitHub/retro++/build`

Once done, press the **Configure** button. A window will open telling you to "specify the generator for this project", and more. For that dropdown, check that says _Xcode_, it is usually selected. You can also use `Makefile` if you prefer the Linux approach. When the _generator_ is selected, press **Finish**. When the configuration step is done, you can press **Generate** and will generate the Xcode project, or the Makefiles. Now you can press **Open Project** in case of select Xcode, and compile from terminal if you selected Makefiles. In Xcode, search for the Play - Stop buttons at the top-left corner, and change the target from `ALL_BUILD` to `retro++`. Happy coding :)

  > **Note**: If you select `Makefile`, search the property `CMAKE_BUILD_TYPE` and write `Debug`.

For export an `.app` of your game, first search in CMake the property `CMAKE_INSTALL_PREFIX` and put where you want to export your game. If you selected `Makefile` as the generator, also modify `CMAKE_BUILD_TYPE` to `Release`. Then press generate. In Xcode, change the target (its near the Play - Stop buttons) from `retro++` to `INSTALL`, and press the Play button. For `Makefile`, execute `make install`. _Game exported_.

### Android

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

### iOS

To make an iOS version of your game, first download the sources for [SDL2][2], [SDL_ttf][3] and [SDL_mixer][4]. Put them under the root folder of the iOS project. Then, create symbolic links of `src/base`, `src/editor`, `src/game`, `lib/glm`, `lib/json`, `lib/stb` and `lib/utfcpp`.

```bash
#It's only an example, for illustrate you
ln -s "/Users/melchor9000/Desktop/retro++/src/base" "/Users/melchor9000/Desktop/retro++/src/editor" "/Users/melchor9000/Desktop/retro++/src/game" .
ln -s "/Users/melchor9000/Desktop/retro++/lib/glm" "/Users/melchor9000/Desktop/retro++/lib/json" "/Users/melchor9000/Desktop/retro++/lib/stb" "/Users/melchor9000/Desktop/retro++/lib/utfcpp" .
```

Then, go to `SDL2-2.0.7/Xcode-iOS/Template/SDL iOS Application` and copy that folder to `SDL2-2.0.7/SDL iOS Application`. Remove the Xcode project and paste the one inside `ios`. Should work correctly.

The Xcode project defines the macro `__IOS__` to differentiate between macOS and iOS.

## inspect.py

This Python 3 script allows you to use the Inspection API quickly. To send commands, write them argument as arguments. If you want to modify an attribute, add at the end `=THE_VALUE`. If the value is an object, use JSON format.

`python3 inspect.py game::currentLevel::uiObjects::0::text="Some text" game::currentLevel::uiObjects::0::font='{"size": 10}'`

## Where's the resources?

You can find the resources in [this link][10]. Download it, and extract it in `res`.

## Libraries
 
`retro++` uses the following great libraries:

  - [SDL2](https://www.libsdl.org/)
  - [SDL2_ttf](https://www.libsdl.org/projects/SDL_ttf/)
  - [SDL2_mixer](https://www.libsdl.org/projects/SDL_mixer/)
  - [nlohmann/json](https://github.com/nlohmann/json)
  - [nothings/stb (stb_image)](https://github.com/nothings/stb)
  - [nemtrif/utfcpp](https://github.com/nemtrif/utfcpp)
  - [glm](https://github.com/g-truc/glm)

  [1]: http://lazyfoo.net/tutorials/SDL/52_hello_mobile/index.php
  [2]: https://www.libsdl.org/release/SDL2-2.0.8.tar.gz
  [3]: https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.14.tar.gz
  [4]: https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.2.tar.gz
  [5]: https://hg.libsdl.org/SDL_mixer/archive/tip.zip
  [6]: https://cmake.org
  [7]: https://www.visualstudio.com/
  [8]: https://cmake.org/download/
  [9]: https://itunes.apple.com/es/app/xcode/id497799835?mt=12
  [10]: https://www.dropbox.com/s/w2jafy4b5rtpwox/resources-retro%2B%2B.7z?dl=1
