#ifndef YADRO_TATLIN_TEST_TASK_FILE_TAPE_H
#define YADRO_TATLIN_TEST_TASK_FILE_TAPE_H

#include "basic_tape.h"

#include <string>
#include <fstream>

class file_tape : public basic_tape {
public:
    explicit file_tape(std::string const& filename);

    int read() const override;
    void write(int data) override;

    void move_left() const override;
    void move_right() const override;

private:
    mutable std::fstream file;
};


#endif //YADRO_TATLIN_TEST_TASK_FILE_TAPE_H
