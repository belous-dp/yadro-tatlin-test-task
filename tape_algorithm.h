#ifndef YADRO_TATLIN_TEST_TASK_TAPE_ALGORITHM_H
#define YADRO_TATLIN_TEST_TASK_TAPE_ALGORITHM_H

#include "basic_tape.h"

/**
 * Sorts src1 using merge sort algorithm. Uses three additional tapes created by `factory`.
 * @param src source (input) tape. It must points to the first element from which to start sorting numbers.
 * @param count number of elements to sort.
 * @param dst destination tape. It must points to the first element from which to start writing numbers.
 * @param cutoff number of elements that can be sorted in RAM. Cannot be zero.
 * @param factory function to create temporary tapes.
 */
void sort(basic_tape const& src, size_t count, basic_tape& dst, size_t cutoff, tape_factory const& factory);

#endif //YADRO_TATLIN_TEST_TASK_TAPE_ALGORITHM_H
