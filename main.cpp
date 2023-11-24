#include "file_tape.h"
#include "tape_algorithm.h"
#include "tape_utils.h"

#include <args.hxx>
#include <filesystem>
#include <iostream>
#include <cassert>


int main(int argc, char* argv[]) {
    args::ArgumentParser parser("Utility for sorting tapes filled with integers.",
                                "Author: Danila Belous.");
    args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });
    args::Flag print(parser, "print", "Whether to print the output tape to stdout", { 'p', "print" });
    args::Positional<std::string> input(parser, "input", "Path to the file that emulates input tape.",
                                        args::Options::Required);
    args::Positional<size_t> size(parser, "count", "Number of elements to sort.",
                                  args::Options::Required);
    args::ValueFlag<size_t> cutoff(parser, "cutoff", "The number of elements "
                                                                    "that fit into the \"computer\"'s memory.",
                                   {'c', "cutoff"}, 1000000);
    args::ValueFlag<std::string> output(parser, "output", "Path to the output tape. "
                                                           "It either should be correct tape or should be an empty file,"
                                                           "or should point to the desired location of the output tape.",
                                   {'o', "output", "dst"}, "sorted_tape.txt");
    args::ValueFlag<std::string> config(parser, "config", "Path to the file tapes config file or "
                                                           "to the desired location where to create it.",
                                   {"config", "cfg"}, "file_tape.cfg");
    try {
        parser.ParseCLI(argc, argv);
        size_t sz = args::get(size);
        std::string cfg = args::get(config);
        file_tape src(args::get(input), sz, cfg);
        file_tape dst(args::get(output), sz, cfg);
        sort(src, sz, dst, args::get(cutoff), create_file_tape_factory(cfg));
        if (print) {
            print_tape(dst, std::cout);
        }
    } catch (args::Help&) {
        std::cout << parser;
        return 0;
    } catch (args::Error& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
}
