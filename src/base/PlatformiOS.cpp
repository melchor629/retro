#include <Platform.hpp>
#include <string>

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

void retro::sendCommandResponse(const Command &cmd, const nlohmann::json &resp) {}

#include "StdFileImpl.hpp"
