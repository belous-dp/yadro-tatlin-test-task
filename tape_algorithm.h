#ifndef YADRO_TATLIN_TEST_TASK_TAPE_ALGORITHM_H
#define YADRO_TATLIN_TEST_TASK_TAPE_ALGORITHM_H

#include "basic_tape.h"

void sort(basic_tape const& src, size_t count, basic_tape& dst, size_t cutoff, tape_factory factory);

#endif //YADRO_TATLIN_TEST_TASK_TAPE_ALGORITHM_H
