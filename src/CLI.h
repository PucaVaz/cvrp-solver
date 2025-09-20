#ifndef CLI_H
#define CLI_H

#include <string>

struct CLIOptions {
    std::string instance_path = "";
    unsigned int seed = 42;
    std::string constructive_method = "nearest";  // "nearest" ou "insertion"
    bool use_vnd = true;
    std::string output_dir = "outputs/";
    bool run_feastest = false;
};

CLIOptions ParseCLI(int argc, char* argv[]);

void PrintUsage(const char* program_name);

#endif