#include <filesystem>
#include <gtest/gtest.h>
#include <random>
#include <file_tape.h>
#include <tape_algorithm.h>
#include <tape_utils.h>
#include <vector_tape.h>


namespace {
    std::string create_temp_filename() {
        static size_t cnt = 0;
        cnt++;
        std::filesystem::path path("tmp");
        path /= "testing";
        std::filesystem::create_directories(path);
        auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
        path /= std::string(test_info->test_suite_name()) + "__" + std::string(test_info->name()) +
                "__" + std::to_string(cnt) + ".txt";
        return path.string();
    }

    const uint8_t FILL_LEN = std::to_string(std::numeric_limits<int>::min()).size();
    const std::string FILE_TAPE_CONFIG_NAME = "file_tape_tests.cfg";

    std::string file_tape_content_from_vec(std::vector<int> const& v) {
        std::stringstream ss;
        for (int i : v) {
            ss << std::setw(FILL_LEN) << i << ' ';
        }
        ss << std::endl;
        return ss.str();
    }

    struct file_tape_fixture : testing::Test {
        void SetUp() override {
            auto filename = create_temp_filename();
            {
                std::ofstream file(filename);
                file << "        123         456         789\n";
            }
            tape = std::make_unique<file_tape>(filename, 3, FILE_TAPE_CONFIG_NAME);
        }

        std::unique_ptr<file_tape> tape;
    };
}

TEST(file_tape, ctor_no_file) {
    EXPECT_ANY_THROW({
        file_tape tape("/not_existing_file_name", 42);
    });
}

TEST(file_tape, timings_default) {
    auto filename = create_temp_filename();
    std::filesystem::remove(FILE_TAPE_CONFIG_NAME);
    {
        file_tape tape(filename, 1, FILE_TAPE_CONFIG_NAME);
    }
    ASSERT_TRUE(std::filesystem::exists(FILE_TAPE_CONFIG_NAME));
    file_tape::timings_config timings(FILE_TAPE_CONFIG_NAME);
    EXPECT_EQ(timings.read, 0);
    EXPECT_EQ(timings.write, 0);
    EXPECT_EQ(timings.move_left, 0);
    EXPECT_EQ(timings.move_right, 0);
    EXPECT_EQ(timings.rewind, 0);
}

TEST(file_tape, timings_to_string) {
    auto config_name = create_temp_filename();
    std::ofstream config(config_name);
    config << "r=123  ml=43 mr=1337 rewind=101 r=42" << std::endl;
    ASSERT_TRUE(std::filesystem::exists(config_name));
    file_tape::timings_config timings(config_name);
    EXPECT_EQ(timings.read, 42);
    EXPECT_EQ(timings.write, 0);
    EXPECT_EQ(timings.move_left, 43);
    EXPECT_EQ(timings.move_right, 1337);
    EXPECT_EQ(timings.rewind, 101);
    EXPECT_EQ(timings.to_string(), "read=42 write=0 move_left=43 move_right=1337 rewind=101");
    auto new_config_name = create_temp_filename();
    std::ofstream new_config(new_config_name);
    new_config << timings.to_string() << std::endl;
    ASSERT_TRUE(std::filesystem::exists(new_config_name));
    file_tape::timings_config new_timings(config_name);
    EXPECT_EQ(new_timings.read, 42);
    EXPECT_EQ(new_timings.write, 0);
    EXPECT_EQ(new_timings.move_left, 43);
    EXPECT_EQ(new_timings.move_right, 1337);
    EXPECT_EQ(new_timings.rewind, 101);
    EXPECT_EQ(new_timings.to_string(), "read=42 write=0 move_left=43 move_right=1337 rewind=101");
}

TEST(file_tape, ctor_empty_file) {
    auto filename = create_temp_filename();
    {
        file_tape tape(filename, 2, FILE_TAPE_CONFIG_NAME);
    }
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "                        ");
}

TEST(file_tape, ctor_created_empty) {
    std::string not_existing = "not_existing_file_name";
    std::filesystem::remove(not_existing);
    ASSERT_FALSE(std::filesystem::exists(not_existing));
    file_tape tape(not_existing, 42);
    ASSERT_TRUE(std::filesystem::exists(not_existing));
    std::ifstream file(not_existing);
    std::string content;
    std::getline(file, content);
    EXPECT_EQ(content, std::string((FILL_LEN + 1) * 42, ' '));
}

