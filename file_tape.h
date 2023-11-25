#ifndef YADRO_TATLIN_TEST_TASK_FILE_TAPE_H
#define YADRO_TATLIN_TEST_TASK_FILE_TAPE_H

#include "basic_tape.h"

#include <string>
#include <fstream>
#include <chrono>

/**
 * This class emulates a tape by interacting with a text file.
 * The tape is stored integer (int) data.
 * The tape data is stored (and parsed) as follows:
 * "          1         123    10000000             -1234567890 "
 * That is, to store each number,
 * <11 minus the length of string representation of the number> whitespaces is written,
 * then the number is written.
 * The elements are separated by whitespace.
 * Empty elements are allowed (stored as 11 whitespaces).
 * For emulate delays in IO operations, timings from timings_config class are used.
 */
class file_tape : public basic_tape {
public:
    struct timings_config {
        std::chrono::milliseconds read{0};
        std::chrono::milliseconds write{0};
        std::chrono::milliseconds move_left{0};
        std::chrono::milliseconds move_right{0};
        std::chrono::milliseconds rewind{0};

        timings_config() = default;
        /**
         * Attempts to parse `filename` and update default timings values from a content of the file.
         * Allowed format: 'r' or "read" for read timing, 'w' or "write" for write timing,
         * "ml" or "move_left" for move_left timing, "mr" or "move_right" for move_right timing,
         * "rewind" for "rewind" timing.
         * Field name must be followed by '=' and field value.
         * Example: "mr=1337 r=123  move_left=42 rewind=101" (without quotes).
         * Result: { .read=123, .write=0, .move_left=42, .move_right=1337, .rewind=101 }.
         * @param filename path to the config file
         */
        explicit timings_config(std::string const& filename);
        [[nodiscard]] std::string to_string() const;
    };

    /**
     * Creates file_tape that stores data in the given file. The file must has one of the following states:
     * 1. it can contains data in valid format (described above);
     * 2. it can be empty, in that case the file is filled (pre-allocated) with required number of whitespaces;
     * 3. it can yet not exists, in that case an attempt to create the file is being made, then see case 2.
     * Default timings are used.
     * @param filename path to the file where to store data
     * @param size number of elements of the tape
     */
    file_tape(std::string const& filename, size_t size);

    /**
     * Creates file_tape that stores data in the given file. The file must has one of the following states:
     * 1. it can contains data in valid format (described above);
     * 2. it can be empty, in that case the file is filled (pre-allocated) with required number of whitespaces;
     * 3. it can yet not exists, in that case an attempt to create the file is being made, then see case 2.
     * Attempts to parse timings from `config_filename` path. See timings_config::timings_config(std::string const&).
     * @param filename path to the file where to store data
     * @param size number of elements of the tape
     * @param config_filename path to the timings configuration file
     */
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
    timings_config timings;
};


#endif //YADRO_TATLIN_TEST_TASK_FILE_TAPE_H
