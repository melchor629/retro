#include <Platform.hpp>
#include <direct.h>

using namespace retro;

std::string retro::getCurrentDirectory() {
    char tmp[500];
    return _getcwd(tmp, 500);
}

std::error_condition retro::getLastError() {
	return std::system_category().default_error_condition(errno);
}


#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <ws2tcpip.h>
static WSADATA wsaData;
static bool initializedWinSock = false;
static SOCKET s4 = -1;
static SOCKET s6 = -1;
Optional<Command> retro::getCommand() {
	auto showError = [](const char* str) -> Optional<Command> { perror(str); return {}; };
	if(!initializedWinSock) {
		int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (r != 0) {
			fprintf(stderr, "Could not initialize a basic component for Windows: (error code) %d\n", r);
			exit(1);
		}
		initializedWinSock = true;
	}

	if(s4 == -1) {
		if((s4 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) { s4 = 0; return showError("Could not open IPv4 socket"); }
		sockaddr_in serv;
		memset(&serv, 0, sizeof(serv));
		serv.sin_family = AF_INET;
		serv.sin_port = htons(32145);
		serv.sin_addr.s_addr = htonl(0x7f000001); //127.0.0.1
		int opt = 1; //Avoid "Address already in use" error
		setsockopt(s4, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
		if(::bind(s4, (sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR) return showError("Could not bind tcp://127.0.0.1:32145");
		if(listen(s4, 1) == SOCKET_ERROR) return showError("Could not listen tcp://127.0.0.1:31245");
		u_long opt2 = 1;
		ioctlsocket(s4, FIONBIO, &opt2);
	}

	if(s6 == -1) {
		if((s6 = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) == -1) { s6 = 0; return showError("Could not open IPv6 socket"); }
		sockaddr_in6 serv;
		memset(&serv, 0, sizeof(serv));
		serv.sin6_family = AF_INET6;
		serv.sin6_port = htons(32145);
		serv.sin6_addr = IN6ADDR_LOOPBACK_INIT;
		int opt = 1; //Avoid "Address already in use" error
		setsockopt(s6, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
		if(::bind(s6, (sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR) return showError("Could not bind tcp://[::1]:32145");
		if(listen(s6, 1) == SOCKET_ERROR) return showError("Could not listen tcp://[::1]:31245");
		u_long opt2 = 1;
		ioctlsocket(s6, FIONBIO, &opt2);
	}

	char buff[1500];
	for(auto s : std::vector<SOCKET>{ s4, s6 }) {
		SOCKET sock = accept(s, NULL, NULL);
		if(sock != SOCKET_ERROR) {
			int readBytes = recv(sock, buff, 1500, 0);
			if(readBytes == SOCKET_ERROR) {
				sendCommandResponse({ nlohmann::json::array(), (void*) sock }, {{
					{ "error", "Could not read your request" },
					{ "detailed", getLastError().message() }
				}});
			} else {
				auto string = std::string(buff, readBytes);
				try {
					auto json = nlohmann::json::parse(string);
					return retro::Command{ json, (void*) sock };
				} catch(const nlohmann::json::parse_error &e) {
					sendCommandResponse({ nlohmann::json::array(), (void*) sock }, {{
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
		closesocket(s4);
		closesocket(s6);
		WSACleanup();
		initializedWinSock = false;
	} else {
		auto sock = (SOCKET) cmd._priv_data;
		auto json = resp.dump();
		send(sock, json.c_str(), json.size(), 0);
		shutdown(sock, SD_SEND);
		closesocket(sock);
	}
}

#include "StdFileImpl.hpp"
