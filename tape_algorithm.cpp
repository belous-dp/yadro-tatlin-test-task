#include "tape_algorithm.h"
#include "tape_utils.h"

#include <algorithm>
#include <cassert>
#include <iostream>

namespace {
    void merge_sort(basic_tape const &src, size_t n_elems, size_t cutoff,
                    basic_tape *tape1, basic_tape *tape2,
                    basic_tape *tape3, basic_tape *tape4) {
        size_t n_blocks = 0;
        std::vector<int> mem(cutoff);
        for (size_t i = 0; i < n_elems; i += cutoff, ++n_blocks) {
            for (size_t j = 0; j < cutoff && i + j < n_elems; ++j) {
                mem[j] = src.read();
                src.move_right();
            }
            if (i + cutoff >= n_elems) {
                mem.resize(n_elems - i);
            }
            std::sort(mem.begin(), mem.end());
            if (n_blocks % 2 == 0) {
                bulk_write(mem, *tape1);
            } else {
                bulk_write(mem, *tape2);
            }
        }

        size_t block_size = cutoff;
        while (block_size < n_elems) { // log(n) merges
            tape1->rewind();
            tape2->rewind();
            tape3->rewind();
            tape4->rewind();
            for (size_t i = 0; i < n_blocks; i += 2) { // merge two blocks from parallel tapes
                size_t left_block_size, right_block_size;
                if (i + 2 >= n_blocks) {
                    if (n_blocks % 2) {
                        left_block_size = n_elems % block_size;
                        if (left_block_size == 0) {
                            left_block_size = block_size;
                        }
                        right_block_size = 0;
                    } else {
                        left_block_size = block_size;
                        right_block_size = n_elems % block_size;
                        if (right_block_size == 0) {
                            right_block_size = block_size;
                        }
                    }
                } else {
                    left_block_size = right_block_size = block_size;
                }

                basic_tape &res = (i % 4 == 0 ? *tape3 : *tape4); // store result in one of next-step tapes

                // basic merge
                for (size_t left = 0, right = 0; left < left_block_size || right < right_block_size;) {
                    if (left >= left_block_size) {
                        res.write(tape2->read());
                        res.move_right();
                        tape2->move_right();
                        right++;
                    } else if (right >= right_block_size) {
                        res.write(tape1->read());
                        res.move_right();
                        tape1->move_right();
                        left++;
                    } else {
                        int e1 = tape1->read();
                        int e2 = tape2->read();
                        if (e1 < e2) {
                            res.write(e1);
                            tape1->move_right();
                            left++;
                        } else {
                            res.write(e2);
                            tape2->move_right();
                            right++;
                        }
                        res.move_right();
                    }
                }
            }

            std::swap(tape1, tape3);
            std::swap(tape2, tape4);
            block_size *= 2;
            n_blocks = (n_blocks + 1) / 2;
        }
    }
}

void sort(basic_tape const& src, size_t count, basic_tape& dst, size_t cutoff) {
    assert(cutoff > 0);

    size_t n_steps = 1;
    size_t power = cutoff;
    while (power < count) {
        power *= 2;
        n_steps++;
    }

    auto tt1 = create_temp_tape(count);
    auto tt2 = create_temp_tape(count);
    auto tt3 = create_temp_tape(count);

    if (n_steps % 2) {
        merge_sort(src, count, cutoff, &dst, tt1.get(), tt2.get(), tt3.get());
    } else {
        // sorted data will be stored in tt2
        // we want tt2 to be dst
        merge_sort(src, count, cutoff, tt2.get(), tt1.get(), &dst, tt3.get());
    }

//    print_tape(dst, std::cerr);
    dst.rewind();
}
