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
    void update_fstream_pos() const;

    static const uint8_t FILL_LEN;
    static const std::string FILL_S;

    mutable std::fstream file;
    mutable size_t pos;
    const size_t size;
};


#endif //YADRO_TATLIN_TEST_TASK_FILE_TAPE_H
