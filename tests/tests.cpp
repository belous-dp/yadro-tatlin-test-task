#include <gtest/gtest.h>
#include <filesystem>
#include "file_tape.h"

namespace {
    std::string create_temp_file() {
        std::filesystem::path path("tmp");
        path /= "testing";
        std::filesystem::create_directory(path);
        auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
        path /= std::string(test_info->test_suite_name()) + "__" + std::string(test_info->name()) + ".txt";
        std::ofstream{path}; // NOLINT(*-unused-raii)
        return path.string();
    }

    struct file_tape_fixture : testing::Test {
        void SetUp() override {
            auto filename = create_temp_file();
            {
                std::ofstream file(filename);
                file << "        123         456         789\n";
            }
            tape = std::make_unique<file_tape>(filename, 3);
        }

        std::unique_ptr<file_tape> tape;
    };
}

TEST(file_tape, ctor_no_file) {
    EXPECT_ANY_THROW({
        file_tape tape("not_existing_file_name", 42);
    });
}

TEST(file_tape, ctor_empty_file) {
    auto filename = create_temp_file();
    {
        file_tape tape(filename, 2);
    }
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);
    ASSERT_EQ(line, "                        ");
}

TEST(file_tape, ctor_partially_filled_file) {
    auto filename = create_temp_file();
    {
        std::ofstream file(filename);
        file << "                     42            \n";
    }
    {
        file_tape tape(filename, 3);
    }
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);
    ASSERT_EQ(line, "                     42            ");
}

TEST_F(file_tape_fixture, read_one) {
    EXPECT_EQ(tape->read(), 123);
}

TEST_F(file_tape_fixture, read_again) {
    EXPECT_EQ(tape->read(), 123);
    EXPECT_EQ(tape->read(), 123);
}

TEST_F(file_tape_fixture, move_right) {
    EXPECT_TRUE(tape->move_right());
    EXPECT_EQ(tape->read(), 456);
}

TEST_F(file_tape_fixture, move_right_subsequent) {
    EXPECT_TRUE(tape->move_right());
    EXPECT_TRUE(tape->move_right());
    EXPECT_EQ(tape->read(), 789);
}

TEST_F(file_tape_fixture, move_right_too_much) {
    EXPECT_TRUE(tape->move_right());
    EXPECT_TRUE(tape->move_right());
    EXPECT_FALSE(tape->move_right());
    EXPECT_FALSE(tape->move_right());
}

TEST(file_tape, move_right_not_filled) {
    file_tape tape(create_temp_file(), 3);
    EXPECT_TRUE(tape.move_right());
    EXPECT_TRUE(tape.move_right());
    EXPECT_FALSE(tape.move_right());
    EXPECT_FALSE(tape.move_right());
}

TEST_F(file_tape_fixture, read_many) {
    EXPECT_EQ(tape->read(), 123);
    EXPECT_TRUE(tape->move_right());
    EXPECT_EQ(tape->read(), 456);
    EXPECT_EQ(tape->read(), 456);
    EXPECT_TRUE(tape->move_right());
    EXPECT_EQ(tape->read(), 789);
    EXPECT_EQ(tape->read(), 789);
    EXPECT_EQ(tape->read(), 789);
}

TEST(file_tape, read_not_filled) {
    file_tape tape(create_temp_file(), 3);
    EXPECT_FALSE(tape.read_safe().has_value());
    tape.move_right();
    EXPECT_FALSE(tape.read_safe().has_value());
    tape.move_right();
    EXPECT_FALSE(tape.read_safe().has_value());
}

TEST(file_tape, read_partially_filled) {
    auto filename = create_temp_file();
    {
        std::ofstream file(filename);
        file << "                     42            \n";
    }
    file_tape tape(filename, 3);
    EXPECT_FALSE(tape.read_safe().has_value());
    tape.move_right();
    ASSERT_TRUE(tape.read_safe().has_value());
    EXPECT_EQ(tape.read_safe().value(), 42);
    tape.move_right();
    EXPECT_FALSE(tape.read_safe().has_value());
}

TEST_F(file_tape_fixture, move_left) {
    EXPECT_FALSE(tape->move_left());
    EXPECT_TRUE(tape->move_right());
    EXPECT_EQ(tape->read(), 456);
    EXPECT_TRUE(tape->move_left());
    EXPECT_EQ(tape->read(), 123);
}

TEST_F(file_tape_fixture, move_left_many) {
    EXPECT_TRUE(tape->move_right());
    EXPECT_TRUE(tape->move_right());
    EXPECT_EQ(tape->read(), 789);
    EXPECT_TRUE(tape->move_left());
    EXPECT_TRUE(tape->move_left());
    EXPECT_FALSE(tape->move_left());
    EXPECT_EQ(tape->read(), 123);
    EXPECT_FALSE(tape->move_left());
}

TEST(file_tape, move_left_not_filled) {
    file_tape tape(create_temp_file(), 3);
    tape.move_right();
    tape.move_right();
    EXPECT_TRUE(tape.move_left());
    EXPECT_TRUE(tape.move_left());
    EXPECT_FALSE(tape.move_left());
}

TEST_F(file_tape_fixture, write_simple) {
    tape->write(42);
    EXPECT_EQ(tape->read(), 42);
    tape->write(420);
    EXPECT_EQ(tape->read(), 420);
}

TEST_F(file_tape_fixture, write_no_affects_others) {
    tape->move_right();
    tape->write(42);
    EXPECT_EQ(tape->read(), 42);
    tape->move_left();
    EXPECT_EQ(tape->read(), 123);
    tape->move_right();
    tape->move_right();
    EXPECT_EQ(tape->read(), 789);
}

TEST(file_tape, write_partially_filled) {
    auto filename = create_temp_file();
    {
        std::ofstream file(filename);
        file << "                     42            \n";
    }
    {
        file_tape tape(filename, 3);
        tape.write(0);
        ASSERT_TRUE(tape.read_safe().has_value());
        EXPECT_EQ(tape.read_safe().value(), 0);
        tape.move_right();
        ASSERT_TRUE(tape.read_safe().has_value());
        EXPECT_EQ(tape.read_safe().value(), 42);
        tape.write(0);
        ASSERT_TRUE(tape.read_safe().has_value());
        EXPECT_EQ(tape.read_safe().value(), 0);
        tape.move_right();
        tape.write(7);
        ASSERT_TRUE(tape.read_safe().has_value());
        EXPECT_EQ(tape.read_safe().value(), 7);
    }
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);
    ASSERT_EQ(line, "          0           0           7 ");
}

TEST(file_tape, write_int_min) {
    auto filename = create_temp_file();
    {
        std::ofstream file(filename);
        file << "                     42            \n";
    }
    const int int_min = std::numeric_limits<int>::min();
    {
        file_tape tape(filename, 3);
        tape.write(int_min);
        ASSERT_TRUE(tape.read_safe().has_value());
        EXPECT_EQ(tape.read_safe().value(), int_min);
        tape.move_right();
        tape.write(int_min);
        ASSERT_TRUE(tape.read_safe().has_value());
        EXPECT_EQ(tape.read_safe().value(), int_min);
        tape.move_right();
        tape.write(int_min);
        ASSERT_TRUE(tape.read_safe().has_value());
        EXPECT_EQ(tape.read_safe().value(), int_min);
    }
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);
    ASSERT_EQ(line, "-2147483648 -2147483648 -2147483648 ");
}
