#include "file_tape.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

namespace {
    [[maybe_unused]] void print_state(std::ostream& out, std::fstream& fs) {
        auto state_before = fs.rdstate();
        out << "good: " << (fs.good() ? "yes" : "no");
        if (fs.good()) {
            out << ", posg=" << fs.tellg() << ", char=" << static_cast<char>(fs.peek());
            out << ", posg=" << fs.tellg();
            out << ", posp=" << fs.tellp() << ", char=" << static_cast<char>(fs.peek());
            out << ", posp=" << fs.tellp();
        } else {
            out << " -- eofbit=" << ((fs.rdstate() & std::fstream::eofbit) > 0)
                << ", failbit=" << ((fs.rdstate() & std::fstream::failbit) > 0)
                << ", fs.badbit=" << ((fs.rdstate() & std::fstream::badbit) > 0);
        }
        out << std::endl;
        fs.clear();
        fs.setstate(state_before);
    }
}

const uint8_t file_tape::FILL_LEN = std::to_string(std::numeric_limits<int>::min()).size();
const std::string file_tape::FILL_S = std::string(FILL_LEN, ' ');

file_tape::file_tape(std::string const& filename, size_t size) : pos(0), size(size) {
    if (size == 0) {
        throw std::invalid_argument("size of a tape cannot be zero");
    }
    if (!std::filesystem::exists(filename)) {
        std::ofstream created(filename);
        if (!created) {
            throw std::runtime_error("cannot create file " + filename);
        }
    }
    file = std::fstream(filename);
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
    }
    file.exceptions(std::fstream::badbit);
}

file_tape::file_tape(const std::string& filename, size_t size, const std::string& config_filename)
                    : file_tape(filename, size) {
    try {
        timings_config config(config_filename);
        timings = config;
    } catch (std::runtime_error& ignored) {
        std::ofstream config_file(config_filename);
        if (config_file) {
            config_file << timings.to_string() << std::endl;
        }
    }
}

void file_tape::update_fstream_pos() const {
    auto spos = static_cast<std::streamoff>(pos) * (FILL_LEN + 1);
    file.seekg(spos);
    file.seekp(spos);
}

int file_tape::read() const {
    std::this_thread::sleep_for(std::chrono::milliseconds(timings.read));
    int res;
    update_fstream_pos();
    file >> res;
    return res;
}

std::optional<int> file_tape::read_safe() const {
    std::this_thread::sleep_for(std::chrono::milliseconds(timings.read));
    char buf[FILL_LEN];
    update_fstream_pos();
    file.read(buf, FILL_LEN);
    std::string s(buf, FILL_LEN);
    try {
        int res = std::stoi(s);
        return { res };
    } catch (std::exception& e) {
        if (s != FILL_S) {
            throw std::runtime_error("file corrupted: " + s);
        }
        return {};
    }
}

void file_tape::write(int data) {
    std::this_thread::sleep_for(std::chrono::milliseconds(timings.write));
    update_fstream_pos();
    file << std::setw(FILL_LEN) << data << ' ';
}

bool file_tape::move_left() const {
    if (pos == 0) {
        return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(timings.move_left));
    pos--;
    return true;
}

bool file_tape::move_right() const {
    if (pos + 1 == size) {
        return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(timings.move_right));
    pos++;
    return true;
}

void file_tape::rewind() const {
    std::this_thread::sleep_for(std::chrono::milliseconds(timings.rewind));
    pos = 0;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"

file_tape::timings_config::timings_config(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("cannot open config file " + filename);
    }
    std::string entry;
    while (file >> entry) {
        auto pos = entry.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        auto field = std::string_view(entry.begin(), entry.begin() + pos);  // NOLINT(*-narrowing-conversions)
        try {
            auto value = std::stoul(entry.substr(pos + 1));
            if (field == "rewind") {
                rewind = value;
            } else if (field.starts_with('r')) {
                read = value;
            } else if (field.starts_with('w')) {
                write = value;
            } else if (field == "move_left" || field == "ml") {
                move_left = value;
            } else if (field == "move_right" || field == "mr") {
                move_right = value;
            }
        } catch (std::invalid_argument& e) {
            continue;
        } catch (std::out_of_range& e) {
            continue;
        }
    }
}

#pragma clang diagnostic pop

std::string file_tape::timings_config::to_string() const {
    std::stringstream ss;
    ss << "read=" << read << " write=" << write;
    ss << " move_left=" << move_left << " move_right=" << move_right;
    ss << " rewind=" << rewind;
    return ss.str();
}
