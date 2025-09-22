#include "LocalSearch.h"
#include "Feasibility.h"
#include <algorithm>
#include <limits>

bool TwoOptStep(const Data& data, Solution& solution, std::mt19937& rng) {
    // Implementa 2-opt intra-rota: inverte um segmento da rota
    for (size_t route_idx = 0; route_idx < solution.routes.size(); ++route_idx) {
        Route& route = solution.routes[route_idx];

        // Precisa de pelo menos 4 nós para fazer 2-opt (0, a, b, 0)
        if (route.nodes.size() < 4) continue;

        double original_cost = RouteCost(data, route);

        // Testa todas as inversões possíveis
        for (size_t i = 1; i < route.nodes.size() - 2; ++i) {
            for (size_t j = i + 1; j < route.nodes.size() - 1; ++j) {
                // Inverte segmento [i, j]
                std::reverse(route.nodes.begin() + i, route.nodes.begin() + j + 1);

                // Verifica viabilidade
                RouteFeasInfo feas_info = CheckRouteFeasible(data, route);
                if (feas_info.ok) {
                    double new_cost = RouteCost(data, route);

                    // Se encontrou melhoria, mantém e retorna
                    if (new_cost < original_cost) {
                        solution.total_cost = SolutionCost(data, solution);
                        return true;
                    }
                }

                // Desfaz inversão
                std::reverse(route.nodes.begin() + i, route.nodes.begin() + j + 1);
            }
        }
    }

    return false; // Nenhuma melhoria encontrada
}

bool RelocateStep(const Data& data, Solution& solution, std::mt19937& rng) {
    // Move um cliente de uma posição para outra (inter ou intra-rota)
    double original_cost = solution.total_cost;

    // Tenta mover cada cliente de cada rota
    for (size_t from_route_idx = 0; from_route_idx < solution.routes.size(); ++from_route_idx) {
        Route& from_route = solution.routes[from_route_idx];

        // Pula rotas muito pequenas (só depósito)
        if (from_route.nodes.size() <= 2) continue;

        // Tenta mover cada cliente da rota
        for (size_t client_pos = 1; client_pos < from_route.nodes.size() - 1; ++client_pos) {
            int client = from_route.nodes[client_pos];

            // Remove temporariamente o cliente
            from_route.nodes.erase(from_route.nodes.begin() + client_pos);

            // Tenta inserir em todas as rotas (incluindo a mesma)
            bool found_improvement = false;
            for (size_t to_route_idx = 0; to_route_idx < solution.routes.size() && !found_improvement; ++to_route_idx) {
                Route& to_route = solution.routes[to_route_idx];

                // Testa todas as posições de inserção
                for (size_t insert_pos = 1; insert_pos <= to_route.nodes.size() - 1 && !found_improvement; ++insert_pos) {
                    // Evita mover para a mesma posição na mesma rota
                    if (to_route_idx == from_route_idx && insert_pos == client_pos) {
                        continue;
                    }

                    // Insere cliente na nova posição
                    to_route.nodes.insert(to_route.nodes.begin() + insert_pos, client);

                    // Verifica viabilidade de ambas as rotas
                    RouteFeasInfo from_feas = CheckRouteFeasible(data, from_route);
                    RouteFeasInfo to_feas = CheckRouteFeasible(data, to_route);

                    if (from_feas.ok && to_feas.ok) {
                        double new_cost = SolutionCost(data, solution);

                        if (new_cost < original_cost) {
                            solution.total_cost = new_cost;
                            return true; // Melhoria encontrada e aplicada
                        }
                    }

                    // Remove cliente da posição testada
                    to_route.nodes.erase(to_route.nodes.begin() + insert_pos);
                }
            }

            // Reinsere cliente na posição original se não encontrou melhoria
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

                    // Verifica viabilidade de ambas as rotas
                    RouteFeasInfo feas1 = CheckRouteFeasible(data, route1);
                    RouteFeasInfo feas2 = CheckRouteFeasible(data, route2);

                    if (feas1.ok && feas2.ok) {
                        double new_cost = SolutionCost(data, solution);

                        if (new_cost < original_cost) {
                            solution.total_cost = new_cost;
                            return true; // Melhoria encontrada
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
    // Move uma cadeia de 2 clientes consecutivos dentro da mesma rota
    for (size_t route_idx = 0; route_idx < solution.routes.size(); ++route_idx) {
        Route& route = solution.routes[route_idx];

        // Precisa de pelo menos 5 nós para Or-opt-2 (0, a, b, c, 0)
        if (route.nodes.size() < 5) continue;

        double original_cost = RouteCost(data, route);

        // Testa mover cada par de clientes consecutivos
        for (size_t start_pos = 1; start_pos < route.nodes.size() - 2; ++start_pos) {
            // Extrai cadeia de 2 clientes
            std::vector<int> chain = {route.nodes[start_pos], route.nodes[start_pos + 1]};

            // Remove cadeia da rota
            route.nodes.erase(route.nodes.begin() + start_pos, route.nodes.begin() + start_pos + 2);

            // Testa inserir em todas as outras posições
            for (size_t insert_pos = 1; insert_pos < route.nodes.size(); ++insert_pos) {
                // Evita inserir na posição original
                if (insert_pos == start_pos) continue;

                // Insere cadeia na nova posição
                route.nodes.insert(route.nodes.begin() + insert_pos, chain.begin(), chain.end());

                // Verifica viabilidade
                RouteFeasInfo feas_info = CheckRouteFeasible(data, route);
                if (feas_info.ok) {
                    double new_cost = RouteCost(data, route);

                    if (new_cost < original_cost) {
                        solution.total_cost = SolutionCost(data, solution);
                        return true; // Melhoria encontrada
                    }
                }

                // Remove cadeia da posição testada
                route.nodes.erase(route.nodes.begin() + insert_pos, route.nodes.begin() + insert_pos + 2);
            }

            // Reinsere cadeia na posição original
            route.nodes.insert(route.nodes.begin() + start_pos, chain.begin(), chain.end());
        }
    }

    return false;
}

Solution VND(const Data& data, Solution start, std::mt19937& rng,
             std::vector<NeighborhoodFunction> neighborhoods) {

    Solution current_solution = start;
    current_solution.total_cost = SolutionCost(data, current_solution);

    int k = 0; // Índice da vizinhança atual
    int improvements_count = 0;

    while (k < static_cast<int>(neighborhoods.size())) {
        bool improved = neighborhoods[k](data, current_solution, rng);

        if (improved) {
            k = 0; // Reinicia do primeira vizinhança
            improvements_count++;
        } else {
            k++; // Move para próxima vizinhança
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