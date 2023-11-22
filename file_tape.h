#ifndef YADRO_TATLIN_TEST_TASK_FILE_TAPE_H
#define YADRO_TATLIN_TEST_TASK_FILE_TAPE_H

#include "basic_tape.h"

#include <string>
#include <fstream>

class file_tape : public basic_tape {
public:
    file_tape(std::string const& filename, size_t size);

    int read() const override;
    std::optional<int> read_safe() const override;

    void write(int data) override;

    bool move_left() const override;
    bool move_right() const override;

private:
    mutable std::fstream file;
};


#endif //YADRO_TATLIN_TEST_TASK_FILE_TAPE_H
