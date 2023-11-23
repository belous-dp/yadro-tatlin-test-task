#include "vector_tape.h"
#include <stdexcept>

vector_tape::vector_tape(size_t size) : v(size), empty(size, true) {
    if (size == 0) {
        throw std::invalid_argument("size of a tape cannot be zero");
    }
}

vector_tape::vector_tape(std::vector<int> content) : v(std::move(content)), empty(0) {
    if (v.empty()) {
        throw std::invalid_argument("size of a tape cannot be zero");
    }
}

int vector_tape::read() const {
    return v[pos];
}

std::optional<int> vector_tape::read_safe() const {
    if (!empty.empty() && empty[pos]) {
        return {};
    }
    return v[pos];
}

void vector_tape::write(int data) {
    v[pos] = data;
    if (!empty.empty()) {
        empty[pos] = false;
    }
}

bool vector_tape::move_left() const {
    if (pos) {
        pos--;
        return true;
    }
    return false;
}

bool vector_tape::move_right() const {
    if (pos + 1 < v.size()) {
        pos++;
        return true;
    }
    return false;
}

void vector_tape::rewind() const {
    pos = 0;
}
