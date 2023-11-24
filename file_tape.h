#ifndef YADRO_TATLIN_TEST_TASK_FILE_TAPE_H
#define YADRO_TATLIN_TEST_TASK_FILE_TAPE_H

#include "basic_tape.h"

#include <string>
#include <fstream>

class file_tape : public basic_tape {
public:
    struct timings_config {
        uint32_t read = 0;
        uint32_t write = 0;
        uint32_t move_left = 0;
        uint32_t move_right = 0;
        uint32_t rewind = 0;

        timings_config() = default;
        explicit timings_config(std::string const& filename);
        [[nodiscard]] std::string to_string() const;
    };

    file_tape(std::string const& filename, size_t size);
    file_tape(std::string const& filename, size_t size, std::string const& config_filename);

    int read() const override;
    std::optional<int> read_safe() const override;

    void write(int data) override;

    bool move_left() const override;
    bool move_right() const override;

    void rewind() const override;

private:
    void update_fstream_pos() const;

    static const uint8_t FILL_LEN;
    static const std::string FILL_S;

    mutable std::fstream file;
    mutable size_t pos;
    const size_t size;
    mutable timings_config timings;
};


#endif //YADRO_TATLIN_TEST_TASK_FILE_TAPE_H
