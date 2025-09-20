#ifndef CONSTRUCTIVE_H
#define CONSTRUCTIVE_H

#include <vector>
#include "Data.h"
#include "Feasibility.h"

struct Solution {
    std::vector<Route> routes;
    long long total_cost = 0;
};

// Calcula o custo de uma rota específica
long long RoutesCost(const Data& data, const Route& route);

// Calcula o custo total de uma solução
long long SolutionCost(const Data& data, const Solution& solution);

// Heurística construtiva gulosa baseada no vizinho mais próximo viável
Solution GreedyNearestFeasible(const Data& data);

#endif