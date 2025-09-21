#include "VND.h"
#include "Feasibility.h"
#include <algorithm>
#include <limits>

NeighborhoodResult ImproveTwoOpt(const Data& data, Solution& solution, std::mt19937& rng) {
    // Implementa 2-opt intra-rota: inverte um segmento da rota
    for (size_t route_idx = 0; route_idx < solution.routes.size(); ++route_idx) {
        Route& route = solution.routes[route_idx];

        // Precisa de pelo menos 4 nós para fazer 2-opt (0, a, b, 0)
        if (route.nodes.size() < 4) continue;

        double original_cost = RoutesCost(data, route);

        // Testa todas as inversões possíveis
        for (size_t i = 1; i < route.nodes.size() - 2; ++i) {
            for (size_t j = i + 1; j < route.nodes.size() - 1; ++j) {
                // Inverte segmento [i, j]
                std::reverse(route.nodes.begin() + i, route.nodes.begin() + j + 1);

                // Verifica viabilidade
                RouteFeasInfo feas_info = CheckRouteFeasible(data, route);
                if (feas_info.ok) {
                    double new_cost = RoutesCost(data, route);

                    // Se encontrou melhoria, mantém e retorna
                    if (new_cost < original_cost) {
                        solution.total_cost = SolutionCost(data, solution);
                        std::set<size_t> modified_routes;
                        modified_routes.insert(route_idx);
                        return NeighborhoodResult(true, modified_routes);
                    }
                }

                // Desfaz inversão
                std::reverse(route.nodes.begin() + i, route.nodes.begin() + j + 1);
            }
        }
    }

    return NeighborhoodResult(false); // Nenhuma melhoria encontrada
}

NeighborhoodResult ImproveRelocate(const Data& data, Solution& solution, std::mt19937& rng) {
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
                            std::set<size_t> modified_routes;
                            modified_routes.insert(from_route_idx);
                            modified_routes.insert(to_route_idx);
                            solution.total_cost = new_cost;
                            return NeighborhoodResult(true, modified_routes);
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

    return NeighborhoodResult(false);
}

NeighborhoodResult ImproveSwap(const Data& data, Solution& solution, std::mt19937& rng) {
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
                            std::set<size_t> modified_routes;
                            modified_routes.insert(route1_idx);
                            modified_routes.insert(route2_idx);
                            solution.total_cost = new_cost;
                            return NeighborhoodResult(true, modified_routes);
                        }
                    }

                    // Desfaz troca se não houve melhoria
                    std::swap(route1.nodes[pos1], route2.nodes[pos2]);
                }
            }
        }
    }

    return NeighborhoodResult(false);
}

NeighborhoodResult ImproveOrOpt2(const Data& data, Solution& solution, std::mt19937& rng) {
    // Move uma cadeia de 2 clientes consecutivos dentro da mesma rota
    for (size_t route_idx = 0; route_idx < solution.routes.size(); ++route_idx) {
        Route& route = solution.routes[route_idx];

        // Precisa de pelo menos 5 nós para Or-opt-2 (0, a, b, c, 0)
        if (route.nodes.size() < 5) continue;

        double original_cost = RoutesCost(data, route);

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
                    double new_cost = RoutesCost(data, route);

                    if (new_cost < original_cost) {
                        solution.total_cost = SolutionCost(data, solution);
                        std::set<size_t> modified_routes;
                        modified_routes.insert(route_idx);
                        solution.total_cost = new_cost;
                        return NeighborhoodResult(true, modified_routes);
                    }
                }

                // Remove cadeia da posição testada
                route.nodes.erase(route.nodes.begin() + insert_pos, route.nodes.begin() + insert_pos + 2);
            }

            // Reinsere cadeia na posição original
            route.nodes.insert(route.nodes.begin() + start_pos, chain.begin(), chain.end());
        }
    }

    return NeighborhoodResult(false);
}

Solution VND(const Data& data, Solution start, std::mt19937& rng,
             std::vector<NeighborhoodFunction> neighborhoods) {

    Solution current_solution = start;
    current_solution.total_cost = SolutionCost(data, current_solution);

    // Separa vizinhanças por tipo
    auto inter_neighborhoods = GetInterRouteNeighborhoods();
    auto intra_neighborhoods = GetIntraRouteNeighborhoods();

    bool global_change = true;

    while (global_change) {
        global_change = false;

        // Tenta melhorias com vizinhanças inter-rota (RVND)
        bool inter_change = true;
        while (inter_change) {
            inter_change = false;

            // Embaralha vizinhanças inter-rota
            std::shuffle(inter_neighborhoods.begin(), inter_neighborhoods.end(), rng);

            for (auto& neighborhood : inter_neighborhoods) {
                NeighborhoodResult result = neighborhood(data, current_solution, rng);

                if (result.improved) {
                    inter_change = true;
                    global_change = true;

                    // Aplica mini-RVND nas rotas modificadas
                    if (!result.modified_routes.empty()) {
                        IntraRouteRVND(data, current_solution, rng, result.modified_routes);
                    }

                    break; // Reinicia com nova ordem inter-rota
                }
            }
        }

        // Se nenhuma inter-rota melhorou, aplica intra-rota globalmente (RVND)
        if (!global_change) {
            bool intra_change = true;
            while (intra_change) {
                intra_change = false;

                // Embaralha vizinhanças intra-rota
                std::shuffle(intra_neighborhoods.begin(), intra_neighborhoods.end(), rng);

                for (auto& neighborhood : intra_neighborhoods) {
                    NeighborhoodResult result = neighborhood(data, current_solution, rng);

                    if (result.improved) {
                        intra_change = true;
                        global_change = true;
                        break; // Reinicia com nova ordem intra-rota
                    }
                }
            }
        }
    }

    return current_solution;
}

