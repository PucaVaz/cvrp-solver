#include "LocalSearch.h"
#include "Feasibility.h"
#include <algorithm>
#include <limits>

bool TwoOptStep(const Data& data, Solution& solution, std::mt19937& rng) {
    for (size_t route_idx = 0; route_idx < solution.routes.size(); ++route_idx) {
        Route& route = solution.routes[route_idx];

        if (route.nodes.size() < 4) continue;

        double original_cost = RouteCost(data, route);

        for (size_t i = 1; i < route.nodes.size() - 2; ++i) {
            for (size_t j = i + 1; j < route.nodes.size() - 1; ++j) {
                std::reverse(route.nodes.begin() + i, route.nodes.begin() + j + 1);

                RouteFeasInfo feas_info = CheckRouteFeasible(data, route);
                if (feas_info.ok) {
                    double new_cost = RouteCost(data, route);

                    if (new_cost < original_cost) {
                        solution.total_cost = SolutionCost(data, solution);
                        return true;
                    }
                }

                std::reverse(route.nodes.begin() + i, route.nodes.begin() + j + 1);
            }
        }
    }

    return false;
}

bool RelocateStep(const Data& data, Solution& solution, std::mt19937& rng) {
    double original_cost = solution.total_cost;

    for (size_t from_route_idx = 0; from_route_idx < solution.routes.size(); ++from_route_idx) {
        Route& from_route = solution.routes[from_route_idx];

        if (from_route.nodes.size() <= 2) continue;

        for (size_t client_pos = 1; client_pos < from_route.nodes.size() - 1; ++client_pos) {
            int client = from_route.nodes[client_pos];

            from_route.nodes.erase(from_route.nodes.begin() + client_pos);

            bool found_improvement = false;
            for (size_t to_route_idx = 0; to_route_idx < solution.routes.size() && !found_improvement; ++to_route_idx) {
                Route& to_route = solution.routes[to_route_idx];

                for (size_t insert_pos = 1; insert_pos <= to_route.nodes.size() - 1 && !found_improvement; ++insert_pos) {
                    if (to_route_idx == from_route_idx && insert_pos == client_pos) {
                        continue;
                    }

                    to_route.nodes.insert(to_route.nodes.begin() + insert_pos, client);

                    RouteFeasInfo from_feas = CheckRouteFeasible(data, from_route);
                    RouteFeasInfo to_feas = CheckRouteFeasible(data, to_route);

                    if (from_feas.ok && to_feas.ok) {
                        double new_cost = SolutionCost(data, solution);

                        if (new_cost < original_cost) {
                            solution.total_cost = new_cost;
                            return true;
                        }
                    }

                    to_route.nodes.erase(to_route.nodes.begin() + insert_pos);
                }
            }

            from_route.nodes.insert(from_route.nodes.begin() + client_pos, client);
        }
    }

    return false;
}

bool SwapStep(const Data& data, Solution& solution, std::mt19937& rng) {
    // Troca dois clientes entre rotas diferentes
    double original_cost = solution.total_cost;

    for (size_t route1_idx = 0; route1_idx < solution.routes.size(); ++route1_idx) {
        Route& route1 = solution.routes[route1_idx];

        if (route1.nodes.size() <= 2) continue; // Pula rotas vazias

        for (size_t route2_idx = route1_idx + 1; route2_idx < solution.routes.size(); ++route2_idx) {
            Route& route2 = solution.routes[route2_idx];

            if (route2.nodes.size() <= 2) continue; // Pula rotas vazias

            // Testa trocar cada par de clientes entre as rotas
            for (size_t pos1 = 1; pos1 < route1.nodes.size() - 1; ++pos1) {
                for (size_t pos2 = 1; pos2 < route2.nodes.size() - 1; ++pos2) {
                    // Troca clientes
                    std::swap(route1.nodes[pos1], route2.nodes[pos2]);

                    RouteFeasInfo feas1 = CheckRouteFeasible(data, route1);
                    RouteFeasInfo feas2 = CheckRouteFeasible(data, route2);

                    if (feas1.ok && feas2.ok) {
                        double new_cost = SolutionCost(data, solution);

                        if (new_cost < original_cost) {
                            solution.total_cost = new_cost;
                            return true;
                        }
                    }

                    // Desfaz troca se não houve melhoria
                    std::swap(route1.nodes[pos1], route2.nodes[pos2]);
                }
            }
        }
    }

    return false;
}

bool OrOpt2Step(const Data& data, Solution& solution, std::mt19937& rng) {
    for (size_t route_idx = 0; route_idx < solution.routes.size(); ++route_idx) {
        Route& route = solution.routes[route_idx];

        if (route.nodes.size() < 5) continue;

        double original_cost = RouteCost(data, route);

        for (size_t start_pos = 1; start_pos < route.nodes.size() - 2; ++start_pos) {
            std::vector<int> chain = {route.nodes[start_pos], route.nodes[start_pos + 1]};

            route.nodes.erase(route.nodes.begin() + start_pos, route.nodes.begin() + start_pos + 2);

            for (size_t insert_pos = 1; insert_pos < route.nodes.size(); ++insert_pos) {
                if (insert_pos == start_pos) continue;

                route.nodes.insert(route.nodes.begin() + insert_pos, chain.begin(), chain.end());

                RouteFeasInfo feas_info = CheckRouteFeasible(data, route);
                if (feas_info.ok) {
                    double new_cost = RouteCost(data, route);

                    if (new_cost < original_cost) {
                        solution.total_cost = SolutionCost(data, solution);
                        return true;
                    }
                }

                route.nodes.erase(route.nodes.begin() + insert_pos, route.nodes.begin() + insert_pos + 2);
            }

            route.nodes.insert(route.nodes.begin() + start_pos, chain.begin(), chain.end());
        }
    }

    return false;
}

Solution VND(const Data& data, Solution start, std::mt19937& rng,
             std::vector<NeighborhoodFunction> neighborhoods) {

    Solution current_solution = start;
    current_solution.total_cost = SolutionCost(data, current_solution);

    int k = 0;
    int improvements_count = 0;

    while (k < static_cast<int>(neighborhoods.size())) {
        bool improved = neighborhoods[k](data, current_solution, rng);

        if (improved) {
            k = 0;
            improvements_count++;
        } else {
            k++;
        }
    }

    return current_solution;
}

std::vector<NeighborhoodFunction> GetDefaultNeighborhoods() {
    return {
        RelocateStep,
        SwapStep,
        TwoOptStep,
        OrOpt2Step
    };
}

Solution RVND(const Data& data, const Solution& start, std::mt19937& rng) {
    auto neighborhoods = GetDefaultNeighborhoods();
    
    Solution current_solution = start;
    current_solution.total_cost = SolutionCost(data, current_solution);

    std::shuffle(neighborhoods.begin(), neighborhoods.end(), rng);

    int k = 0;

    while (k < static_cast<int>(neighborhoods.size())) {
        bool improved = neighborhoods[k](data, current_solution, rng);

        if (improved) {
            std::shuffle(neighborhoods.begin(), neighborhoods.end(), rng);
            k = 0;
        } else {
            k++;
        }
    }

    return current_solution;
}