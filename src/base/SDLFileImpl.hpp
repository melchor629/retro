//
//  SDLFileImpl.hpp
//  retro
//
//  Created by Melchor Garau Madrigal on 6/2/18.
//

#pragma once

//I suppose "Platform.hpp" is included already
#if !defined(_WIN32) and !defined(__ANDROID__) && !defined(__IOS__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

using namespace std;
using namespace retro;

////////////////////////////////////////////////////////////////////////////////////////////////////

InputFile::InputFile() {}

InputFile::InputFile(const string &file, bool binary) {
    open(file, binary);
}

bool InputFile::open(const string &file, bool binary) {
    SDL_RWops* ops;
    if(binary) {
        ops = SDL_RWFromFile(file.c_str(), "rb");
    } else {
        ops = SDL_RWFromFile(file.c_str(), "r");
    }
    if(ops != nullptr) {
        _impl = ops;
        fail = BasicFile::Nothing;
        return true;
    } else {
        _impl = nullptr;
        fail = BasicFile::CannotOpen;
        return false;
    }
}

bool InputFile::close() {
    SDL_RWops* ops = get_impl_ptr<SDL_RWops>();
    return SDL_RWclose(ops) == 0;
}

size_t InputFile::read(void* buff, size_t n, size_t sizeOfCType) {
    SDL_RWops* ops = get_impl_ptr<SDL_RWops>();
    size_t readSize = SDL_RWread(ops, buff, sizeOfCType, n);
    if(readSize != 0) {
        return readSize;
    } else {
        fail = BasicFile::CannotRead;
        return 0;
    }
}

off_t InputFile::seeki(off_t offset, BasicFile::SeekDirection dir) {
    SDL_RWops* ops = get_impl_ptr<SDL_RWops>();
    int whence;
    if(dir == BasicFile::Beginning) whence = RW_SEEK_SET;
    if(dir == BasicFile::Current) whence = RW_SEEK_CUR;
    if(dir == BasicFile::End) whence = RW_SEEK_END;
    off_t off = SDL_RWseek(ops, offset, whence);
    if(off >= 0) {
        return off;
    } else {
        fail = BasicFile::CannotSeek;
        return -1;
    }
}

off_t InputFile::telli() {
    SDL_RWops* ops = get_impl_ptr<SDL_RWops>();
    off_t off = SDL_RWtell(ops);
    if(off == off_t(-1)) {
        fail = BasicFile::CannotSeek;
    }
    return off;
}

InputFile::~InputFile() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

OutputFile::OutputFile() {}

OutputFile::OutputFile(const string &file, bool binary, bool append) {
    SDL_RWops* ops;
    char mode[] = { append ? 'a' : 'w', binary ? 'b' : '\0', '\0' };
    ops = SDL_RWFromFile(file.c_str(), mode);
    if(ops != nullptr) {
        _impl = ops;
        fail = BasicFile::Nothing;
    } else {
        _impl = nullptr;
        fail = BasicFile::CannotOpen;
    }
}

bool OutputFile::open(const string &file, bool binary) {
    SDL_RWops* ops;
    char mode[] = { 'w', binary ? 'b' : '\0', '\0' };
    ops = SDL_RWFromFile(file.c_str(), mode);
    if(ops != nullptr) {
        _impl = ops;
        fail = BasicFile::Nothing;
        return true;
    } else {
        _impl = nullptr;
        fail = BasicFile::CannotOpen;
        return false;
    }
}

bool OutputFile::close() {
    SDL_RWops* ops = get_impl_ptr<SDL_RWops>();
    return SDL_RWclose(ops) == 0;
}

size_t OutputFile::write(const void* buff, size_t n, size_t sizeOfCType) {
    SDL_RWops* ops = get_impl_ptr<SDL_RWops>();
    size_t writeSize = SDL_RWwrite(ops, buff, sizeOfCType, n);
    if(writeSize == n) {
        return writeSize;
    } else {
        fail = BasicFile::CannotWrite;
        return writeSize;
    }
}

off_t OutputFile::seeko(off_t offset, BasicFile::SeekDirection dir) {
    SDL_RWops* ops = get_impl_ptr<SDL_RWops>();
    int whence;
    if(dir == BasicFile::Beginning) whence = RW_SEEK_SET;
    if(dir == BasicFile::Current) whence = RW_SEEK_CUR;
    if(dir == BasicFile::End) whence = RW_SEEK_END;
    off_t off = SDL_RWseek(ops, offset, whence);
    if(off >= 0) {
        return off;
    } else {
        fail = BasicFile::CannotSeek;
        return -1;
    }
}

off_t OutputFile::tello() {
    SDL_RWops* ops = get_impl_ptr<SDL_RWops>();
    off_t off = SDL_RWtell(ops);
    if(off == off_t(-1)) {
        fail = BasicFile::CannotSeek;
    }
    return off;
}

OutputFile::~OutputFile() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

InputOutputFile::InputOutputFile(const string &file, bool binary, bool append) {
    SDL_RWops* ops;
    char mode[] = { append ? 'a' : 'w', binary ? 'b' : '+', binary ? '+' : '\0', '\0' };
    ops = SDL_RWFromFile(file.c_str(), mode);
    if(ops != nullptr) {
        _impl = ops;
        fail = BasicFile::Nothing;
    } else {
        _impl = nullptr;
        fail = BasicFile::CannotOpen;
    }
}

bool InputOutputFile::open(const string &file, bool binary) {
    SDL_RWops* ops;
    char mode[] = { 'w', binary ? 'b' : '+', binary ? '+' : '\0', '\0' };
    ops = SDL_RWFromFile(file.c_str(), mode);
    if(ops != nullptr) {
        _impl = ops;
        fail = BasicFile::Nothing;
        return true;
    } else {
        _impl = nullptr;
        fail = BasicFile::CannotOpen;
        return false;
    }
}

bool InputOutputFile::close() {
    SDL_RWops* ops = get_impl_ptr<SDL_RWops>();
    return SDL_RWclose(ops) == 0;
}

InputOutputFile::~InputOutputFile() {}

