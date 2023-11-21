#ifndef YADRO_TATLIN_TEST_TASK_BASIC_TAPE_H
#define YADRO_TATLIN_TEST_TASK_BASIC_TAPE_H

// const basic_tape == read-only
struct basic_tape {
    virtual int read() const = 0;
    virtual void write(int data) = 0;
    virtual void move_left() const = 0;
    virtual void move_right() const = 0;
    virtual ~basic_tape() = default;
};


#endif //YADRO_TATLIN_TEST_TASK_BASIC_TAPE_H
