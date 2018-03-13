LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL2

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \
    $(LOCAL_PATH)/$(SDL_PATH)_ttf \
    $(LOCAL_PATH)/$(SDL_PATH)_mixer \
    $(LOCAL_PATH)/base/headers \
    $(LOCAL_PATH)/editor/headers \
    $(LOCAL_PATH)/glm \
    $(LOCAL_PATH)/utfcpp/source \
    $(LOCAL_PATH)/json/single_include/nlohmann \
    $(LOCAL_PATH)/stb

# Add your application source files at the end of that list...
LOCAL_SRC_FILES := base/Game.cpp \
    base/GameActions.cpp \
    base/Logger.cpp \
    base/Image.cpp \
    base/Map.cpp \
    base/PlatformAndroid.cpp \
    base/Sprites.cpp \
    base/Timer.cpp \
    base/UIObject.cpp \
    editor/Editor.cpp \
    game/main.cpp

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_ttf SDL2_mixer

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

LOCAL_CPPFLAGS += -std=c++14 -fexceptions -frtti

include $(BUILD_SHARED_LIBRARY)
