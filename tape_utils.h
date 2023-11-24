#ifndef YADRO_TATLIN_TEST_TASK_TAPE_UTILS_H
#define YADRO_TATLIN_TEST_TASK_TAPE_UTILS_H

#include "basic_tape.h"

#include <vector>

void bulk_write(std::vector<int> const& data, basic_tape& tape);

tape_factory create_file_tape_factory(std::string const& path_to_config);

std::unique_ptr<basic_tape> create_temp_vector_tape(size_t size);

void print_tape(basic_tape const& tape, std::ostream& out);


#endif //YADRO_TATLIN_TEST_TASK_TAPE_UTILS_H
