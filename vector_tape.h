#ifndef YADRO_TATLIN_TEST_TASK_VECTOR_TAPE_H
#define YADRO_TATLIN_TEST_TASK_VECTOR_TAPE_H

#include "basic_tape.h"
#include <vector>

class vector_tape : public basic_tape {
public:
    explicit vector_tape(size_t size);
    explicit vector_tape(std::vector<int> content);

    int read() const override;
    std::optional<int> read_safe() const override;

    void write(int data) override;

    bool move_left() const override;
    bool move_right() const override;

    void rewind() const override;

private:
    mutable std::vector<int> v;
    mutable std::vector<bool> empty;
    mutable size_t pos = 0;
};


#endif //YADRO_TATLIN_TEST_TASK_VECTOR_TAPE_H
