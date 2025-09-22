#include "Feasibility.h"
#include <algorithm>

long long NodeDemand(const Data& data, int node) {
    if (node == 0) {
        return 0;
    }
    return data.getStationDemand(node - 1);
}

RouteFeasInfo CheckRouteFeasible(const Data& data, const Route& route) {
    RouteFeasInfo info;
    
    if (route.nodes.empty() || route.nodes.front() != 0 || route.nodes.back() != 0) {
        return info;
    }
    
    int n = data.getNumStations();
    
    for (int node : route.nodes) {
        if (node < 0 || node > n) {
            return info;
        }
    }
    
    long long prefix = 0;
    long long max_prefix = 0;
    long long min_prefix = 0;
    
    for (size_t i = 1; i < route.nodes.size() - 1; ++i) {
        prefix += NodeDemand(data, route.nodes[i]);
        max_prefix = std::max(max_prefix, prefix);
        min_prefix = std::min(min_prefix, prefix);
    }
    
    long long Q = static_cast<long long>(data.getVehicleCapacity());

    long long L0_min = std::max(0LL, -min_prefix);
    long long L0_max = Q - max_prefix;

    bool feasible = (L0_min <= L0_max);

    info.ok = feasible;
    info.max_prefix = max_prefix;
    info.min_prefix = min_prefix;
    info.L0_min = L0_min;
    info.L0_max = L0_max;
    info.suggested_L0 = std::clamp(L0_min, 0LL, Q);
    info.cap_violation = feasible ? 0LL : (L0_min - L0_max);
    
    return info;
}

bool CheckSolutionFeasible(const Data& data, const std::vector<Route>& sol) {
    const int n = data.getNumStations();
    const int m = data.getNumVehicles();
    
    // Verifica se usamos mais rotas do que veículos disponíveis
    if (static_cast<int>(sol.size()) > m) {
        return false;
    }
    
    // Contador por estação (índice = ID da estação 1..n)
    std::vector<int> count(n + 1, 0);
    
    // Verifica cada rota quanto à viabilidade
    for (const auto& route : sol) {
        RouteFeasInfo info = CheckRouteFeasible(data, route);
        if (!info.ok) {
            return false; // rota é inviável
        }
        
        // Conta visitas a cada estação (exclui depósito)
        for (int node : route.nodes) {
            if (node != 0) { // exclui depósito
                if (node < 1 || node > n) {
                    return false; // nó fora do intervalo válido
                }
                if (++count[node] > 1) {
                    return false; // estação visitada mais de uma vez
                }
            }
        }
    }
    
    // Verifica que cada estação 1..n é visitada exatamente uma vez
    for (int i = 1; i <= n; ++i) {
        if (count[i] != 1) {
            return false; // estação i não visitada ou visitada múltiplas vezes
        }
    }
    
    return true;
}
