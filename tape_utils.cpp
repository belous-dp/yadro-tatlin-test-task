#include "file_tape.h"
#include "tape_utils.h"
#include "vector_tape.h"

#include <filesystem>
#include <iostream>

void bulk_write(std::vector<int> const& data, basic_tape& tape) {
    for (size_t i = 0; i < data.size(); ++i) {
        tape.write(data[i]);
        if (i + 1 < data.size()) {
            tape.move_right();
        }
    }
}

tape_factory create_file_tape_factory(std::string const& path_to_config) {
    auto factory = [&](size_t size) {
        static size_t cnt = 0;
        cnt++;
        std::filesystem::path path("tmp");
        std::filesystem::create_directory(path);
        path /= "tape" + std::to_string(cnt) + ".txt";
        return std::make_unique<file_tape>(path.string(), size, path_to_config);
    };
    return factory;
}

std::unique_ptr<basic_tape> create_temp_vector_tape(size_t size) {
    return std::make_unique<vector_tape>(size);
}

void print_tape(const basic_tape &tape, std::ostream& out) {
    tape.rewind();
    out << "tape = { ";
    do {
        auto v = tape.read_safe();
        if (v) {
            out << *v << ' ';
        } else {
            out << "_ ";
        }
    } while (tape.move_right());
    out << "}" << std::endl;
}