// Versão direcionada de 2-opt que atua apenas em rotas específicas
NeighborhoodResult TwoOptTargeted(const Data& data, Solution& solution, std::mt19937& rng,
                                  const std::set<size_t>& target_routes) {
    for (size_t route_idx : target_routes) {
        // Verifica se o índice da rota é válido
        if (route_idx >= solution.routes.size()) continue;

        Route& route = solution.routes[route_idx];

        // Precisa de pelo menos 4 nós para fazer 2-opt (0, a, b, 0)
        if (route.nodes.size() < 4) continue;

        double original_cost = RoutesCost(data, route);

        // Testa todas as inversões possíveis
        for (size_t i = 1; i < route.nodes.size() - 2; ++i) {
            for (size_t j = i + 1; j < route.nodes.size() - 1; ++j) {
                // Inverte segmento [i, j]
                std::reverse(route.nodes.begin() + i, route.nodes.begin() + j + 1);

                // Verifica viabilidade
                RouteFeasInfo feas_info = CheckRouteFeasible(data, route);
                if (feas_info.ok) {
                    double new_cost = RoutesCost(data, route);

                    // Se encontrou melhoria, mantém e retorna
                    if (new_cost < original_cost) {
                        solution.total_cost = SolutionCost(data, solution);
                        std::set<size_t> modified_routes;
                        modified_routes.insert(route_idx);
                        return NeighborhoodResult(true, modified_routes);
                    }
                }

                // Desfaz inversão
                std::reverse(route.nodes.begin() + i, route.nodes.begin() + j + 1);
            }
        }
    }

    return NeighborhoodResult(false);
}

// Versão direcionada de Or-opt-2 que atua apenas em rotas específicas
NeighborhoodResult OrOpt2Targeted(const Data& data, Solution& solution, std::mt19937& rng,
                                  const std::set<size_t>& target_routes) {
    for (size_t route_idx : target_routes) {
        // Verifica se o índice da rota é válido
        if (route_idx >= solution.routes.size()) continue;

        Route& route = solution.routes[route_idx];

        // Precisa de pelo menos 5 nós para Or-opt-2 (0, a, b, c, 0)
        if (route.nodes.size() < 5) continue;

        double original_cost = RoutesCost(data, route);

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
                    double new_cost = RoutesCost(data, route);

                    if (new_cost < original_cost) {
                        solution.total_cost = SolutionCost(data, solution);
                        std::set<size_t> modified_routes;
                        modified_routes.insert(route_idx);
                        return NeighborhoodResult(true, modified_routes);
                    }
                }

                // Remove cadeia da posição testada
                route.nodes.erase(route.nodes.begin() + insert_pos, route.nodes.begin() + insert_pos + 2);
            }

            // Reinsere cadeia na posição original
            route.nodes.insert(route.nodes.begin() + start_pos, chain.begin(), chain.end());
        }
    }

    return NeighborhoodResult(false);
}

// Mini-RVND que aplica vizinhanças intra-rota apenas em rotas específicas
bool IntraRouteRVND(const Data& data, Solution& solution, std::mt19937& rng,
                    const std::set<size_t>& target_routes) {
    if (target_routes.empty()) return false;

    // Lista de funções intra-rota direcionadas
    std::vector<std::function<NeighborhoodResult(const Data&, Solution&, std::mt19937&, const std::set<size_t>&)>>
        intra_functions = {
            TwoOptTargeted,
            OrOpt2Targeted
        };

    bool global_change = false;
    bool local_change = true;

    // Continua enquanto houver melhorias
    while (local_change) {
        local_change = false;

        // Embaralha as vizinhanças intra-rota (RVND)
        std::shuffle(intra_functions.begin(), intra_functions.end(), rng);

        // Testa cada vizinhança até encontrar melhoria
        for (auto& func : intra_functions) {
            NeighborhoodResult result = func(data, solution, rng, target_routes);
            if (result.improved) {
                local_change = true;
                global_change = true;
                break; // Reinicia o processo com nova ordem
            }
        }
    }

    return global_change;
}

std::vector<NeighborhoodFunction> GetInterRouteNeighborhoods() {
    return {
        ImproveRelocate,  // Move cliente entre/dentro de rotas
        ImproveSwap       // Troca clientes entre rotas diferentes
    };
}

std::vector<NeighborhoodFunction> GetIntraRouteNeighborhoods() {
    return {
        ImproveTwoOpt,    // 2-opt dentro da rota
        ImproveOrOpt2     // Move cadeia de 2 clientes dentro da rota
    };
}

std::vector<NeighborhoodFunction> GetAllNeighborhoods() {
    std::vector<NeighborhoodFunction> all_neighborhoods;

    // Adiciona inter-rota
    auto inter = GetInterRouteNeighborhoods();
    all_neighborhoods.insert(all_neighborhoods.end(), inter.begin(), inter.end());

    // Adiciona intra-rota
    auto intra = GetIntraRouteNeighborhoods();
    all_neighborhoods.insert(all_neighborhoods.end(), intra.begin(), intra.end());

    return all_neighborhoods;
}