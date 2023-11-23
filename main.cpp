#include "file_tape.h"
#include "tape_algorithm.h"
#include "tape_utils.h"

#include <filesystem>
#include <iostream>


int main() {
    size_t n = 16;
    file_tape src("src.txt", n);
    std::ofstream{"dst.txt"};
    file_tape dst("dst.txt", n);
    sort(src, n, dst, 3);
    print_tape(dst, std::cout);
}
