//
//  StdFileImpl.hpp
//  retro
//
//  Created by Melchor Garau Madrigal on 6/2/18.
//

#pragma once

//I suppose "Platform.hpp" is included already
#include <fstream>

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////

InputFile::InputFile() {}

InputFile::InputFile(const std::string &file, bool binary) {
    open(file, binary);
}

bool InputFile::open(const std::string &file, bool binary) {
    ifstream* stream = new ifstream;
    stream->open(file, ios_base::in | (binary ? ios_base::binary : ios_base::in));
    if(stream->good()) {
        _impl = stream;
        fail = BasicFile::Nothing;
        return true;
    } else {
        delete stream;
        _impl = nullptr;
        fail = BasicFile::CannotOpen;
        return false;
    }
}

bool InputFile::close() {
    ifstream& stream = get_impl<ifstream>();
    stream.close();
    return stream.good();
}

size_t InputFile::read(void* buff, size_t n, size_t sizeOfCType) {
    ifstream& stream = get_impl<ifstream>();
    stream.read((char*) buff, n * sizeOfCType);
    if(stream) {
        return stream.gcount() / sizeOfCType;
    } else {
        if(stream.eof()) {
            fail = BasicFile::EndOfFile;
            return stream.gcount() / sizeOfCType;
        } else {
            fail = BasicFile::CannotRead;
            return 0;
        }
    }
}

off_t InputFile::seeki(off_t offset, BasicFile::SeekDirection dir) {
    ifstream& stream = get_impl<ifstream>();
    ifstream::seekdir stddir;
    if(dir == BasicFile::Beginning) stddir = ifstream::beg;
    if(dir == BasicFile::Current) stddir = ifstream::cur;
    if(dir == BasicFile::End) stddir = ifstream::end;
    stream.seekg(offset, stddir);
    if(stream) {
        return telli();
    } else {
        fail = BasicFile::CannotSeek;
        return -1;
    }
}

off_t InputFile::telli() {
    ifstream& stream = get_impl<ifstream>();
    off_t off = stream.tellg();
    if(off == off_t(-1)) {
        fail = BasicFile::CannotSeek;
    }
    return off;
}

InputFile::~InputFile() {
    if(_impl != nullptr) delete get_impl_ptr<ifstream>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

OutputFile::OutputFile() {}

OutputFile::OutputFile(const string &file, bool binary, bool append) {
    ofstream* stream = new ofstream(file, ios_base::out | (binary ? ios_base::binary : ios_base::out) | (append ? ios_base::ate : ios_base::out));
    if(stream->good()) {
        _impl = stream;
        fail = BasicFile::Nothing;
    } else {
        delete stream;
        _impl = nullptr;
        fail = BasicFile::CannotOpen;
    }
}

bool OutputFile::open(const string &file, bool binary) {
    ofstream* stream = new ofstream(file, ios_base::out | (binary ? ios_base::binary : ios_base::out));
    if(stream->good()) {
        _impl = stream;
        fail = BasicFile::Nothing;
        return true;
    } else {
        delete stream;
        _impl = nullptr;
        fail = BasicFile::CannotOpen;
        return false;
    }
}

bool OutputFile::close() {
    ofstream& stream = get_impl<ofstream>();
    stream.close();
    return stream.good();
}

size_t OutputFile::write(const void* buff, size_t n, size_t sizeOfCType) {
    ofstream& stream = get_impl<ofstream>();
    stream.write((char*) buff, n * sizeOfCType);
    if(stream) {
        return n;
    } else {
        fail = BasicFile::CannotWrite;
        return 0;
    }
}

off_t OutputFile::seeko(off_t offset, BasicFile::SeekDirection dir) {
    ofstream& stream = get_impl<ofstream>();
    ofstream::seekdir stddir;
    if(dir == BasicFile::Beginning) stddir = ofstream::beg;
    if(dir == BasicFile::Current) stddir = ofstream::cur;
    if(dir == BasicFile::End) stddir = ofstream::end;
    stream.seekp(offset, stddir);
    if(stream) {
        return tello();
    } else {
        fail = BasicFile::CannotSeek;
        return -1;
    }
}

off_t OutputFile::tello() {
    ofstream& stream = get_impl<ofstream>();
    off_t off = stream.tellp();
    if(off == off_t(-1)) {
        fail = BasicFile::CannotSeek;
    }
    return off;
}

OutputFile::~OutputFile() {
    if(_impl != nullptr) delete get_impl_ptr<ofstream>();
    _impl = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

InputOutputFile::InputOutputFile(const string &file, bool binary, bool append) {
    fstream* stream = new fstream(file, fstream::in | fstream::out | (binary ? fstream::binary : fstream::out) | (append ? fstream::ate : fstream::out));
    if(stream->good()) {
        _impl = stream;
        fail = BasicFile::Nothing;
    } else {
        delete stream;
        _impl = nullptr;
        fail = BasicFile::CannotOpen;
    }
}

bool InputOutputFile::open(const string &file, bool binary) {
    fstream* stream = new fstream(file, ios_base::out | (binary ? ios_base::binary : ios_base::out));
    if(stream->good()) {
        _impl = stream;
        fail = BasicFile::Nothing;
        return true;
    } else {
        delete stream;
        _impl = nullptr;
        fail = BasicFile::CannotOpen;
        return false;
    }
}

bool InputOutputFile::close() {
    fstream& stream = get_impl<fstream>();
    stream.close();
    return stream.good();
}

InputOutputFile::~InputOutputFile() {
    if(_impl != nullptr) delete get_impl_ptr<fstream>();
    _impl = nullptr;
}
