#ifndef CONSTRUCTIVE_H
#define CONSTRUCTIVE_H

#include <vector>
#include <random>
#include "Data.h"
#include "Feasibility.h"

struct Solution {
    std::vector<Route> routes;
    double total_cost = 0.0;
};

// Calcula o custo de uma rota específica
double RoutesCost(const Data& data, const Route& route);

// Calcula o custo total de uma solução
double SolutionCost(const Data& data, const Solution& solution);

// Heurística construtiva gulosa baseada no vizinho mais próximo viável
Solution GreedyNearestFeasible(const Data& data, std::mt19937& rng);

// Heurística construtiva baseada na melhor inserção viável
Solution GreedyBestInsertion(const Data& data, std::mt19937& rng);

#endif