#ifndef YADRO_TATLIN_TEST_TASK_BASIC_TAPE_H
#define YADRO_TATLIN_TEST_TASK_BASIC_TAPE_H

#include <functional>
#include <memory>
#include <optional>

/**
 * Interface that abstracts a tape.
 * const basic_tape means read-only tape.
 * All methods can block.
 * Empty elements are allowed.
 */
struct basic_tape {
    /**
     * Reads current element from the tape.
     * @return current element of the tape
     */
    virtual int read() const = 0; // NOLINT(*-use-nodiscard)

    /**
     * If current element is empty, the method returns null optional.
     * Otherwise the method returns the same data as basic_tape::read(),
     * but (usually) works slower.
     * @return optionally returns current element of the tape
     */
    virtual std::optional<int> read_safe() const = 0; // NOLINT(*-use-nodiscard)

    /**
     * Writes `data` to the current position on the tape.
     * @param data data to write
     */
    virtual void write(int data) = 0;

    /**
     * Moves left reading/writing head of the tape.
     * If the head is already on the left-most position, does nothing.
     * @return true the head of the tape was moved left, false otherwise
     */
    virtual bool move_left() const = 0; // NOLINT(*-use-nodiscard)

    /**
     * Moves right reading/writing head of the tape.
     * If the head is already on the right-most position, does nothing.
     * @return true the head of the tape was moved right, false otherwise
     */
    virtual bool move_right() const = 0; // NOLINT(*-use-nodiscard)

    /**
     * Destroys the tape. Frees resources.
     */
    virtual ~basic_tape() = default;

    /**
     * Sets the head of the tape in left-most position.
     */
    virtual void rewind() const {
        while (move_left());
    }
};

using tape_factory = std::function<std::unique_ptr<basic_tape>(size_t)>;

#endif //YADRO_TATLIN_TEST_TASK_BASIC_TAPE_H
