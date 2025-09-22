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

// Calcula o custo de uma rota específica
[[nodiscard]] double RouteCost(const Data& data, const Route& route);

// Calcula o custo total de uma solução
[[nodiscard]] double SolutionCost(const Data& data, const Solution& solution);

// Heurística construtiva gulosa baseada no vizinho mais próximo viável
Solution GreedyNearestFeasible(const Data& data, std::mt19937& rng);

// Heurística construtiva baseada na melhor inserção viável
Solution GreedyBestInsertion(const Data& data, std::mt19937& rng);

/// @brief GRASP-style constructor using Best-Insertion with RCL.
/// @param alpha_in [alpha_min, alpha_max], sampled each build.
/// @return A feasible (or repaired) initial solution.
[[nodiscard]] Solution BuildInitial_GRASP(const Data& data,
                                          std::mt19937& rng,
                                          double alpha_in);

#endif