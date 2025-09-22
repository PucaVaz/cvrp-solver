#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <string>

struct CLIOptions {
    std::string instance_path = "";
    unsigned int seed = 42;
    std::string constructive_method = "nearest";  // "nearest" ou "insertion"
    bool use_vnd = true;
    std::string output_dir = "outputs/";
    bool run_feastest = false;
    bool verbose = false;
};

CLIOptions ParseCLI(int argc, char* argv[]);

void PrintUsage(const char* program_name);

#endif