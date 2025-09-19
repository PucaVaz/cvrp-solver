#include "relocate.h"
#include <stdexcept>
#include <algorithm>

// Calculo do custo de uma rota somando c(u,v)
double RouteCost(const Data &data, const Route &route) {
    double cost = 0.0;
    if (route.nodes.size() < 2) return cost;
    for (size_t i = 0; i + 1 < route.nodes.size(); ++i) {
        int u = route.nodes[i];
        int v = route.nodes[i+1];
        cost += data.getTravelCost(u, v);
    }
    return cost;
}

static bool validRouteIndex(const std::vector<Route> &sol, int r) {
    return (r >= 0 && r < (int)sol.size());
}

bool TryRelocate(const Data &data,
                 std::vector<Route> &sol,
                 int r_from, int from_idx,
                 int r_to, int to_idx,
                 double &delta_cost,
                 bool apply) {
    delta_cost = 0.0;

    // valida índices de rota
    if (!validRouteIndex(sol, r_from) || !validRouteIndex(sol, r_to)) {
        return false;
    }

    // tamanhos atuais das rotas
    int size_from = static_cast<int>(sol[r_from].nodes.size());
    int size_to   = static_cast<int>(sol[r_to].nodes.size());

    if (from_idx < 1 || from_idx > size_from - 2) {
        return false;
    }

    if (to_idx < 1 || to_idx > size_to - 1) {
        return false;
    }

    int node = sol[r_from].nodes[from_idx];
    if (node == 0) {
        return false;
    }

    if (r_from == r_to) {
        // se é operação nula (mover para a mesma posição ou posição adjacente que não altera a sequência)
        // consideramos como válido e sem custo
        if (from_idx == to_idx || from_idx + 1 == to_idx) {
            delta_cost = 0.0;
            return true;
        }

        // simula em cópia
        Route sim_route = sol[r_from];
        double old_cost = RouteCost(data, sim_route);

        sim_route.nodes.erase(sim_route.nodes.begin() + from_idx);

        // ajusta o índice de inserção se a remoção ocorreu antes do local de inserção
        int insert_pos = to_idx;
        if (from_idx < to_idx) {
            insert_pos = to_idx - 1;
        }

        if (insert_pos < 1 || insert_pos > static_cast<int>(sim_route.nodes.size()) - 1) {
            return false;
        }

        sim_route.nodes.insert(sim_route.nodes.begin() + insert_pos, node);

        // valida viabilidade da rota resultante
        if (!CheckRouteFeasible(data, sim_route).ok) {
            return false;
        }

        // calcula delta de custo
        double new_cost = RouteCost(data, sim_route);
        delta_cost = new_cost - old_cost;

        if (apply) {
            sol[r_from] = sim_route;
        }
        return true;
    }
    // Caso de rotas diferentes
    else {
        // simula as rotas em cópias
        Route sim_from = sol[r_from];
        Route sim_to   = sol[r_to];

        double old_cost_from = RouteCost(data, sim_from);
        double old_cost_to   = RouteCost(data, sim_to);

        // remove o nó da rota de origem (na cópia)
        sim_from.nodes.erase(sim_from.nodes.begin() + from_idx);

        if (to_idx < 0 || to_idx > static_cast<int>(sim_to.nodes.size())) {
            return false; 
        }
        sim_to.nodes.insert(sim_to.nodes.begin() + to_idx, node);

        if (!CheckRouteFeasible(data, sim_from).ok || !CheckRouteFeasible(data, sim_to).ok) {
            return false;
        }

        // calcula delta de custo
        double new_cost_from = RouteCost(data, sim_from);
        double new_cost_to   = RouteCost(data, sim_to);
        delta_cost = (new_cost_from + new_cost_to) - (old_cost_from + old_cost_to);

        if (apply) {
            sol[r_from] = sim_from;
            sol[r_to]   = sim_to;
        }
        return true;
    }
}
