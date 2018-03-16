cmake_minimum_required(VERSION 3.1.0)

project(retro)

if(APPLE)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/lib/macOS/SDL2.framework")
        message("Downloading SDL2 frameworks")
        execute_process(
            COMMAND "${CMAKE_SOURCE_DIR}/lib/macOS/install.sh"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/macOS"
        )
    endif()
    set(SDL_INCLUDE_DIR "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -F ${CMAKE_SOURCE_DIR}/lib/macOS")
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,-F${CMAKE_SOURCE_DIR}/lib/macOS -Wl,-rpath ${CMAKE_SOURCE_DIR}/lib/macOS -framework Cocoa")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
    set(SDL_LIBRARY
        "-framework SDL2"
        "-framework SDL2_ttf"
        "-framework SDL2_mixer"
    )
    set(SDL_EXPORT_LIBRARIES
        "${CMAKE_SOURCE_DIR}/lib/macOS/SDL2.framework"
        "${CMAKE_SOURCE_DIR}/lib/macOS/SDL2_ttf.framework"
        "${CMAKE_SOURCE_DIR}/lib/macOS/SDL2_mixer.framework"
    )
    mark_as_advanced(SDL_INCLUDE_DIR SDL_LIBRARY)
    set(SO_PLATFORM_FILE "src/base/PlatformMacOS.mm")
elseif(WIN32)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/lib/windows/SDL2")
        message("Downloading SDL2 libraries")
        execute_process(
            COMMAND "${CMAKE_SOURCE_DIR}/lib/windows/install.bat"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/windows"
        )
    endif()
    CONFIGURE_FILE("src/retro++.vcxproj.user.in" "${CMAKE_CURRENT_BINARY_DIR}/retro++.vcxproj.user")
    set(SDL_INCLUDE_DIR
        "${CMAKE_SOURCE_DIR}/lib/windows/SDL2/include"
        "${CMAKE_SOURCE_DIR}/lib/windows/SDL2_ttf/include"
        "${CMAKE_SOURCE_DIR}/lib/windows/SDL2_mixer/include"
    )
    set(SDL_LIBRARY
        "${CMAKE_SOURCE_DIR}/lib/windows/SDL2/lib/x64/SDL2.lib"
        "${CMAKE_SOURCE_DIR}/lib/windows/SDL2/lib/x64/SDL2main.lib"
        "${CMAKE_SOURCE_DIR}/lib/windows/SDL2_ttf/lib/x64/SDL2_ttf.lib"
        "${CMAKE_SOURCE_DIR}/lib/windows/SDL2_mixer/lib/x64/SDL2_mixer.lib"
    )
    set(SDL_EXPORT_LIBRARIES
        "${CMAKE_SOURCE_DIR}/lib/windows/SDL2/lib/x64/"
        "${CMAKE_SOURCE_DIR}/lib/windows/SDL2_ttf/lib/x64/"
        "${CMAKE_SOURCE_DIR}/lib/windows/SDL2_mixer/lib/x64/"
    )
    mark_as_advanced(SDL_INCLUDE_DIR SDL_LIBRARY)
    set(SO_PLATFORM_FILE "src/base/PlatformWindows.cpp")
