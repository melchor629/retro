#include <Platform.hpp>
#include <string>
#include <jni.h>

using namespace retro;

std::string retro::getCurrentDirectory() {
    return "";
}

std::error_condition getLastError() {
    return std::system_category().default_error_condition(errno);
}

Optional<Command> retro::getCommand() {
    return {};
}

namespace retro {
    float _android_factor_scale = 1.5f;
};
JNICALL extern "C" void Java_me_melchor9000_retro_RetroActivity_tellScale(JNIEnv* env, jclass clazz, jfloat scale) {
    retro::_android_factor_scale = scale;
}

void retro::sendCommandResponse(const Command &cmd, const nlohmann::json &resp) {}

#include "SDLFileImpl.hpp"
