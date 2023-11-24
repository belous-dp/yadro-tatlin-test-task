#ifndef YADRO_TATLIN_TEST_TASK_BASIC_TAPE_H
#define YADRO_TATLIN_TEST_TASK_BASIC_TAPE_H

#include <functional>
#include <memory>
#include <optional>

// const basic_tape == read-only
struct basic_tape {
    virtual int read() const = 0; // NOLINT(*-use-nodiscard)
    virtual std::optional<int> read_safe() const = 0; // NOLINT(*-use-nodiscard)
    virtual void write(int data) = 0;
    virtual bool move_left() const = 0; // NOLINT(*-use-nodiscard)
    virtual bool move_right() const = 0; // NOLINT(*-use-nodiscard)
    virtual ~basic_tape() = default;
    virtual void rewind() const {
        while (move_left());
    }
};

using tape_factory = std::function<std::unique_ptr<basic_tape>(size_t)>;

#endif //YADRO_TATLIN_TEST_TASK_BASIC_TAPE_H