else()
    include(FindPkgConfig)
    PKG_SEARCH_MODULE(SDL2 sdl2>=2.0.5)
    PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)
    PKG_SEARCH_MODULE(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)
    if(NOT SDL2_FOUND)
        PKG_SEARCH_MODULE(SDL2 sd2)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSDL2_OLD")
        message("-- You have a version of SDL2 prior to 2.0.5, some methods won't work!")
    endif()
    set(SDL_INCLUDE_DIR
        "${SDL2_INCLUDE_DIRS}"
        "${SDL2TTF_INCLUDE_DIRS}"
        "${SDL2MIXER_INCLUDE_DIRS}"
    )
    set(SDL_LIBRARY
        "${SDL2_LIBRARIES}"
        "${SDL2TTF_LIBRARIES}"
        "${SDL2MIXER_LIBRARIES}"
    )
    set(SO_PLATFORM_FILE "src/base/PlatformLinux.cpp")
    set(CMAKE_INSTALL_RPATH "../lib")
    set(ADDRESS_SANITIZE FALSE CACHE BOOL "Enables Address Sanitizer")
    set(UNDEFINED_SANITIZE FALSE CACHE BOOL "Enables Undefined Behaviour Sanitizer")
    if(ADDRESS_SANITIZE)
        message("Enabled Address Sanitizer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
    endif()
    if(UNDEFINED_SANITIZE)
        message("Enabled Undefined Sanitizer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=undefined")
    endif()
endif()

set(GLM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lib/glm/")
set(JSON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lib/json/single_include/nlohmann/")
set(UTFCPP_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lib/utfcpp/source/")
set(STB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lib/stb/")

include_directories(${SDL_INCLUDE_DIR})
include_directories(${GLM_INCLUDE_DIR})
include_directories(${JSON_INCLUDE_DIR})
include_directories(${UTFCPP_INCLUDE_DIR})
include_directories(${STB_INCLUDE_DIR})
include_directories("${CMAKE_SOURCE_DIR}/src/base/headers")
include_directories("${CMAKE_SOURCE_DIR}/src/editor/headers")

if(NOT WIN32)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Wno-unused-parameter")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEFAULT_GAME_PATH='${CMAKE_SOURCE_DIR}/res'")
else()
    #Because I don't export anything using __declspec(dllexport)
    #I have to tell the Visual Studio Compiler to export everything
    #So a .lib can be generated, and therefore an executable can link
    #against the .dll of retroengine++
    # <Damn Windows...>
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DDEFAULT_GAME_PATH=\"${CMAKE_SOURCE_DIR}/res\"")
endif()

set(RETRO_BASE_FILES
    src/base/headers/Animation.hpp
    src/base/headers/AnimationChain.hpp
    src/base/headers/Collisionable.hpp
    src/base/headers/CollisionDetection.hpp
    src/base/headers/Color.hpp
    src/base/headers/ControlledPlayer.hpp
    src/base/headers/Documentation.hpp
    src/base/headers/Frame.hpp
    src/base/headers/Game.hpp
    src/base/headers/GameActions.hpp
    src/base/headers/Level.hpp
    src/base/headers/Logger.hpp
    src/base/headers/Image.hpp
    src/base/headers/Map.hpp
    src/base/headers/MapObject.hpp
    src/base/headers/MovableObject.hpp
    src/base/headers/Object.hpp
    src/base/headers/Optional.hpp
    src/base/headers/Palette.hpp
    src/base/headers/Platform.hpp
    src/base/headers/Player.hpp
    src/base/headers/Sprites.hpp
    src/base/headers/Timeline.hpp
    src/base/headers/Timer.hpp
    src/base/headers/UIObject.hpp

    src/base/Game.cpp
    src/base/GameActions.cpp
    src/base/Logger.cpp
    src/base/Image.cpp
    src/base/Map.cpp
    ${SO_PLATFORM_FILE}
    src/base/Sprites.cpp
    src/base/Timer.cpp
    src/base/UIObject.cpp
)

set(RETRO_EDITORS_FILES
    src/editor/headers/Editor.hpp

    src/editor/SelectMapFileScreen.hpp
    src/editor/CreateMapFileScreen.hpp
    src/editor/MapEditorScreen.hpp
    src/editor/SelectSpritesFileScreen.hpp
    src/editor/CreateSpritesFileScreen.hpp
    src/editor/SpritesEditorScreen.hpp

    src/editor/Editor.cpp
)

add_library(retroeditors++ STATIC ${RETRO_EDITORS_FILES})
add_library(retroengine++ SHARED ${RETRO_BASE_FILES})
target_link_libraries(retroengine++ ${SDL_LIBRARY})

add_executable(retro++
    src/game/DemoGame.inc.hpp
    src/game/lel.inc.hpp
    src/game/Palette.hpp

    src/game/objects/CollectableObject.hpp
    src/game/objects/DaPlayer.hpp
    src/game/uiobjects/Dialog.hpp
    src/game/uiobjects/InventoryHUD.hpp
    src/game/levels/FirstLevel.hpp
    src/game/levels/EndLevel.hpp

    src/game/objects/CollectableObject.cpp
    src/game/objects/DaPlayer.cpp
    src/game/uiobjects/Dialog.cpp
    src/game/uiobjects/InventoryHUD.cpp
    src/game/levels/FirstLevel.cpp
    src/game/levels/EndLevel.cpp

    src/game/main.cpp
)

target_link_libraries(retro++ retroengine++ retroeditors++)


if(WIN32)
    install(TARGETS retro++ DESTINATION .)
    install(TARGETS retroengine++ DESTINATION .)
    install(DIRECTORY ${SDL_EXPORT_LIBRARIES} DESTINATION . FILES_MATCHING PATTERN *.dll)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/res" DESTINATION .)
elseif(APPLE)
    install(TARGETS retro++ DESTINATION retro++.app/Contents/MacOS)
    install(TARGETS retroengine++ DESTINATION retro++.app/Contents/Frameworks)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/res" DESTINATION retro++.app/Contents)
    install(CODE "execute_process(COMMAND rm -r Resources WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/retro++.app/Contents/\" ERROR_QUIET)")
    install(CODE "execute_process(COMMAND mv res Resources WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/retro++.app/Contents/\")")
    install(DIRECTORY ${SDL_EXPORT_LIBRARIES} DESTINATION retro++.app/Contents/Frameworks)
    install(FILES "${CMAKE_SOURCE_DIR}/src/Info.plist" DESTINATION retro++.app/Contents)
    foreach(library ${SDL_EXPORT_LIBRARIES})
        string(REPLACE "${CMAKE_SOURCE_DIR}/lib/macOS/" "" library ${library})
        string(REPLACE ".framework" "" lib_name ${library})
        install(CODE "execute_process(COMMAND install_name_tool -change \"@rpath/${library}/Versions/A/${lib_name}\" \"@rpath/${library}/Versions/A/${lib_name}\" \"${CMAKE_INSTALL_PREFIX}/retro++.app/Contents/Frameworks/libretroengine++.dylib\")")
        install(CODE "execute_process(COMMAND install_name_tool -change \"@rpath/${library}/Versions/A/${lib_name}\" \"@executable_path/../Frameworks/${library}/Versions/A/${lib_name}\" \"${CMAKE_INSTALL_PREFIX}/retro++.app/Contents/MacOS/retro++\")")
    endforeach(library)
    install(CODE "execute_process(COMMAND install_name_tool -change \"@rpath/libretroengine++.dylib\" \"@executable_path/../Frameworks/libretroengine++.dylib\" \"${CMAKE_INSTALL_PREFIX}/retro++.app/Contents/MacOS/retro++\")")
elseif(UNIX)
    install(TARGETS retro++ DESTINATION bin)
    install(TARGETS retroengine++ DESTINATION lib)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/res" DESTINATION share/retro++)
endif()