TEST(file_tape, ctor_partially_filled_file) {
    auto filename = create_temp_filename();
    {
        std::ofstream file(filename);
        file << "                     42            \n";
    }
    {
        file_tape tape(filename, 3, FILE_TAPE_CONFIG_NAME);
    }
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "                     42            ");
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
    file_tape tape(create_temp_filename(), 3, FILE_TAPE_CONFIG_NAME);
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
    file_tape tape(create_temp_filename(), 3, FILE_TAPE_CONFIG_NAME);
    EXPECT_FALSE(tape.read_safe().has_value());
    tape.move_right();
    EXPECT_FALSE(tape.read_safe().has_value());
    tape.move_right();
    EXPECT_FALSE(tape.read_safe().has_value());
}

TEST(file_tape, read_partially_filled) {
    auto filename = create_temp_filename();
    {
        std::ofstream file(filename);
        file << "                     42            \n";
    }
    file_tape tape(filename, 3, FILE_TAPE_CONFIG_NAME);
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
    file_tape tape(create_temp_filename(), 3, FILE_TAPE_CONFIG_NAME);
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
    auto filename = create_temp_filename();
    {
        std::ofstream file(filename);
        file << "                     42            \n";
    }
    {
        file_tape tape(filename, 3, FILE_TAPE_CONFIG_NAME);
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
    auto filename = create_temp_filename();
    {
        std::ofstream file(filename);
        file << "                     42            \n";
    }
    const int int_min = std::numeric_limits<int>::min();
    {
        file_tape tape(filename, 3, FILE_TAPE_CONFIG_NAME);
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

TEST(file_tape, big) {
    auto filename = create_temp_filename();
    {
        std::ofstream file(filename);
        file << "                     42                                            1337                                     \n";
    }
    {
        size_t size = 9;
        file_tape tape(filename, size, FILE_TAPE_CONFIG_NAME);
        ASSERT_FALSE(tape.read_safe().has_value());
        ASSERT_TRUE(tape.move_right());
        ASSERT_TRUE(tape.read_safe().has_value());
        EXPECT_EQ(tape.read_safe().value(), 42);
        ASSERT_TRUE(tape.move_right());
        ASSERT_FALSE(tape.read_safe().has_value());
        ASSERT_TRUE(tape.move_right());
        ASSERT_FALSE(tape.read_safe().has_value());
        ASSERT_TRUE(tape.move_right());
        ASSERT_FALSE(tape.read_safe().has_value());
        ASSERT_TRUE(tape.move_right());
        ASSERT_TRUE(tape.read_safe().has_value());
        EXPECT_EQ(tape.read_safe().value(), 1337);
        ASSERT_TRUE(tape.move_right());
        ASSERT_FALSE(tape.read_safe().has_value());
        ASSERT_TRUE(tape.move_right());
        ASSERT_FALSE(tape.read_safe().has_value());
        ASSERT_TRUE(tape.move_right());
        ASSERT_FALSE(tape.read_safe().has_value());
        ASSERT_FALSE(tape.move_right());
        ASSERT_TRUE(tape.move_left());
        ASSERT_FALSE(tape.read_safe().has_value());
        ASSERT_TRUE(tape.move_left());
        ASSERT_FALSE(tape.read_safe().has_value());
        ASSERT_TRUE(tape.move_left());
        ASSERT_TRUE(tape.read_safe().has_value());
        EXPECT_EQ(tape.read_safe().value(), 1337);
        ASSERT_TRUE(tape.move_left());
        ASSERT_FALSE(tape.read_safe().has_value());
        ASSERT_TRUE(tape.move_left());
        ASSERT_FALSE(tape.read_safe().has_value());
        ASSERT_TRUE(tape.move_left());
        ASSERT_FALSE(tape.read_safe().has_value());
        ASSERT_TRUE(tape.move_left());
        ASSERT_TRUE(tape.read_safe().has_value());
        EXPECT_EQ(tape.read_safe().value(), 42);
        ASSERT_TRUE(tape.move_left());
        ASSERT_FALSE(tape.read_safe().has_value());
        ASSERT_FALSE(tape.move_left());

        for (size_t i = 0; i < size; ++i) {
            tape.write(static_cast<int>(i));
            for (size_t j = i + 1; j > 0; --j) {
                ASSERT_TRUE(tape.read_safe().has_value());
                EXPECT_EQ(tape.read(), static_cast<int>(j - 1));
                EXPECT_EQ(tape.move_left(), (j != 1));
            }
            for (size_t j = 0; j <= i; ++j) {
                EXPECT_EQ(tape.move_right(), (j + 1 < size));
            }
        }
    }
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);
    ASSERT_EQ(line, "          0           1           2           3           4           5           6           7           8 ");
}

TEST_F(file_tape_fixture, rewind_beginning) {
    tape->rewind();
    EXPECT_EQ(tape->read(), 123);
    EXPECT_FALSE(tape->move_left());
    EXPECT_TRUE(tape->move_right());
    EXPECT_EQ(tape->read(), 456);
    EXPECT_TRUE(tape->move_right());
    EXPECT_EQ(tape->read(), 789);
}

TEST_F(file_tape_fixture, rewind) {
    EXPECT_TRUE(tape->move_right());
    EXPECT_EQ(tape->read(), 456);
    tape->rewind();
    EXPECT_EQ(tape->read(), 123);
    EXPECT_FALSE(tape->move_left());
    EXPECT_TRUE(tape->move_right());
    EXPECT_TRUE(tape->move_right());
    EXPECT_EQ(tape->read(), 789);
    tape->rewind();
    EXPECT_EQ(tape->read(), 123);
    EXPECT_FALSE(tape->move_left());
}

namespace {
    void test_sorted(std::vector<int> content, size_t cutoff) {
        auto src_filename = create_temp_filename();
        {
            std::ofstream file(src_filename);
            file << file_tape_content_from_vec(content) << '\n';
        }
        auto dst_filename = create_temp_filename();
        {
            auto size = content.size();
            file_tape src(src_filename, size, FILE_TAPE_CONFIG_NAME);
            file_tape dst(dst_filename, size, FILE_TAPE_CONFIG_NAME);
            sort(src, size, dst, cutoff, create_file_tape_factory(FILE_TAPE_CONFIG_NAME));
        }
        std::sort(content.begin(), content.end());
        std::ifstream file(dst_filename);
        std::stringstream buffer;
        buffer << file.rdbuf();
        ASSERT_EQ(buffer.str(), file_tape_content_from_vec(content));
    }
}

TEST(sort, one_elem) {
    test_sorted({ 42 }, 1);
    test_sorted({ 42 }, 2);
}

TEST(sort, simple) {
    test_sorted({ 3, 4, 2, 9, 4, 8, 0, 8, 1, 8, 9, 2, 6, 4, 7, 5 }, 3);
}

TEST(sort, simple_all_cutoffs) {
    std::vector<int> content = { 3, 4, 2, 9, 4, 8, 0, 8, 1, 8, 9, 2, 6, 4, 7, 5 };
    for (size_t cutoff = 1; cutoff <= content.size(); ++cutoff) {
        std::cout << "testing cutoff " << cutoff << std::endl;
        test_sorted(content, cutoff);
    }
}

namespace {
    void test_sorted_subsequent_random(size_t start_count, size_t stop_count, int min_val, int max_val) {
        std::random_device rd;
        std::default_random_engine gen(rd());
        std::uniform_int_distribution<> distrib(min_val, max_val);

        for (size_t count = start_count; count < stop_count; ++count) {
            std::vector<int> content(count);
            for (int& i : content) {
                i = distrib(gen);
            }
            for (size_t cutoff = 1; cutoff <= content.size(); ++cutoff) {
                test_sorted(content, cutoff);
            }
        }
    }
}

TEST(sort, small) {
    test_sorted_subsequent_random(2, 15, -20, 20);
}

TEST(large, sort) { // runs ~61 seconds on an SSD
    test_sorted_subsequent_random(15, 150, -200, 200);
}

TEST(large, big_file) { // runs ~53 seconds on an SSD
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_int_distribution<> distrib(-10000, 100000);

    std::vector<int> content(300000);
    for (int& i : content) {
        i = distrib(gen);
    }
    test_sorted(content, 100);
}

TEST(sort, vector_tapes) {
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_int_distribution<> distrib(-10000, 10000);

    std::vector<int> content(300000);
    for (int& i : content) {
        i = distrib(gen);
    }

    vector_tape src(content);
    vector_tape dst(content);
    sort(src, content.size(), dst, 100, create_temp_vector_tape);

    std::vector<int> res(content.size());
    dst.rewind();
    for (int& i : res) {
        i = dst.read();
        dst.move_right();
    }

    std::sort(content.begin(), content.end());
    ASSERT_EQ(res, content);
}
