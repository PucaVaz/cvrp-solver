#ifndef CONSTRUCTION_H
#define CONSTRUCTION_H

#include <vector>
#include <random>
#include "Instance.h"
#include "Feasibility.h"

struct Solution {
    std::vector<Route> routes;
    double total_cost = 0.0;
};

[[nodiscard]] double RouteCost(const Data& data, const Route& route);
[[nodiscard]] double SolutionCost(const Data& data, const Solution& solution);

Solution GreedyNearestFeasible(const Data& data, std::mt19937& rng);
Solution GreedyBestInsertion(const Data& data, std::mt19937& rng);

[[nodiscard]] Solution BuildInitial_GRASP(const Data& data,
                                          std::mt19937& rng,
                                          double alpha_in);

#endif