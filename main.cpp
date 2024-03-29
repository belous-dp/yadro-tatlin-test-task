#include "file_tape.h"
#include "tape_algorithm.h"
#include "tape_utils.h"

#include <args.hxx>
#include <iostream>


int main(int argc, char* argv[]) {
    args::ArgumentParser parser("A utility for sorting tapes containing integers. "
                                "Tapes are emulated using regular text files. "
                                "For the format of a configuration file and the data file of a tape, see README.md.",
                                "Author: Danila Belous.");
    args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });
    args::Flag print(parser, "print", "Whether to print the output tape to stdout", { 'p', "print" });
    args::Positional<std::string> input(parser, "input", "Path to the file that stores input tape's data.",
                                        args::Options::Required);
    args::Positional<size_t> size(parser, "count", "The number of elements to sort. Cannot be zero.",
                                  args::Options::Required);
    args::ValueFlag<size_t> cutoff(parser, "cutoff", "The number of elements "
                                                                    "that can be sorted in RAM. "
                                                                    "This number can be less than 'count' "
                                                                    "but cannot be zero."
                                                                    "The default value is 1e7.",
                                   {'m', "cutoff"}, 10000000);
    args::ValueFlag<std::string> output(parser, "output", "Path to the output tape. "
                                                           "It either should be correct tape or should be an empty file, "
                                                           "or should point to the desired location of the output tape. "
                                                           "The default value is 'sorted_tape.txt'.",
                                   {"output", "dst"}, "sorted_tape.txt");
    args::ValueFlag<std::string> config(parser, "config", "Path to the file tapes config file or "
                                                           "to the desired location where to create it. "
                                                           "The default value is 'file_tape.cfg'.",
                                   {"config", "cfg"}, "file_tape.cfg");
    try {
        parser.ParseCLI(argc, argv);
        auto sz = args::get(size);
        if (sz == 0) {
            std::cout << "Number of elements cannot be zero.";
            return 1;
        }
        auto ctff = args::get(cutoff);
        if (ctff == 0) {
            std::cout << "Number of elements which can be sorted in RAM cannot be zero.";
            return 1;
        }
        auto cfg = args::get(config);
        file_tape src(args::get(input), sz, cfg);
        file_tape dst(args::get(output), sz, cfg);
        sort(src, sz, dst, ctff, create_file_tape_factory(cfg));
        if (print) {
            print_tape(dst, std::cout);
        }
    } catch (args::Help&) {
        std::cout << parser;
    } catch (args::Error& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    } catch (std::ios_base::failure& e) {
        std::cout << "I/O error occurred while working with the tapes: " << e.what() << std::endl;
        return 1;
    } catch (std::runtime_error& e) {
        std::cout << "An error occurred while working with the tapes: " << e.what() << std::endl;
        return 1;
    }
}
