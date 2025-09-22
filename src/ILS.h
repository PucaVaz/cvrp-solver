#ifndef ILS_H
#define ILS_H

#include <random>
#include "Instance.h"
#include "Construction.h"   // Solution type
#include "LocalSearch.h"    // RVND steps

struct ILSParams {
    int     max_iter        = 50;
    int     max_iter_ils    = 150;
    double  alpha_min       = 0.1;
    double  alpha_max       = 0.5;
    int     perturb_strength= 2;
};

[[nodiscard]] Solution ILS(const Data& data, std::mt19937& rng,
                           const ILSParams& p, bool verbose);

#endif
