#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <vector>
#include <functional>
#include <random>
#include "Construction.h"
#include "Instance.h"

using NeighborhoodFunction = std::function<bool(const Data&, Solution&, std::mt19937&)>;

bool TwoOptStep(const Data& data, Solution& solution, std::mt19937& rng);
bool RelocateStep(const Data& data, Solution& solution, std::mt19937& rng);
bool SwapStep(const Data& data, Solution& solution, std::mt19937& rng);
bool OrOpt2Step(const Data& data, Solution& solution, std::mt19937& rng);

Solution VND(const Data& data, Solution start, std::mt19937& rng,
             std::vector<NeighborhoodFunction> neighborhoods);

std::vector<NeighborhoodFunction> GetDefaultNeighborhoods();

[[nodiscard]] Solution RVND(const Data& data, const Solution& start,
                            std::mt19937& rng);

#endif