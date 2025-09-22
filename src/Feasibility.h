#ifndef FEASIBILITY_H
#define FEASIBILITY_H

#include <vector>
#include "Instance.h"

struct Route {
    std::vector<int> nodes;
};

struct RouteFeasInfo {
    bool ok = false;
    long long cap_violation = 0;
    long long max_prefix = 0;
    long long min_prefix = 0;
    long long L0_min = 0;
    long long L0_max = 0;
    long long suggested_L0 = 0;
};

RouteFeasInfo CheckRouteFeasible(const Data& data, const Route& route);
bool         CheckSolutionFeasible(const Data& data, const std::vector<Route>& sol);

long long    NodeDemand(const Data& data, int node);

#endif
