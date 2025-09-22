#ifndef FEASIBILITY_H
#define FEASIBILITY_H

#include <vector>
#include "Instance.h"

struct Route {
    std::vector<int> nodes;  // deve ser como: {0, ..., 0}
};

struct RouteFeasInfo {
    bool ok = false;
    long long cap_violation = 0; // >0 se inviável
    long long max_prefix = 0;    // demanda cumulativa máxima
    long long min_prefix = 0;    // demanda cumulativa mínima
    long long L0_min = 0;        // limite inferior viável da carga no depósito (= max_prefix)
    long long L0_max = 0;        // limite superior viável da carga no depósito (= Q + min_prefix)
    long long suggested_L0 = 0;  // clamp(L0_min, 0, Q)
};

RouteFeasInfo CheckRouteFeasible(const Data& data, const Route& route);
bool         CheckSolutionFeasible(const Data& data, const std::vector<Route>& sol);

long long    NodeDemand(const Data& data, int node); // 0->0; i>=1 -> station_demands[i-1]

#endif
