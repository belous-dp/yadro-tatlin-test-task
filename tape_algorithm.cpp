#include "tape_algorithm.h"
#include "tape_utils.h"

#include <algorithm>

namespace {
    /**
     * Merges two src tapes (which are split into blocks of size `block_size`)
     * to two dst tapes (which will be split into blocks of size `block_size`*2).
     * Each block is sorted either ascending or descending
     * (depending on the current step in the merge sort algorithm).
     * If src tapes blocks are sorted in ascending order,
     * then dst blocks will be sorted in descending order, and vice versa.
     * @param src1 left source tape
     * @param src2 right source tape
     * @param dst1 left destination tape
     * @param dst2 right destination tape
     * @param n_layers number of layers on src tapes (max number of blocks on either of src tapes); equals total number of blocks on dst tapes
     * @param last_layer_id last layer ID
     * @param n_elems number of elements to sort
     * @param n_blocks total number of blocks on src tapes
     * @param block_size current (src) size of the blocks
     * @param cmp_greater used to set the order of elements
     */
    void merge(basic_tape const* src1, basic_tape const* src2,
               basic_tape* dst1, basic_tape* dst2,
               size_t n_layers, size_t last_layer_id,
               size_t n_elems, size_t n_blocks, size_t block_size,
               int cmp_greater) {
        for (size_t i = 0; i < n_layers; ++i) {
            size_t left_block_size, right_block_size;
            if (i == last_layer_id) {
                size_t last = n_elems % block_size;
                if (last == 0) {
                    last = block_size;
                }
                if (n_blocks % 2) {
                    left_block_size = last;
                    right_block_size = 0;
                } else {
                    left_block_size = block_size;
                    right_block_size = last;
                }
            } else {
                left_block_size = right_block_size = block_size;
            }

            // basic merge, but we move from right to left on src tapes
            for (size_t left = 0, right = 0; left < left_block_size || right < right_block_size;) {
                if (left >= left_block_size) {
                    dst1->write(src2->read());
                    src2->move_left();
                    right++;
                } else if (right >= right_block_size) {
                    dst1->write(src1->read());
                    src1->move_left();
                    left++;
                } else {
                    int e1 = src1->read();
                    int e2 = src2->read();
                    if ((e1 < e2) ^ cmp_greater) {
                        dst1->write(e1);
                        src1->move_left();
                        left++;
                    } else {
                        dst1->write(e2);
                        src2->move_left();
                        right++;
                    }
                }
                if (left < left_block_size || right < right_block_size || i + 2 < n_layers) {
                    dst1->move_right();
                }
            }

            std::swap(dst1, dst2);
        }
    }

    /**
     * Performs iterative merge sort algorithm. Scans src tapes from right to left,
     * and stores merged sorted blocks to dst tapes from left to right.
     * The size of blocks is doubled on each step.
     * The result is stored in reversed order each two iterations of the algorithm.
     * @param n_elems number of elements to sort
     * @param cutoff initial size of blocks (already sorted and stored in tape1 and tape2)
     * @param tape1 must point at its last element
     * @param tape2 must point at its last element
     * @param tape3 must point at beginning of the tape
     * @param tape4 must point at beginning of the tape
     * @return number of iterations of the algorithm
     */
    size_t merge_sort(size_t n_elems, size_t cutoff,
                    basic_tape* tape1, basic_tape* tape2,
                    basic_tape* tape3, basic_tape* tape4) {

        size_t n_steps = 1;
        size_t block_size = cutoff;
        size_t n_blocks = (n_elems + cutoff - 1) / cutoff;
        while (block_size < n_elems) { // log(n) merges

            size_t n_layers = (n_blocks + 1) / 2; // also equals n_blocks on the next step
            bool first_write_to_tape3 = n_layers % 2 || (n_steps % 2 == 0);
            merge(tape1, tape2,
                  (first_write_to_tape3 ? tape3 : tape4), (first_write_to_tape3 ? tape4 : tape3),
                  n_layers, (n_steps % 2 ? 0 : n_layers - 1), n_elems, n_blocks, block_size,
                  static_cast<int>(n_steps) % 2);

            std::swap(tape1, tape3);
            std::swap(tape2, tape4);
            block_size *= 2;
            n_blocks = n_layers;
            n_steps++;
        }
        return n_steps;
    }

    /**
     * Splits the source tape into two tapes as evenly as possible.
     * Writes to the dst tapes by sorted blocks of size `cutoff`.
     * The number of elements on the dst1 tape is guaranteed to be not less than
     * the number of elements on the dst2.
     * @param src
     * @param n_elems
     * @param cutoff
     * @param dst1
     * @param dst2
     */
    void split_tape(basic_tape const& src, size_t n_elems, size_t cutoff,
                    basic_tape* dst1, basic_tape* dst2) {
        size_t n_blocks = (n_elems + cutoff - 1) / cutoff;
        std::vector<int> mem(cutoff);
        for (size_t i = 0, nb = 0; i < n_elems; i += cutoff, ++nb) {
            for (size_t j = 0; j < cutoff && i + j < n_elems; ++j) {
                mem[j] = src.read();
                src.move_right();
            }
            if (i + cutoff >= n_elems) {
                mem.resize(n_elems - i);
            }
            std::sort(mem.begin(), mem.end());
            bulk_write(mem, *dst1);
            if (nb + 2 < n_blocks) {
                dst1->move_right();
            }
            std::swap(dst1, dst2);
        }
    }
}

void sort(basic_tape const& src, size_t count, basic_tape& dst, size_t cutoff, tape_factory const& factory) {
    if (count == 0) {
        return;
    }
    if (cutoff == 0) {
        throw std::invalid_argument("cutoff must be positive integer");
    }

    auto tt1 = factory(count);
    auto tt2 = factory(count);
    auto tt3 = factory(count);

    split_tape(src, count, cutoff, &dst, tt1.get());
    size_t n_steps = merge_sort(count, cutoff, &dst, tt1.get(), tt2.get(), tt3.get());
    if (n_steps % 2 == 0) {
        // sorted data is stored in tt2 but reversed
        for (size_t i = 0; i < count; ++i) {
            dst.write(tt2->read());
            tt2->move_left();
            dst.move_right();
        }
    }
}
