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
    
    // ILS metaheuristic options
    bool use_ils = false;
    int max_iter = 50;
    int max_iter_ils = 150;
    double rcl_alpha_min = 0.1;
    double rcl_alpha_max = 0.5;
    int perturb_strength = 2;
};

CLIOptions ParseCLI(int argc, char* argv[]);

void PrintUsage(const char* program_name);

#endif