#include "file_tape.h"
#include "tape_utils.h"

#include <filesystem>
#include <iostream>

void bulk_write(std::vector<int> const& data, basic_tape& tape) {
    for (int i : data) {
        tape.write(i);
        tape.move_right();
    }
}

std::unique_ptr<basic_tape> create_temp_tape(size_t size) {
    static size_t cnt = 0;
    cnt++;
    std::filesystem::path path("tmp");
    std::filesystem::create_directory(path);
    path /= "tape" + std::to_string(cnt) + ".txt";
    std::ofstream{path};
    return std::make_unique<file_tape>(path.string(), size);
}

void print_tape(const basic_tape &tape) {
    tape.rewind();
    std::cout << "tape = { ";
    do {
        auto v = tape.read_safe();
        if (v) {
            std::cout << *v << ' ';
        } else {
            std::cout << "_ ";
        }
    } while (tape.move_right());
    std::cout << " }" << std::endl;
}
