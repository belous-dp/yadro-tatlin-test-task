#include "file_tape.h"

#include <cassert>
#include <iostream>
#include <limits>
#include <filesystem>
#include <vector>

namespace {
    void print_state(std::ostream& out, std::fstream& fs) {
        auto state_before = fs.rdstate();
        out << "good: " << (fs.good() ? "yes" : "no");
        if (fs.good()) {
            out << ", posg=" << fs.tellg() << ", char=" << static_cast<char>(fs.peek());
            out << ", posg=" << fs.tellg();
            out << ", posp=" << fs.tellp() << ", char=" << static_cast<char>(fs.peek());
            out << ", posp=" << fs.tellp();
        } else {
            out << " -- eofbit=" << ((fs.rdstate() & std::ios::eofbit) > 0)
                << ", failbit=" << ((fs.rdstate() & std::ios::failbit) > 0)
                << ", fs.badbit=" << ((fs.rdstate() & std::ios::badbit) > 0);
        }
        out << std::endl;
        fs.clear();
        fs.setstate(state_before);
    }
}

const uint8_t file_tape::FILL_LEN = std::to_string(std::numeric_limits<int>::min()).size();
const std::string file_tape::FILL_S = std::string(FILL_LEN, ' ');

file_tape::file_tape(std::string const& filename, size_t size) : file(filename), pos(0), size(size) {
    if (size == 0) {
        throw std::invalid_argument("size of a tape cannot be zero");
    }
    if (!file) {
        throw std::runtime_error("cannot open file " + filename);
    }
    size_t expected_size = size * (FILL_LEN + 1) - 1;
    size_t file_size = std::filesystem::file_size(filename);
    if (file_size < expected_size) {
        if (file_size != 0) {
            throw std::runtime_error("the file must either be empty "
                                     "or contains valid tape of length " + std::to_string(size));
        }
        std::string content(size * (FILL_LEN + 1), ' ');
        file << content << std::endl;
        file.seekg(0, std::ios::beg);
        file.seekp(0, std::ios::beg);
    }
}

void file_tape::update_fstream_pos() const {
    auto spos = static_cast<std::streamoff>(pos) * (FILL_LEN + 1);
    file.seekg(spos);
    file.seekp(spos);
}

int file_tape::read() const {
    std::cout << "entered read" << std::endl;
    print_state(std::cout, file);
    assert(std::isdigit(file.peek()) || file.peek() == '-' || file.peek() == ' ');
    int res;
    file >> res; // what if failed? https://cplusplus.com/reference/istream/istream/operator%3E%3E/
    print_state(std::cout, file);
    update_fstream_pos();
    print_state(std::cout, file);
    assert(std::isdigit(file.peek()) || file.peek() == '-' || file.peek() == ' ');
    std::cout << "exited read" << std::endl;
    return res;
}

std::optional<int> file_tape::read_safe() const {
//    std::cout << "entered read_safe" << std::endl;
    char buf[FILL_LEN];
    file.read(buf, FILL_LEN);
    update_fstream_pos();
    std::string s(buf, FILL_LEN); // maybe file >> std::noskipws >> std::setw(FILL_LEN + 1) >> s?
    try {
        int res = std::stoi(s);
//        std::cout << "exited read_safe (not empty)" << std::endl;
        return {res};
    } catch (std::exception& e) {
        if (s != FILL_S) {
            throw std::runtime_error("file corrupted: " + s);
        }
//        std::cout << "exited read_safe (empty)" << std::endl;
        return {};
    }
}

void file_tape::write(int data) {
    std::cout << "entered write" << std::endl;
    print_state(std::cout, file);
    update_fstream_pos();
    file << std::setw(FILL_LEN) << data << ' '; // todo error handling
    print_state(std::cout, file);
    update_fstream_pos();
    print_state(std::cout, file);
    assert(std::isdigit(file.peek()) || file.peek() == '-' || file.peek() == ' ');
    std::cout << "exited write" << std::endl;
}

bool file_tape::move_left() const {
    print_state(std::cout, file);
    if (pos == 0) {
        return false;
    }
    pos--;
    update_fstream_pos();
    print_state(std::cout, file);
    assert(std::isdigit(file.peek()) || file.peek() == '-' || file.peek() == ' ');
    return true;
}

bool file_tape::move_right() const {
    print_state(std::cout, file);
    assert(std::isdigit(file.peek()) || file.peek() == '-' || file.peek() == ' ');
    if (pos + 1 == size) {
        return false;
    }
    pos++;
    update_fstream_pos();
    print_state(std::cout, file);
    assert(std::isdigit(file.peek()) || file.peek() == '-' || file.peek() == ' ');
    return true;
}
