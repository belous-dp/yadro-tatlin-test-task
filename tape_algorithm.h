#ifndef YADRO_TATLIN_TEST_TASK_TAPE_ALGORITHM_H
#define YADRO_TATLIN_TEST_TASK_TAPE_ALGORITHM_H

#include "basic_tape.h"
#include <memory>

using tape_factory = std::unique_ptr<basic_tape>(*)(size_t size);

void sort(basic_tape const& src, size_t count, basic_tape& dst, size_t cutoff, tape_factory factory);

#endif //YADRO_TATLIN_TEST_TASK_TAPE_ALGORITHM_H
