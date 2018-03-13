#pragma once

#ifndef _WIN32
#include <dirent.h>
#include <cstring>
#else
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#pragma comment(lib, "User32.lib")
#undef min
#undef max
#endif

#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <system_error>
#include <Optional.hpp>
#include <json.hpp>

namespace retro {
    
    static inline std::vector<std::string> listFiles(const std::string &path, bool recursive = true) {
        std::vector<std::string> list;
#ifndef _WIN32
        DIR* dir = opendir(path.c_str());
        dirent* dp;
        do {
            if((dp = readdir(dir)) != nullptr) {
                if(dp->d_type & DT_REG || dp->d_type & DT_LNK) {
                    list.push_back(dp->d_name);
                } else if(dp->d_type & DT_DIR) {
                    if(recursive && strcmp(".", dp->d_name) && strcmp("..", dp->d_name)) {
                        auto newList = listFiles(path + "/" + dp->d_name);
                        std::transform(newList.begin(), newList.end(), newList.begin(), [&dp] (std::string e) {
                            return std::string(dp->d_name) + "/" + e;
                        });
                        list.insert(list.end(), newList.begin(), newList.end());
                    }
                }
            }
        } while(dp != nullptr);
        closedir(dir);
#else
		std::string lepath(path);
		if(*(lepath.end() - 1) != '\\') lepath += '\\';
		lepath += '*';
        WIN32_FIND_DATA ffd;
        HANDLE hFind = INVALID_HANDLE_VALUE;
        hFind = FindFirstFile(lepath.c_str(), &ffd);
        if(INVALID_HANDLE_VALUE == hFind) {
            throw std::runtime_error("Could not found game path");
        }
        
        do {
            if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if(recursive && strcmp(".", ffd.cFileName) && strcmp("..", ffd.cFileName)) {
                    auto newList = listFiles(path + "\\" + ffd.cFileName);
                    std::transform(newList.begin(), newList.end(), newList.begin(), [&ffd] (std::string e) {
                        return std::string(ffd.cFileName) + "\\" + e;
                    });
                    list.insert(list.end(), newList.begin(), newList.end());
                }
            } else {
                list.push_back(ffd.cFileName);
            }
        } while(FindNextFile(hFind, &ffd) != 0);
        FindClose(hFind);
#endif
        return list;
    }

    /// Allow you to open a file, but in fact, is an interface
    class BasicFile {
    public:
        /// Direction for the absolute offset in seek operations
        enum SeekDirection { Beginning, Current, End };
        /// Failure type
        enum FailType { Nothing = 0, CannotOpen = 1, CannotRead = 2, CannotWrite = 4, EndOfFile = 8, CannotSeek = 16, ConversionError = 32 };
    protected:
        void* _impl;
        template<typename type>
        type* get_impl_ptr() { return (type*) _impl; }
        template<typename type>
        type& get_impl() { return *((type*) _impl); }
        FailType fail;
    public:
        /// Opens a file
        virtual bool open(const std::string &file, bool binary = false) = 0;
        /// Closes the file, when it's done.
        virtual bool close() = 0;
        /// Returns true if there's no error
        constexpr operator bool() { return fail == 0; }
        /// Returns true if there's no error
        constexpr bool ok() { return fail == 0; }
        /// Returns the failure kind and clears the fail error
        constexpr FailType failure() { auto f = fail; fail = Nothing; return f; }
        /// Returns true if end of the file is reached
        constexpr bool eof() { return fail & EndOfFile; }
        virtual ~BasicFile() {};
    };

    constexpr auto operator|(BasicFile::FailType a, BasicFile::FailType b) -> BasicFile::FailType {
        return static_cast<BasicFile::FailType>(static_cast<int>(a) | static_cast<int>(b));
    }

    constexpr auto operator&(BasicFile::FailType a, BasicFile::FailType b) -> BasicFile::FailType {
        return static_cast<BasicFile::FailType>(static_cast<int>(a) | static_cast<int>(b));
    }

    constexpr auto operator~(BasicFile::FailType a) -> BasicFile::FailType {
        return static_cast<BasicFile::FailType>(~static_cast<int>(a) & 0x3F);
    }

    constexpr auto operator|=(BasicFile::FailType &a, BasicFile::FailType b) -> BasicFile::FailType {
        return a = (a | b);
    }

    constexpr auto operator&=(BasicFile::FailType &a, BasicFile::FailType b) -> BasicFile::FailType {
        return a = (a & b);
    }

    /// Class that allows you to read files
    class InputFile: public virtual BasicFile {
        virtual size_t read(void* buff, size_t n, size_t sizeOfCType);

        template<typename T, typename lim = std::numeric_limits<T>>
        void conversion(const std::string &nstr, T &num, std::enable_if_t<lim::is_integer && !lim::is_signed>* = 0) {
            try {
                unsigned long long u = stoull(nstr);
                if(u <= lim::max()) {
                    num = T(u);
                } else {
                    num = 0;
                    fail = fail | BasicFile::ConversionError;
                }
            } catch(...) {
                num = 0;
                if(!(fail & BasicFile::EndOfFile))
                    fail = fail | BasicFile::ConversionError;
            }
        }

        template<typename T, typename lim = std::numeric_limits<T>>
        void conversion(const std::string &nstr, T &num, std::enable_if_t<lim::is_integer && lim::is_signed>* = 0) {
            try {
                long long u = stoll(nstr);
                if(lim::min() <= u && u <= lim::max()) {
                    num = T(u);
                } else {
                    num = 0;
                    fail = fail | BasicFile::ConversionError;
                }
            } catch(...) {
                num = 0;
                if(!(fail & BasicFile::EndOfFile))
                    fail = fail | BasicFile::ConversionError;
            }
        }

        template<typename T, typename lim = std::numeric_limits<T>>
        void conversion(const std::string &nstr, T &num, std::enable_if_t<!lim::is_integer>* = 0) {
            try {
                long double u = stold(nstr);
                if(lim::min() <= u && u <= lim::max()) {
                    num = T(u);
                } else {
                    num = T(0.0);
                    fail = fail | BasicFile::ConversionError;
                }
            } catch(...) {
                num = T(0.0);
                if(!(fail & BasicFile::EndOfFile))
                    fail = fail | BasicFile::ConversionError;
            }
        }
    protected:
        InputFile();
    public:
        /// Opens a file to be read
        /**
         * Given a `file` path, opens it to be read. If the file doesn't exist, ok() will
         * return `false` and failure() will be set to CannotOpen. If the file contains
         * binary data, you must set `binary` to `true` to avoid problems while reading.
         * By default, if `binary` is not set, will treat the input using the current
         * locale encoding (usually as UTF-8).
         * @param file Path to the file
         * @param binary True to treat the input as binary data
         **/
        InputFile(const std::string &file, bool binary = false);

        /**
         * Does the same as InputFile().
         * @param file Path to the file
         * @param binary True to treat the input as binary data
         * @return true if everything went ok
         */
        virtual bool open(const std::string &file, bool binary = false) override;

        /**
         * Closes the file. **This method must be called always**.
         * @return true if everything went ok
         */
        virtual bool close() override;

        /**
         * Reads some data into a buffer. It's a templated read version that allows you to
         * use different kind of types (integer types in general). Returns the number of
         * items read (not the number of bytes). If the number is 0, indicates that an
         * error has occurred.
         * @param buff Array of elements where to write
         * @param n Number of elements to read (by default 1)
         * @return Number of elements read or 0 on failure
         */
        template<typename CType = char>
        size_t read(CType* buff, size_t n = 1) { return read(buff, n, sizeof(CType)); }

        /**
         * Seeks the reading position to another one using an absolute offset determined
         * by the direction. If `dir` is BasicFile::Current, the offset is applied from
         * the current position. If `dir` is BasicFile::Beginning, the offset is applied
         * from the beginning of the file (aka relative offset). If `dir` is BasicFile::End
         * the offset is applied from the end of the file. Negative offsets means to go
         * backwards. If the resultant offset is negative, the seek will fail. Returns the
         * relative offset or -1 on failure.
         * @param offset Absolute offset
         * @param dir Direction of the offset
         * @return The relative offset or -1 on failure
         * @see getLastError()
         */
        virtual off_t seeki(off_t offset, BasicFile::SeekDirection dir = BasicFile::Current);

        /**
         * @return The relative offset in which the file is currently, or -1 in case of error
         **/
        virtual off_t telli();
        virtual ~InputFile();

        //Extra read functions
        /// Reads the entire file as a std::string
        std::string read() {
            std::stringstream ss;
            char tmp[1024];
            size_t size;
            while((size = read(tmp, 1024)) != 0) {
                ss.write(tmp, size);
            }
          return ss.str();
        }

        /// Read a line
        std::string readline() {
            std::stringstream ss;
            char c;
            while((c = get()) != '\n') ss << c;
            return ss.str();
        }

        /// Gets a char or another type
        template<typename CType = char>
        CType get() {
            CType b;
            if(read(&b) == 1) return b;
            return 0;
        }

        /// Reads a *word* (anything until read something different of an alphanumeric char)
        /**
         * @return `*this`
         * @see isalnum() http://en.cppreference.com/w/cpp/string/byte/isalnum
         **/
        InputFile& operator>>(std::string &str) {
            char c;
            while(isalnum(c = get())) {
                str.push_back(c);
            }
            return *this;
        }

        /// Reads a character (an alphanumeric character)
        InputFile& operator>>(char &c) {
            do { c = get(); } while(!isalnum(c));
            return *this;
        }

        /// Reads a basic arithmetic type or sets a 0 if cannot read it
        /**
         * The operator tries to read a number from the stream as string,
         * and then tries to convert that string into a number. If the
         * conversion cannot be done, the flag BasicFile::ConversionError
         * will be set. If cannot read from the file, the number will be
         * set to 0 too and the corresponding error will be set as well.
         * For reading characters, there's an specialization for `char`.
         */
        template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
        InputFile& operator>>(T &i) {
            std::string nstr;
            *this >> nstr;
            conversion(nstr, i);
            return *this;
        }
    };

    /// Class that allows you to write files
    class OutputFile: public virtual BasicFile {
        virtual size_t write(const void* buff, size_t n, size_t sizeOfCType);
    protected:
        OutputFile();
    public:
        /// Opens a file to be writen
        /**
         * Given a `file` path, opens it to be writen. If the file doesn't exist, ok() will
         * return `false` and failure() will be set to CannotOpen. If the file contains
         * binary data, you must set `binary` to `true` to avoid problems while writting.
         * By default, if `binary` is not set, will treat the input using the current
         * locale encoding (usually as UTF-8). If the file doesn't exist, it will be created.
         * If the file exists and `append` is set to false (by default), will clear its
         * contents, but if `append` is set to `true`, will start writting at the end of
         * the file.
         * @param file Path to the file
         * @param binary True to treat the input as binary data
         * @param append True to append new data at the end of the file, false to clear
         * the contents if the file already exists, if doesn't exists, does nothing
         **/
        OutputFile(const std::string &file, bool binary = false, bool append = false);

        /**
         * Does the same as OutputFile(), but doesn't allow you to set the `append` value,
         * it will be se always to `false`.
         * @param file Path to the file
         * @param binary True to treat the input as binary data
         * @return true if everything went ok
         **/
        virtual bool open(const std::string &file, bool binary = false) override;

        /**
         * Closes the file. **This method must be called always**.
         * @return true if everything went ok
         */
        virtual bool close() override;

        /**
         * Writes some data from a buffer. It's a templated write version that allows you to
         * use different kind of types (integer types in general). Returns the number of
         * items written (not the number of bytes). If the number is 0, indicates that an
         * error has occurred.
         * @param buff Array of elements where to read
         * @param n Number of elements to write (by default 1)
         * @return Number of elements writen or `0 < returnedValue < n` in case of failure
         */
        template<typename CType = char>
        size_t write(const CType* buff, size_t n = 1) { return write(buff, n, sizeof(CType)); }

        /**
         * Seeks the writting position to another one using an absolute offset determined
         * by the direction. If `dir` is BasicFile::Current, the offset is applied from
         * the current position. If `dir` is BasicFile::Beginning, the offset is applied
         * from the beginning of the file (aka relative offset). If `dir` is BasicFile::End
         * the offset is applied from the end of the file. Negative offsets means to go
         * backwards. If the resultant offset is negative, the seek will fail. Returns the
         * relative offset or -1 on failure.
         * @param offset Absolute offset
         * @param dir Direction of the offset
         * @return The relative offset or -1 on failure
         * @see getLastError()
         */
        virtual off_t seeko(off_t offset, BasicFile::SeekDirection dir = BasicFile::Current);

        /// Returns the current relative offset or -1 in case of error
        virtual off_t tello();
        virtual ~OutputFile();

        //Extra write functions
        /**
         * Writes a std::string into the file.
         * @param str The string to write
         * @return The number of characters writen or `0 < returnedValue < n` in case of failure
         */
        inline size_t write(const std::string &str) { return write(str.c_str(), str.length()); }

        /// Writes the character into the file
        OutputFile& operator<<(char c) {
            write(&c);
            return *this;
        }

        /// Writes the integer representated as a string
        template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
        OutputFile& operator<<(T c) {
            write(std::to_string(c));
            return *this;
        }
    };

    /// Class that allows you to read and write files
    class InputOutputFile: public virtual InputFile, public virtual OutputFile {
    public:
        /// Opens a file to be read and/or writen
        /**
         * Given a `file` path, opens it to be writen and read. If the file doesn't exist,
         * ok() will return `false` and failure() will be set to CannotOpen. If the file
         * contains binary data, you must set `binary` to `true` to avoid problems while
         * writting. By default, if `binary` is not set, will treat the input using the
         * current locale encoding (usually as UTF-8). If the file doesn't exist, it will
         * be created. If the file exists and `append` is set to false (by default), will
         * open at the beginning of the file, but if `append` is set to `true`, will open
         * at the end of the file.
         * @param file Path to the file
         * @param binary True to treat the input as binary data
         * @param append True to append new data at the end of the file, false to clear
         * the contents if the file already exists, if doesn't exists, does nothing
         **/
        InputOutputFile(const std::string &file, bool binary = false, bool append = false);

        /**
         * Does the same as OutputFile(), but doesn't allow you to set the `append` value,
         * it will be se always to `false`.
         * @param file Path to the file
         * @param binary True to treat the input as binary data
         **/
        virtual bool open(const std::string &file, bool binary = false) override;

        /**
         * Closes the file. **This method must be called always**.
         * @return true if everything went ok
         */
        virtual bool close() override;
        virtual ~InputOutputFile();
    };

    /// Gets the current directory (Android will return empty string)
    std::string getCurrentDirectory();

    /// Gets the last error as a std::error_condition. Use `e.message()` to get a string representation.
    std::error_condition getLastError();

    struct Command { nlohmann::json data; void* _priv_data; };
    Optional<Command> getCommand();
    void sendCommandResponse(const Command &, const nlohmann::json &);

#if defined(__APPLE__) && defined(__MACH__)
    void changeDockIcon(void*,unsigned x, unsigned y);
#endif

};
