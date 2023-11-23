#ifndef YADRO_TATLIN_TEST_TASK_TAPE_UTILS_H
#define YADRO_TATLIN_TEST_TASK_TAPE_UTILS_H

#include "basic_tape.h"

#include <memory>
#include <vector>

void bulk_write(std::vector<int> const& data, basic_tape& tape);

std::unique_ptr<basic_tape> create_temp_tape(size_t size);

void print_tape(basic_tape const& tape, std::ostream& out);


#endif //YADRO_TATLIN_TEST_TASK_TAPE_UTILS_H
