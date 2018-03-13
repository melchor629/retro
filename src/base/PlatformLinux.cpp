#include <Platform.hpp>
#include <unistd.h>
#include <sys/param.h>

using namespace retro;

std::string retro::getCurrentDirectory() {
    char tmp[MAXPATHLEN];
    return getcwd(tmp, MAXPATHLEN);
}

std::error_condition retro::getLastError() {
    return std::system_category().default_error_condition(errno);
}

#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
static int s4 = -1;
static int s6 = -1;
Optional<Command> retro::getCommand() {
    auto showError = [] (const char* str) -> Optional<Command> { perror(str); return {}; };
    if(s4 == -1) {
        if((s4 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) { s4 = 0; return showError("Could not open IPv4 socket"); }
        sockaddr_in serv;
        memset(&serv, 0, sizeof(serv));
        serv.sin_family = AF_INET;
        serv.sin_port = htons(32145);
        serv.sin_addr.s_addr = htonl(0x7f000001); //127.0.0.1
        int opt = 1; //Avoid "Address already in use" error
        setsockopt(s4, SOL_SOCKET, SO_REUSEADDR, (char*) &opt, sizeof(opt));
        if(bind(s4, (sockaddr*) &serv, sizeof(serv)) == -1) return showError("Could not bind tcp://127.0.0.1:32145");
        if(listen(s4, 1) == -1) return showError("Could not listen tcp://127.0.0.1:31245");
        fcntl(s4, F_SETFL, O_NONBLOCK);
    }

    if(s6 == -1) {
        if((s6 = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) == -1) { s6 = 0; return showError("Could not open IPv6 socket"); }
        sockaddr_in6 serv;
        memset(&serv, 0, sizeof(serv));
        serv.sin6_family = AF_INET6;
        serv.sin6_port = htons(32145);
        serv.sin6_addr = IN6ADDR_LOOPBACK_INIT;
        int opt = 1; //Avoid "Address already in use" error
        setsockopt(s6, SOL_SOCKET, SO_REUSEADDR, (char*) &opt, sizeof(opt));
        if(bind(s6, (sockaddr*) &serv, sizeof(serv)) == -1) return showError("Could not bind tcp://[::1]:32145");
        if(listen(s6, 1) == -1) return showError("Could not listen tcp://[::1]:31245");
        fcntl(s6, F_SETFL, O_NONBLOCK);
    }

    char buff[1500];
    for(auto s: std::vector<int>{ s4, s6 }) {
        int sock = accept(s, NULL, NULL);
        if(sock != -1) {
            ssize_t readBytes = recv(sock, buff, 1500, 0);
            if(readBytes == -1) {
                sendCommandResponse({ nlohmann::json::array(), (void*) (long) sock }, {{
                    { "error", "Could not read your request" },
                    { "detailed", getLastError().message() }
                }});
            } else {
                auto string = std::string(buff, readBytes);
                try {
                    auto json = nlohmann::json::parse(string);
                    return retro::Command{ json, (void*) (long) sock };
                } catch(const nlohmann::json::parse_error &e) {
                    sendCommandResponse({ nlohmann::json::array(), (void*) (long) sock }, {{
                        { "error", "Cannot understand your request" },
                        { "detailed", e.what() }
                    }});
                }
            }
        }
    }
    return {};
}

void retro::sendCommandResponse(const Command &cmd, const nlohmann::json &resp) {
    if(cmd._priv_data == nullptr) {
        close(s4);
        close(s6);
    } else {
        auto sock = (int) (long) cmd._priv_data;
        auto json = resp.dump();
        send(sock, json.c_str(), json.size(), 0);
        shutdown(sock, SHUT_WR);
        close(sock);
    }
}

#include "StdFileImpl.hpp"
