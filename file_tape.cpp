#include "file_tape.h"

#include <cassert>
#include <iostream>

namespace {
    void seek(std::fstream& fs, std::streamoff offset) {
        fs.seekg(offset, std::ios::cur);
        fs.seekp(offset, std::ios::cur);
    }

    void print_state(std::ostream& out, std::fstream& fs) {
        out << "good: " << (fs.good() ? "yes" : "no");
        if (fs.good()) {
            assert(fs.tellg() == fs.tellp());
            out << ", pos=" << fs.tellg() << ", char=" << static_cast<char>(fs.peek());
            out << ", pos=" << fs.tellg();
        }
        out << std::endl;
    }
}

file_tape::file_tape(std::string const& filename) : file(filename) {
    if (!file) {
        throw std::runtime_error("cannot open file " + filename);
    }
}

int file_tape::read() const {
    int res;
    file >> res; // what if failed? https://cplusplus.com/reference/istream/istream/operator%3E%3E/
    return res;
}

void file_tape::write(int data) {
    file << data << ' '; // todo error handling
}

void file_tape::move_left() const {
    assert(file.tellg() == file.tellp());
//    print_state(std::cout, file);
    if (!std::isspace(file.peek())) {
        // beginning of the file.
        // it is impossible to use istream::tellg() for the check
        // because internal buffer position is modified by istream::peek()
        return;
    }
    seek(file, -1);
//    print_state(std::cout, file);
    assert(std::isdigit(file.peek()));
    while (file && !isspace(static_cast<char>(file.peek()))) {
        seek(file, -1);
//        std::cout << "loop: ";
//        print_state(std::cout, file);
    }
    // todo error handling (file.bad())
//    std::cout << "exited from loop" << std::endl;
//    std::cout << "eof=" << file.eof() << std::endl;
//    std::cout << "fail=" << file.fail() << std::endl;
//    std::cout << "bad=" << file.bad() << std::endl;
    file.clear(); // clear failbit in case of reaching the beginning of the file
//    print_state(std::cout, file);
    assert(file.tellg() == file.tellp());
}

void file_tape::move_right() const {
    read();
}
